
#include "app/ProcRecFacial.h"
#include "lib/graphics/GDibujo.h"
#include "lib/graphics/ImageSource.h"
#include "lib/utils/timedate.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/systemUtils.h"
#include "lib/utils/base64.h"
#include "lib/utils/GStringUtils.h"
#include "lib/web/GHttpClient.h"
#include "lib/hailolib/DetectionTrackerHailo.h"

#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <cstring>


/**
 * Lista con las ultimas caras detectadas
 */
vector<TrackedDetectionHailo *> ProcesoRecFacial::lstUltCarasDet;

/**
 * Constructor
 */
ProcesoRecFacial::ProcesoRecFacial()
{
    invertirVertical = false;
    usarDistEuclideana = true;
    tiempoMaxNoReconocido = 10;

    numIndentificacionesMin = 3;
}

/**
 * Establece el creador de imagenes
 */
void ProcesoRecFacial::setImageFactory( shared_ptr<ImageSourceFactory> factory )
{
    imageSource = factory->getInstance();
}

/**
 * Bucle que realiza todo el flujo
 */
void ProcesoRecFacial::iniciar()
{
    GImage imagenCapturada, imagenDeteccion;    
    GImage imagenVisor,imagenPreviaVisor;
    Mat flipped;
    vector<DeteccionCaraHailo> lstDet;
    vector<DeteccionCaraHailo> lstDetFiltrado;
    vector<TrackedDetectionHailo *> lstCaras;
    vector<TrackedDetectionHailo *> lstCarasTrack;
    hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr;
    float factorXVisor,factorYVisor;
    DetectorCarasHailoSCRFD detector;        
    FaceRecHailo generadorDesc;
    int cargaEnDuro = 1;

    errorInicial = false;
    numIndentificacionesMin = lstParamsApp->getStringLong("numIndentificacionesMin",3);
    
    // Crea el dispositivo virtual
    cout << "Creando VDevice" << endl;
    hailort::Expected<std::unique_ptr<hailort::VDevice>> device = hailort::VDevice::create();    
    if (!device) 
    {
        cerr << "Error: No se pudo inicializar el dispositivo Hailo." << endl;
        errorInicial = true;
        return;
    }
    devicePtr = &device;

    // crea el detector
    detector.setDimImagenes(640,640);    
    detector.previsualizaImgParaDeteccion = lstParamsApp->getStringBool("previsualizaImgParaDeteccion", false);
    detector.esperarPrevImgParaDeteccion = lstParamsApp->getStringBool("esperarPrevImgParaDeteccion", false);
        
    detector.runner.device = &device;
    if ( detector.cargarModelo("./models/scrfd_2.5ga.hef") != 0 )
    {
        cout << "Error al cargar el modelo detector" << endl;
        errorInicial = true;
        return;
    }               
    // if ( detector.cargarModelo("./models/scrfd_500m.hef") != 0 )
    // {
    //     cout << "Error al cargar el modelo detector" << endl;
    //     errorInicial = true;
    //     return;
    // }           
    // if ( detector.cargarModelo("./models/scrfd_10g.hef") != 0 )
    // {
    //     cout << "Error al cargar el modelo detector" << endl;
    //     errorInicial = true;
    //     return;
    // }           
    cout << "Detector de rostros iniciado" << endl;
    // detector.runner.printInfoStreamSalida();

    // crea el generador de descriptores faciales
    generadorDesc.runner.device = &device;
    generadorDesc.paramContraste = paramContraste;
    generadorDesc.nombreUltimaCapaModelo = nombreCapaSalidaRedFacial;
    generadorDesc.previsualizaImgReconocimiento = lstParamsApp->getStringBool("previsualizaImgReconocimiento", false);
    generadorDesc.esperarPrevImgReconocimiento = lstParamsApp->getStringBool("esperarPrevImgReconocimiento", false);
    generadorDesc.guardarPrevImgReconocimiento = lstParamsApp->getStringBool("guardarPrevImgReconocimiento", false);
    generadorDesc.guardarCarasFrontalesAlineadas = lstParamsApp->getStringBool("guardarCarasFrontalesAlineadas", false);
    generadorDesc.prefijoImgReconocimiento = lstParamsApp->getString("prefijoImgReconocimiento");
    generadorDesc.prefijoImgReconocimiento+= "_" + to_string(TimeDateUtils::getDateTimeMs()) + "_";
    generadorDesc.similaridadFrontal = lstParamsApp->getStringDouble("similaridadFrontal", 5.0);
    
    // if ( generadorDesc.cargarModelo("./models/arcface_mobilefacenet.hef") != 0 )
    if ( generadorDesc.cargarModelo(pathModeloDescFacial) != 0 )
    {
        cout << "Error al cargar modelo generador de descriptores" << endl;
        errorInicial = true;
        return;
    }
    cout << "Generador de descriptores iniciado" << endl;
    // generadorDesc.runner.printInfoStreamSalida();    

    // Inicia el servidor web
    servidorWeb = make_shared<ServidorHttpImagenes>();
    servidorWeb->lstParamsApp = this->lstParamsApp;
    servidorWeb->procRecFacial = this;
    servidorWeb->iniciar();

    // Factor de escala para dimensiones de visualizacion
    factorEscalaVisualizaX = ((double)anchoVisualiza) / ((double)anchoCamara);
    factorEscalaVisualizaY = ((double)alturaVisualiza) / ((double)alturaCamara);

    // ejecuta el procesador de eventos    
    generadorEventos.start();
    generadorPingsMonitoreo.start();
    generadorPingsAppWeb.start();

    factorXVisor = (float)anchoCamara / ((float)640);
    factorYVisor = (float)alturaCamara / ((float)640);
        
    // bucle principal
    finalizar = false;
    idDesconocidoSgte = 0;
    while( finalizar == false )
    {
        auto inicio = std::chrono::high_resolution_clock::now();

        imagenCapturada = imageSource->getImage();
        
        if (( imagenCapturada.ancho == 0 ) || ( imagenCapturada.altura == 0 ))
        {           
            continue;
        }
        
        // invierte verticalmente
        if ( invertirVertical )
        {
            flip(imagenCapturada.imagenOpencv, flipped, -1);
            imagenCapturada.imagenOpencv = flipped;
        }

        if (( regionInteresInicioX > 0 ) || ( regionInteresFinX < anchoCamara ))
        {
            imagenDeteccion = imagenCapturada.getRect(regionInteresInicioX,0,regionInteresFinX,alturaCamara);
        }    
        else imagenDeteccion = imagenCapturada;
        
        auto finCaptura = std::chrono::high_resolution_clock::now();
        
        // detecta rostros
        lstDet = detector.detectar_2_5g(imagenDeteccion, toleranciaDetec);        
        // lstDet = detector.detectar_500m(imagenCapturada, toleranciaDetec);        
        // lstDet = detector.detectar_10g(imagenCapturada, toleranciaDetec);        
        if ( detector.errorDeteccion == true )
        {
            cout << "Error al detectar rostros" << endl;
            break;
        }
        // elimina rostros pequeños
        lstDetFiltrado.clear();
        // cout << "Num Caras Iniciales " << lstDet.size() << endl;
        for(DeteccionCaraHailo cara : lstDet)
        {
            if ((( cara.getAncho() > anchoRostroMinimo ) && ( cara.getAltura() > alturaRostroMinima))  &&              
                ( cara.caraDePerfil() == false ))
            {                
                cara.desplazaPtosCara(regionInteresInicioX,0);                
                cara.desplazaRegion(regionInteresInicioX,0);
                if ( encuadrarRostros.compare("S") == 0 ) cara.ajustarCuadrado(anchoCamara, alturaCamara);
                else
                if ( encuadrarRostros.compare("M") == 0 ) cara.ajustarMiniCuadrado(anchoCamara, alturaCamara);

                // if ( cara.getAltura() == cara.getAncho() )
                    lstDetFiltrado.push_back(cara);
            }

            // cara.ajustarCuadrado(anchoCamara, alturaCamara);
            // lstDetFiltrado.push_back(cara);
        }        
        // cout << "Num Caras Finales " << lstDetFiltrado.size() << endl;

        // Analiza tracking 
        // lstCaras = detTracker.analizaRapido(&lstDetFiltrado);
        lstCaras = detTracker.generaListaTrackTemporal(&lstDetFiltrado);
        auto finDeteccion = std::chrono::high_resolution_clock::now();

        // genera descriptores faciales
        calculaDescriptores( &generadorDesc, imagenCapturada, &lstCaras);
        // if ( generadorDesc.errorCalculo == true )
        // {
        //     cout << "Error al generar descriptores faciales " << endl;
        //     break;
        // }
        auto finDescriptores = std::chrono::high_resolution_clock::now();

        // Busca coincidencias de las detecciones con el universo de personas conocidas
        identificarPersonas(&lstCaras);
        auto finIdentificacion = std::chrono::high_resolution_clock::now();

        // Hace el tracking de las caras procesadas
        // lstCarasTrack = detTracker.analizaPorIdentificacion(&lstCaras);
        // imagenVisor = imagenCapturada.cloneResize(detector.imgModeloAncho, detector.imgModeloAltura);
        imagenVisor.ancho = detector.imgModeloAncho;
        imagenVisor.altura = detector.imgModeloAltura;
        imagenVisor.imagenOpencv = detector.imagenRedim;
        lstCarasTrack = detTracker.analizaPorIdentificacionYTracker(&lstCaras, &imagenVisor, &imagenPreviaVisor, factorXVisor, factorYVisor);
        auto finTracking = std::chrono::high_resolution_clock::now();

        // Notifica al servidor web las incidencias o detecciones
        notificaDetecciones(imagenCapturada, &lstCarasTrack);
        auto finNotifica = std::chrono::high_resolution_clock::now();
        imagenPreviaVisor = imagenVisor.clone();
        
        imagenVisor = dibujaCaras(imagenCapturada,&lstCarasTrack);
        auto finDibuja = std::chrono::high_resolution_clock::now();

        universoPersonas.eliminaDesAntiguos(tiempoMaxNoReconocido);

        // calcula los tiempos transcurridos
        auto durCaptura = std::chrono::duration_cast<std::chrono::microseconds>(finCaptura - inicio);
        auto durDetecta = std::chrono::duration_cast<std::chrono::microseconds>(finDeteccion - finCaptura);
        auto durDescriptor = std::chrono::duration_cast<std::chrono::microseconds>(finDescriptores - finDeteccion);
        auto durIdentificacion = std::chrono::duration_cast<std::chrono::microseconds>(finIdentificacion - finDescriptores);
        auto durTracking = std::chrono::duration_cast<std::chrono::microseconds>(finTracking - finIdentificacion);
        auto durNotifica = std::chrono::duration_cast<std::chrono::microseconds>(finNotifica - finTracking);
        auto durDibuja = std::chrono::duration_cast<std::chrono::microseconds>(finDibuja - finNotifica);
        auto durTotal = std::chrono::duration_cast<std::chrono::microseconds>(finDibuja - inicio);

        // cout << "Tiempos microsegundos " << endl << endl;
        // cout << "Captura      : " << durCaptura.count() << endl;
        // cout << "Detecta      : " << durDetecta.count() << endl;
        // cout << "Descriptores : " << durDescriptor.count() << endl;
        // cout << "Identifica : " << durIdentificacion.count() << endl;
        // cout << "Tracking : " << durTracking.count() << endl;
        // cout << "notifica : " << durNotifica.count() << endl;
        // cout << "Dibujo       : " << durDibuja.count() << endl;
        // cout << "** TOTA      : " << durTotal.count() << endl;
        // cout << endl;

        // Envia la imagen al servidor web de configuracion        
        ProcesoRecFacial::lstUltCarasDet = lstCarasTrack;
        servidorWeb->setImage(imagenVisor);
        
        // Envia la imagen a la pantalla local
        GDibujo::show(imagenVisor, "Visor");        
        
        // configura el procesar de eventos del mouse
        cv::setMouseCallback("Visor", onMouse, this);

        // Lee el teclada para finalziar si presiona ESC        
        int tecla = GDibujo::waitForKey(10);
        if ( tecla == 'q' )
        {
            break;
        }
        else
        if ( tecla == '1' )
        {
            cargaEnDuro = 1;
        }

        // SystemUtils::printRamUsage();
    }
    
    cout << "Finaliza Bucle reconocimiento" << endl;
    GDibujo::closeAllWindows();    
    imageSource->stop();    
    generadorPingsAppWeb.finalizar();
    generadorPingsMonitoreo.finalizar();
    generadorEventos.finalizar();
    
    universoPersonas.lstPerIdentificadas.reset();
    universoPersonas.lstPerNoIdent.reset();

    
    detTracker.reset();
    detTracker.lstUniverso.reset();

    exit(0);
}


/**
 * Calcula los descriptores faciales y califica los objetos
 * Retorne una lista de detecciones clasificados
 * 
 *      foto :
 *          Imagen desde la que se extrae la cara
 * 
 *      lstRostros: 
 *          Lista de detecciones hechas en la foto
 */
void ProcesoRecFacial::calculaDescriptores( FaceRecHailo *generador, GImage foto, vector<TrackedDetectionHailo *> *lstRostros )
{
    GLinkedList<TrackedDetectionHailo *> lstCarasDet;
    GLinkedList<GImage> lstCaras;
    GLinkedList<DeteccionCaraHailo> lstDetecciones;
    GLinkedList<SIMD_TYPE*>lstDescriptores;
    TrackedDetectionHailo *deteccion;
    int n = lstRostros->size();
        
    for(int i=0; i<n; i++)
    {
        deteccion = lstRostros->at(i);        
              
        // calcula la deteccion relativa al rectangulo en el que se detecto
        deteccion->cara.detRelCara = deteccion->cara.deteccion;            
        deteccion->cara.detRelCara.desplazaPtosCara(-deteccion->cara.deteccion.ptoSupIzq.x, -deteccion->cara.deteccion.ptoSupIzq.y);
        deteccion->cara.detRelCara.ptoSupIzq.x = 0;
        deteccion->cara.detRelCara.ptoSupIzq.y = 0;
        deteccion->cara.detRelCara.ptoInfDer.x -= deteccion->cara.deteccion.ptoSupIzq.x;
        deteccion->cara.detRelCara.ptoInfDer.y -= deteccion->cara.deteccion.ptoSupIzq.y;

        // deteccion->cara.descCalculado = true;        
        deteccion->cara.calculaFotoCara(foto);
        
        // lstCaras.add(deteccion->cara.fotoCara);
        // lstDetecciones.add(deteccion->cara.detRelCara);
        // lstDescriptores.add(deteccion->cara.descriptor);

        lstCarasDet.add(deteccion);
        
        // comantado porque se calcularan por batch
        // generador->calculaDescriptor(deteccion->cara.fotoCara, deteccion->cara.detRelCara, deteccion->cara.descriptor);        
    }

    if ( n > 0)
        // generador->calculaDescriptor(&lstCaras, &lstDetecciones, &lstDescriptores);
        generador->calculaDescriptor(&lstCarasDet);

    // lstCaras.reset();
    // lstDetecciones.reset();
    // lstDescriptores.reset();
    lstCarasDet.reset();
}



/**
 * Analiza todas las caras de la lista de rostros detectados
 * para ver si alguno coincide con una persona conocida
 */
void ProcesoRecFacial::identificarPersonas( vector<TrackedDetectionHailo *> *lstRostros )
{
    TrackedDetectionHailo *deteccion;
    DescPersonaExterno *personaConocida;
    IdentificacionPersona idenPersonaValidaDec;    
    int i,n;
    long long fecDet;
    float distEucli;

    fecDet = TimeDateUtils::getDateTimeMs();
    n = lstRostros->size()-1;
    for(i=n; i>=0; i--)
    {
        deteccion = lstRostros->at(i);
        if ( deteccion->ciclosNoDetectados > 0 )
        {
            continue;
        }            

        // solo se identifica a la persona si es nueva   
        if ( deteccion->tracker.empty() == true )
        {     
            personaConocida = universoPersonas.buscaPersonaCon(deteccion->cara.descriptor, toleranciaIden, &distEucli, usarDistEuclideana);
        }
        else
        {
            // no se procesa la deteccion porque es resultado de un tracking
            continue;
        }    
        if ( personaConocida != NULL )
        {
            IdentificacionPersona iden;

            iden.fecDet = fecDet;
            std::memcpy(iden.vecDescripcion,deteccion->cara.descriptor,NUM_ELEMS_DESC_FACIAL*sizeof(SIMD_TYPE));
            iden.comparacion = distEucli;
            deteccion->cara.personaIdent = true;      
            personaConocida->anonimo =  false;                  
            deteccion->cara.identificador.agregaIdentif(personaConocida, iden , fecDet);

            idenPersonaValidaDec = deteccion->cara.identificador.getUltimaIdentificacion();            
        }
        else 
        if ( reportarDesconocidos == true )
        {
            personaConocida = universoPersonas.buscaPersonaDesc(deteccion->cara.descriptor, toleranciaIden, &distEucli, usarDistEuclideana);
            if ( personaConocida != NULL )
            {
                IdentificacionPersona iden;

                iden.fecDet = fecDet;
                std::memcpy(iden.vecDescripcion,deteccion->cara.descriptor,NUM_ELEMS_DESC_FACIAL*sizeof(SIMD_TYPE));
                iden.comparacion = distEucli;
                deteccion->cara.personaIdent = true;            
                deteccion->cara.identificador.agregaIdentif(personaConocida, iden , fecDet);    
                personaConocida->ultimaFechaDetectada = fecDet;         
                
                idenPersonaValidaDec = deteccion->cara.identificador.getUltimaIdentificacion();                
            }
            else
            {                
                IdentificacionPersona iden;

                iden.fecDet = fecDet;
                std::memcpy(iden.vecDescripcion,deteccion->cara.descriptor,NUM_ELEMS_DESC_FACIAL*sizeof(SIMD_TYPE));
                iden.comparacion = 100;
                deteccion->cara.personaIdent = true;            
                
                DescPersonaExterno personaExterna;

                personaExterna.anonimo = true;
                std::memcpy(personaExterna.vecDescripcion,iden.vecDescripcion,NUM_ELEMS_DESC_FACIAL*sizeof(SIMD_TYPE));
                personaExterna.id = std::to_string(idDesconocidoSgte);
                personaExterna.nombre = "Desc." + personaExterna.id;
                personaExterna.ultimaFechaDetectada = fecDet;
                idDesconocidoSgte++;
                universoPersonas.lstPerNoIdent.add(personaExterna);
                
                deteccion->cara.identificador.agregaIdentif(universoPersonas.lstPerNoIdent.getAddrUltimo(), iden , fecDet);
                idenPersonaValidaDec = deteccion->cara.identificador.getUltimaIdentificacion();
            }
        }
        else
        {
            deteccion->cara.personaIdent = false; 
            // removemos a la persona pues es desconocida
            // lstRostros->erase(lstRostros->begin() + i); 
        }
    }
}

/**
 * Dibuja las caras encontradas
 * Retorna la imagen para el visor
 */
GImage ProcesoRecFacial::dibujaCaras( GImage imagen, vector<TrackedDetectionHailo *> *lstCaras )
{
    GImage imagenVisor;
    CaraDescrita *cara;
    TrackedDetectionHailo *det;
    GPoint punto;
    GRect caja;
    GColor amarillo(0,255,255);
    GColor rojo(255,0,0);
    GColor verde(0,255,0);
    GColor azul(0,0,255);
    GColor blanco(255,255,255); 
    DescPersonaExterno *datosPersona;
    GRect rc;
    string id;
    int temp, radioPuntos = 2;
    float escalaX,escalaY,escalaXTrack,escalaYTrack;

    imagenVisor = imagen.cloneResize(anchoVisualiza, alturaVisualiza);
    escalaX = ((float)anchoVisualiza)/((float)anchoCamara);
    escalaY = ((float)alturaVisualiza)/((float)alturaCamara);

    escalaXTrack = ((float)anchoVisualiza)/((float)NUM_ELEMS_DESC_FACIAL);
    escalaYTrack = ((float)alturaVisualiza)/((float)NUM_ELEMS_DESC_FACIAL);

    rc.x1 = (int)((float)regionInteresInicioX)*escalaX;
    rc.x2 = (int)((float)regionInteresFinX)*escalaX;
    rc.y1 = 0;
    rc.y2 = alturaVisualiza;
    GDibujo::drawRect(imagenVisor, rc, azul, 1); 

    int n = lstCaras->size();
    for(int i=0; i<n; i++ )
    {
        det = lstCaras->at(i);
        
        if ( det->id > 0 ) id = to_string(det->id);
        else id = "";

        cara = &det->cara;

        caja.x1 = cara->deteccion.ptoSupIzq.x * escalaX;        
        caja.y1 = cara->deteccion.ptoSupIzq.y * escalaY;
        caja.x2 = cara->deteccion.ptoInfDer.x * escalaX;
        caja.y2 = cara->deteccion.ptoInfDer.y * escalaY;

        datosPersona = det->cara.identificador.getDatosPerIden();     
        
        if ( det->tracker.empty() == false )
        {
            rc.x1 = (int)(((float)det->trackerBox.x)*escalaXTrack) ;
            rc.y1 = (int)(((float)det->trackerBox.y)*escalaYTrack);
            rc.x2 = rc.x1 + (int)(((float)det->trackerBox.width)*escalaXTrack);
            rc.y2 = rc.y1 + (int)(((float)det->trackerBox.height)*escalaYTrack);

            GDibujo::drawRect(imagenVisor, rc, amarillo, 1); 
        }
        
        if ( cara->descCalculado == false )
        {
            GDibujo::drawRect(imagenVisor, caja, amarillo, 2);        
        }
        else
        if (( cara->personaIdent == false ) || ( datosPersona == NULL ))
        {
            GDibujo::drawRect(imagenVisor, caja, rojo, 2);
        }
        else
        {            
            if ( datosPersona != NULL )
            {
                id.append(" - ");
                id.append(GStringUtils::to_string_fixed(det->cara.identificador.getPromedioComparacion(),2));
                id.append(" : ");                
                id.append(datosPersona->nombre);
                
                // id.append(to_string(det->cara.identificador.getUltimaIdentificacion().comparacion));                
            }                
            if ( datosPersona->anonimo == true ) GDibujo::drawRect(imagenVisor, caja, rojo, 2);
            else GDibujo::drawRect(imagenVisor, caja, verde, 2);
        }        
    
        if ( det->tracker.empty() == true )
        {
            punto.x = (int)(((float)cara->deteccion.ojoIzq.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.ojoIzq.y) * escalaY);
            GDibujo::drawElipse(imagenVisor, punto, radioPuntos, radioPuntos, rojo, 1 );

            punto.x = (int)(((float)cara->deteccion.ojoDer.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.ojoDer.y) * escalaY);
            GDibujo::drawElipse(imagenVisor, punto, radioPuntos, radioPuntos, rojo, 1 );

            punto.x = (int)(((float)cara->deteccion.nariz.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.nariz.y) * escalaY);
            GDibujo::drawElipse(imagenVisor, punto, radioPuntos, radioPuntos, rojo, 1 );

            punto.x = (int)(((float)cara->deteccion.bocaIzq.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.bocaIzq.y) * escalaY);
            GDibujo::drawElipse(imagenVisor, punto, radioPuntos, radioPuntos, rojo, 1 );

            punto.x = (int)(((float)cara->deteccion.bocaDer.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.bocaDer.y) * escalaY);
            GDibujo::drawElipse(imagenVisor, punto, radioPuntos, radioPuntos, rojo, 1 );
        }

        // dibuja la caja con el ID de seguimiento
        temp = caja.y1 - 30;
        if ( temp < 0 )
        {
            // se dibuja la caja en la parte inferior
            caja.y1 = caja.y2;            
            caja.y2+= 40;
        }
        else
        {
            caja.y2 = caja.y1;            
            caja.y1-= 40;
        }
        caja.x2+= 100;
        caja.x1-= 200;
        if ( caja.x1 < 0 ) caja.x1 = 0;

        if ( datosPersona !=  NULL )
        {
            if ( det->cara.identificador.fechaUltEvento == 0 )
            {
                GDibujo::drawRect(imagenVisor, caja, blanco, -1 );
                GDibujo::drawText(imagenVisor, caja.x1, caja.y1+30, id , GDIBUJO_FONT_HELVETICA, azul, 0.8, 2 );
            }
            else
            {
                GDibujo::drawRect(imagenVisor, caja, azul, -1 );
                GDibujo::drawText(imagenVisor, caja.x1, caja.y1+30, id , GDIBUJO_FONT_HELVETICA, blanco, 0.8, 2 );
            }
            
        }        
    }

    return imagenVisor;
}

/**
 * Procesa el evento click sobre la pantalla
 */
void ProcesoRecFacial::onMouse( int event, int x, int y, int flags, void *userdata )
{
    float escalaX = ((float)1920)/((float)1280);
    float escalaY = ((float)1080)/((float)720);
    float x1,y1,x2,y2;
    int i,n;    
    TrackedDetectionHailo *track;

    if ( event != cv::EVENT_LBUTTONDOWN )
        return;

    n = ProcesoRecFacial::lstUltCarasDet.size();
    for(i=0;i<n;i++)
    {
        track = ProcesoRecFacial::lstUltCarasDet.at(i);
        x1 = ((float)track->cara.deteccion.ptoSupIzq.x)/escalaX;
        y1 = ((float)track->cara.deteccion.ptoSupIzq.y)/escalaY;
        x2 = ((float)track->cara.deteccion.ptoInfDer.x)/escalaX;
        y2 = ((float)track->cara.deteccion.ptoInfDer.y)/escalaY;

        if ( ( ( x1 <= x ) && ( x2 >= x ) ) && ( ( y1 <= y ) && ( y2 >= y ) ) )
        {
            cout << endl;
            for(int j=0;j<NUM_ELEMS_DESC_FACIAL; j++)
            {
                cout << track->cara.descriptor[j] << ",";
            }
            cout << endl << endl;
            break;
        }
    }
}


/**
 * Guarda los parametros de la aplicacion
 */
void ProcesoRecFacial::guardaParametros()
{
    guardaArchivoConfig(pathParametros, lstParamsApp);
}

/**
 * Lee los parametros de ejecucion
 */
void ProcesoRecFacial::leeParametros(string path)
{
    pathParametros = path;
    lstParamsApp = leeArchivoConfig(pathParametros);
    
    // --- Nuevo: Config Pings App Web (desde config.txt) ---
    generadorPingsAppWeb.appWebUrl      = lstParamsApp->getString("appWebUrl");
    generadorPingsAppWeb.generarLogPing = lstParamsApp->getStringBool("generarLogPingAppWeb", false);
    generadorPingsAppWeb.pathLogPing    = lstParamsApp->getString("pathLogPingAppWeb");
    generadorPingsAppWeb.pingIntervalMs = lstParamsApp->getStringLong("pingIntervalMsAppWeb", 5000);
    // ------------------------------------------------------
    generadorPingsMonitoreo.dotnetUrl      = lstParamsApp->getString("dotnetUrl");
    generadorPingsMonitoreo.generarLogPing = lstParamsApp->getStringBool("generarLogPingMonitoreo", false);
    generadorPingsMonitoreo.pathLogPing    = lstParamsApp->getString("pathLogPingMonitoreo");
    generadorPingsMonitoreo.pingIntervalMs = lstParamsApp->getStringLong("pingIntervalMsMonitoreo", 5000);
}


/**
 * Notifica al servidor web sobre las detecciones ocurridas
 */
void ProcesoRecFacial::notificaDetecciones( GImage imagen, vector<TrackedDetectionHailo *> *lstCaras )
{
    int i,n,j,numIteracionesUnif;
    int bufferLen;
    double valor;
    size_t bufferLen64;
    string jsonLstRostros,nombres;
    TrackedDetectionHailo *persona;
    char *bufferImgVisor, *bufferImgVisor64;    
    long long delta, fecEvento = TimeDateUtils::getDateTimeMs();
    string ahora = TimeDateUtils::getFechaDesdeMs(fecEvento);
    DescPersonaExterno *descPersona;

    if ( generadorEventos.usarEndpointUnificado == true ) numIteracionesUnif = 1;
    else numIteracionesUnif = 2;

    n = lstCaras->size();
    // bucle para generar datos de perasonas conocidas (bucleTipoPer=0) y para personas no idenentificadas (bucleTipoPer=1)
    for(int bucleTipoPer=0; bucleTipoPer < numIteracionesUnif; bucleTipoPer++)
    {
        jsonLstRostros = "";
        for(i=0; i<n; i++)
        {
            persona = lstCaras->at(i);
            descPersona = persona->cara.identificador.getDatosPerIden();
            if ( descPersona == NULL )
                continue;

            // se valida si el tipo de persona pertnece al bucle, si no se ignora
            if ( generadorEventos.usarEndpointUnificado == false )
            {
                if (( bucleTipoPer == 0 ) && ( persona->cara.identificador.getDatosPerIden()->anonimo == true )) continue;
                else
                if (( bucleTipoPer == 1 ) && ( persona->cara.identificador.getDatosPerIden()->anonimo == false )) continue;
            }
                
            delta = fecEvento-persona->cara.identificador.fechaUltEvento;
            if (( persona->cara.identificador.getNumIdentificaciones() >= numIndentificacionesMin ) && 
                (( persona->cara.identificador.cambioIdentificacion == true ) || 
                (( persona->cara.personaIdent == true ) && ( delta >= tiempoReEvento ))))
            {
                // persona->cara.identificador.cambioIdentificacion = false;
                persona->cara.identificador.fechaUltEvento = fecEvento;

                nombres.append(persona->cara.identificador.getDatosPerIden()->nombre);
                nombres.append(",");
                
                bufferImgVisor = GDibujo::encode(persona->cara.fotoCara, GDIBUJO_ENCODE_JPEG, 80, bufferLen);
                bufferImgVisor64 = base64_encode((const unsigned char *)bufferImgVisor, bufferLen, &bufferLen64);
                free(bufferImgVisor);

                if ( jsonLstRostros.length() > 0 ) 
                    jsonLstRostros.append(",");

                jsonLstRostros.append("\n{");
                            
                GStringUtils::addJsonAtt(&jsonLstRostros,"foto", bufferImgVisor64,false);            
                free(bufferImgVisor64);

                jsonLstRostros.append("\"coordenadas\":{");
                        
                valor = ((double)persona->cara.deteccion.ptoSupIzq.x)*factorEscalaVisualizaX;
                GStringUtils::addJsonAtt(&jsonLstRostros,"x1",to_string(valor),true);                        

                valor = ((double)persona->cara.deteccion.ptoSupIzq.y)*factorEscalaVisualizaY;
                GStringUtils::addJsonAtt(&jsonLstRostros,"y1",to_string(valor),true);                     

                valor = ((double)persona->cara.deteccion.ptoInfDer.x)*factorEscalaVisualizaX;
                GStringUtils::addJsonAtt(&jsonLstRostros,"x2",to_string(valor),true);                        

                valor = ((double)persona->cara.deteccion.ptoInfDer.y)*factorEscalaVisualizaY;
                GStringUtils::addJsonAtt(&jsonLstRostros,"y2",to_string(valor),true,false);
                
                jsonLstRostros.append("},\n");
                jsonLstRostros.append("\"descripcionFacial\":[\n");
        
                for(j=0; j<NUM_ELEMS_DESC_FACIAL; j++)
                {            
                    jsonLstRostros.append(to_string(persona->cara.descriptor[j]));
                    if ( j < (NUM_ELEMS_DESC_FACIAL-1) ) jsonLstRostros.append(",");
                }
        
                jsonLstRostros.append("\n],\n");                
                if ( persona->cara.identificador.getDatosPerIden()->anonimo == false )
                {                    
                    GStringUtils::addJsonAtt(&jsonLstRostros,"idReconocido",persona->cara.identificador.getDatosPerIden()->id,true);
                    jsonLstRostros.append("\"idTipoRostro\":2,\n");    
                }
                else
                {
                    GStringUtils::addJsonAtt(&jsonLstRostros,"idReconocido","0",true);
                    jsonLstRostros.append("\"idTipoRostro\":1,\n");    
                }           
                // IdentificacionPersona iden = persona->cara.identificador.getUltimaIdentificacion();                      
                GStringUtils::addJsonAtt(&jsonLstRostros,"probabilidad",GStringUtils::to_string_fixed(persona->cara.identificador.getPromedioComparacion(),2),true,false);
                jsonLstRostros.append("}\n");
            }
        }   

        if ( jsonLstRostros.size() > 0 )
        {
            // Si hay detecciones nuevas, se debe generar un evento
            string trama;
            GImage fotoEscalada = imagen.cloneResize(anchoVisualiza, alturaVisualiza);

            bufferImgVisor = GDibujo::encode(fotoEscalada, GDIBUJO_ENCODE_JPEG, compresionJpeg, bufferLen);
            bufferImgVisor64 = base64_encode((const unsigned char *)bufferImgVisor, bufferLen, &bufferLen64);            
            free(bufferImgVisor);

            trama.append("{\n");

            // trama.append("\"cuadro\":\"");
            // trama.append(bufferImgVisor64);
            // trama.append("\",\n");
            GStringUtils::addJsonAtt(&trama, "cuadro", bufferImgVisor64, false);
            // GStringUtils::addJsonAtt(&trama, "cuadro", "Foto Completa", false);

            free(bufferImgVisor64);

            // trama.append("\"serieEquipo\":\"");
            // trama.append(GStringUtils::trim(idEquipo));
            // trama.append("\",\n");
            GStringUtils::addJsonAtt(&trama, "serieEquipo", GStringUtils::trim(idEquipo), false);
            GStringUtils::addJsonAtt(&trama,"fecEvento",ahora,false);

            trama.append("\"lstRostros\":[");    
            trama.append(jsonLstRostros);
            trama.append("]\n");
            trama.append("}\n");
            
            // cout << "Reconocidos: " << nombres << endl;
            // cout << "Trama:" <<  trama << endl;

            if ( bucleTipoPer == 0 ) generadorEventos.agregarTramaIden(trama);
            else generadorEventos.agregarTramaNoIden(trama);
        }
    }
}
