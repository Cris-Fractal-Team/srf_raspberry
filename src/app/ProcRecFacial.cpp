
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
    GImage imagenCapturada;    
    GImage imagenVisor;
    Mat flipped;
    vector<DeteccionCaraHailo> lstDet;
    vector<DeteccionCaraHailo> lstDetFiltrado;
    vector<TrackedDetectionHailo *> lstCaras;
    vector<TrackedDetectionHailo *> lstCarasTrack;
    hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr;
    DetectorCarasHailoSCRFD detector;        
    FaceRecHailo generadorDesc;

    errorInicial = false;

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
    detector.runner.device = &device;
    if ( detector.cargarModelo("./models/scrfd_2.5ga.hef") != 0 )
    {
        cout << "Error al cargar el modelo detector" << endl;
        errorInicial = true;
        return;
    }           
    cout << "Detector de rostros iniciado" << endl;
    // detector.runner.printInfoStreamSalida();

    // crea el generador de descriptores faciales
    generadorDesc.runner.device = &device;
    if ( generadorDesc.cargarModelo("./models/arcface_mobilefacenet.hef") != 0 )
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

    // ejecuta el procesador de eventos
    generadorEventos.start();
        
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
        
        auto finCaptura = std::chrono::high_resolution_clock::now();
        
        // detecta rostros
        lstDet = detector.detectar_2_5g(imagenCapturada, toleranciaDetec);        
        if ( detector.errorDeteccion == true )
        {
            cout << "Error al detectar rostros" << endl;
            break;
        }
        // elimina rostros pequeños
        lstDetFiltrado.clear();
        for(DeteccionCaraHailo cara : lstDet)
        {
            if (( cara.getAncho() > anchoRostroMinimo ) || ( cara.getAltura() > alturaRostroMinima))
            {
                lstDetFiltrado.push_back(cara);
            }
        }

        // Analiza tracking 
        // lstCaras = detTracker.analizaRapido(&lstDetFiltrado);
        lstCaras = detTracker.generaListaTrackTemporal(&lstDetFiltrado);
        auto finDeteccion = std::chrono::high_resolution_clock::now();

        // genera descriptores faciales
        calculaDescriptores( &generadorDesc, imagenCapturada, &lstCaras);
        if ( generadorDesc.errorCalculo == true )
        {
            cout << "Error al generar descriptores faciales " << endl;
            break;
        }
        auto finDescriptores = std::chrono::high_resolution_clock::now();

        // Busca coincidencias de las detecciones con el universo de personas conocidas
        identificarPersonas(&lstCaras);
        auto finIdentificacion = std::chrono::high_resolution_clock::now();

        // Hace el tracking de las caras procesadas
        lstCarasTrack = detTracker.analizaPorIdentificacion(&lstCaras);

        // Notifica al servidor web las incidencias o detecciones
        notificaDetecciones(imagenCapturada, &lstCarasTrack);
        auto finNotifica = std::chrono::high_resolution_clock::now();
        
        imagenVisor = dibujaCaras(imagenCapturada,&lstCarasTrack);
        auto finDibuja = std::chrono::high_resolution_clock::now();

        universoPersonas.eliminaDesAntiguos(tiempoMaxNoReconocido);

        // calcula los tiempos transcurridos
        auto durCaptura = std::chrono::duration_cast<std::chrono::microseconds>(finCaptura - inicio);
        auto durDetecta = std::chrono::duration_cast<std::chrono::microseconds>(finDeteccion - finCaptura);
        auto durDescriptor = std::chrono::duration_cast<std::chrono::microseconds>(finDescriptores - finDeteccion);
        auto durIdentificacion = std::chrono::duration_cast<std::chrono::microseconds>(finIdentificacion - finDescriptores);
        auto durNotifica = std::chrono::duration_cast<std::chrono::microseconds>(finNotifica - finIdentificacion);
        auto durDibuja = std::chrono::duration_cast<std::chrono::microseconds>(finDibuja - finNotifica);
        auto durTotal = std::chrono::duration_cast<std::chrono::microseconds>(finDibuja - inicio);

        // cout << "Tiempos microsegundos " << endl << endl;
        // cout << "Captura      : " << durCaptura.count() << endl;
        // cout << "Detecta      : " << durDetecta.count() << endl;
        // cout << "Descriptores : " << durDescriptor.count() << endl;
        // cout << "Identifica : " << durIdentificacion.count() << endl;
        // cout << "notifica : " << durNotifica.count() << endl;
        // cout << "Dibujo       : " << durDibuja.count() << endl;
        // cout << "** TOTA      : " << durTotal.count() << endl;
        // cout << endl;

        // Envia la imagen al servidor web de configuracion
        // ProcesoRecFacial::lstUltCarasDet = lstCaras;
        ProcesoRecFacial::lstUltCarasDet = lstCarasTrack;
        servidorWeb->setImage(imagenVisor);
        
        // Envia la imagen a la pantalla local
        GDibujo::show(imagenVisor, "Visor");        
        
        // configura el procesar de eventos del mouse
        cv::setMouseCallback("Visor", onMouse, this);

        // Lee el teclada para finalziar si presiona ESC
        if ( GDibujo::waitForKey(10) == 'q' )
        {
            break;
        }

        // SystemUtils::printRamUsage();
    }
    
    cout << "Finaliza Bucle reconocimiento" << endl;
    GDibujo::closeAllWindows();    
    imageSource->stop();    
    generadorEventos.finalizar();
    
    universoPersonas.lstPerIdentificadas.reset();
    universoPersonas.lstPerNoIdent.reset();
    detTracker.lstUniverso.reset();
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
    TrackedDetectionHailo *deteccion;
    int n = lstRostros->size();
        
    for(int i=0; i<n; i++)
    {
        deteccion = lstRostros->at(i);        
              
        // calcula la deteccion relativa al rectangulo en el que se detecto
        deteccion->cara.detRelCara = deteccion->cara.deteccion;            
        deteccion->cara.detRelCara.desplazaPtosCara(-deteccion->cara.deteccion.ptoSupIzq.x, -deteccion->cara.deteccion.ptoSupIzq.y);

        deteccion->cara.descCalculado = true;        
        deteccion->cara.calculaFotoCara(foto);
        
        generador->calculaDescriptor(deteccion->cara.fotoCara, deteccion->cara.detRelCara, deteccion->cara.descriptor);
    }
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
    n = lstRostros->size();
    for(i=0; i<n; i++)
    {
        deteccion = lstRostros->at(i);
        if ( deteccion->ciclosNoDetectados > 0 )
        {
            continue;
        }            

        // solo se identifica a la persona si es nueva        
        personaConocida = universoPersonas.buscaPersonaCon(deteccion->cara.descriptor, toleranciaIden, &distEucli, usarDistEuclideana);
        if ( personaConocida != NULL )
        {
            IdentificacionPersona iden;

            iden.fecDet = fecDet;
            std::memcpy(iden.vecDescripcion,deteccion->cara.descriptor,512*sizeof(SIMD_TYPE));
            iden.comparacion = distEucli;
            deteccion->cara.personaIdent = true;                        
            deteccion->cara.identificador.agregaIdentif(personaConocida, iden , fecDet);

            idenPersonaValidaDec = deteccion->cara.identificador.getUltimaIdentificacion();            
        }
        else
        {
            personaConocida = universoPersonas.buscaPersonaDesc(deteccion->cara.descriptor, toleranciaIden, &distEucli, usarDistEuclideana);
            if ( personaConocida != NULL )
            {
                IdentificacionPersona iden;

                iden.fecDet = fecDet;
                std::memcpy(iden.vecDescripcion,deteccion->cara.descriptor,512*sizeof(SIMD_TYPE));
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
                std::memcpy(iden.vecDescripcion,deteccion->cara.descriptor,512*sizeof(SIMD_TYPE));
                iden.comparacion = 100;
                deteccion->cara.personaIdent = true;            
                
                DescPersonaExterno personaExterna;

                personaExterna.anonimo = true;
                std::memcpy(personaExterna.vecDescripcion,iden.vecDescripcion,512*sizeof(SIMD_TYPE));
                personaExterna.id = std::to_string(idDesconocidoSgte);
                personaExterna.nombre = "Desc." + personaExterna.id;
                personaExterna.ultimaFechaDetectada = fecDet;
                idDesconocidoSgte++;
                universoPersonas.lstPerNoIdent.add(personaExterna);
                
                deteccion->cara.identificador.agregaIdentif(universoPersonas.lstPerNoIdent.getAddrUltimo(), iden , fecDet);
                idenPersonaValidaDec = deteccion->cara.identificador.getUltimaIdentificacion();
            }
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
    GRect caja;
    GColor amarillo(0,255,255);
    GColor rojo(255,0,0);
    GColor verde(0,255,0);
    GColor azul(0,0,255);
    GColor blanco(255,255,255);
    DescPersonaExterno *datosPersona;
    string id;
    int temp;
    float escalaX,escalaY;

    imagenVisor = imagen.cloneResize(anchoVisualiza, alturaVisualiza);
    escalaX = ((float)anchoVisualiza)/((float)anchoCamara);
    escalaY = ((float)alturaVisualiza)/((float)alturaCamara);

    int n = lstCaras->size();
    for(int i=0; i<n; i++ )
    {
        det = lstCaras->at(i);
        id = to_string(det->id);
        cara = &det->cara;

        caja.x1 = cara->deteccion.ptoSupIzq.x * escalaX;
        caja.y1 = cara->deteccion.ptoSupIzq.y * escalaY;
        caja.x2 = cara->deteccion.ptoInfDer.x * escalaX;
        caja.y2 = cara->deteccion.ptoInfDer.y * escalaY;

        if ( cara->descCalculado == false )
        {
            GDibujo::drawRect(imagenVisor, caja, amarillo, 2);        
        }
        else
        if ( cara->personaIdent == false )
        {
            GDibujo::drawRect(imagenVisor, caja, rojo, 2);
        }
        else
        {
            datosPersona = det->cara.identificador.getDatosPerIden();            
            if ( datosPersona != NULL )
            {
                id.append(" - ");
                id.append(datosPersona->nombre);
                id.append(" : ");
                id.append(to_string(det->cara.identificador.getUltimaIdentificacion().comparacion));
            }                
            if ( det->cara.identificador.getDatosPerIden()->anonimo == false ) GDibujo::drawRect(imagenVisor, caja, verde, 2);
            else GDibujo::drawRect(imagenVisor, caja, rojo, 2);
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

        GDibujo::drawRect(imagenVisor, caja, blanco, -1 );
        GDibujo::drawText(imagenVisor, caja.x1, caja.y1+30, id , GDIBUJO_FONT_HELVETICA, azul, 0.9, 2 );
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
            for(int j=0;j<512; j++)
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
}


/**
 * Notifica al servidor web sobre las detecciones ocurridas
 */
void ProcesoRecFacial::notificaDetecciones( GImage imagen, vector<TrackedDetectionHailo *> *lstCaras )
{
    int i,n,j;
    int bufferLen;
    size_t bufferLen64;
    string jsonLstRostros,nombres;
    TrackedDetectionHailo *persona;
    char *bufferImgVisor, *bufferImgVisor64;    
    long long delta, fecEvento = TimeDateUtils::getDateTimeMs();

    n = lstCaras->size();
    // bucle para generar datos de perasonas conocidas (bucleTipoPer=0) y para personas no idenentificadas (bucleTipoPer=1)
    for(int bucleTipoPer=0; bucleTipoPer < 2; bucleTipoPer++)
    {
        jsonLstRostros = "";
        for(i=0; i<n; i++)
        {
            persona = lstCaras->at(i);

            // se valida si el tipo de persona pertnece al bucle, si no se ignora
            if (( bucleTipoPer == 0 ) && ( persona->cara.identificador.getDatosPerIden()->anonimo == true )) continue;
            else
            if (( bucleTipoPer == 1 ) && ( persona->cara.identificador.getDatosPerIden()->anonimo == false )) continue;
            
            delta = fecEvento-persona->cara.identificador.fechaUltEvento;
            if (( persona->cara.identificador.cambioIdentificacion == true ) || 
                (( persona->cara.personaIdent == true ) && ( delta >= tiempoReEvento )))
            {
                // persona->cara.identificador.cambioIdentificacion = false;
                persona->cara.identificador.fechaUltEvento = fecEvento;

                nombres.append(persona->cara.identificador.getDatosPerIden()->nombre);
                nombres.append(",");

                bufferImgVisor = GDibujo::encode(persona->cara.fotoCara, GDIBUJO_ENCODE_JPEG, 70, bufferLen);
                bufferImgVisor64 = base64_encode((const unsigned char *)bufferImgVisor, bufferLen, &bufferLen64);
                free(bufferImgVisor);

                if ( jsonLstRostros.length() > 0 ) 
                    jsonLstRostros.append(",");

                jsonLstRostros.append("\n{");
            
                // jsonLstRostros.append("\"foto\":\"");
                // jsonLstRostros.append(bufferImgVisor64);
                // jsonLstRostros.append("\",\n");
                GStringUtils::addJsonAtt(&jsonLstRostros,"foto", bufferImgVisor64,false);
                // GStringUtils::addJsonAtt(&jsonLstRostros,"foto", "CARA",false);
                free(bufferImgVisor64);

                jsonLstRostros.append("\"coordenadas\":{");
        
                // jsonLstRostros.append("\"x1\":");
                // jsonLstRostros.append(to_string(persona->cara.deteccion.ptoSupIzq.x));
                // jsonLstRostros.append(",\n");
                GStringUtils::addJsonAtt(&jsonLstRostros,"x1",to_string(persona->cara.deteccion.ptoSupIzq.x),true);
        
                // jsonLstRostros.append("\"y1\":");
                // jsonLstRostros.append(to_string(persona->cara.deteccion.ptoSupIzq.y));
                // jsonLstRostros.append(",\n");
                GStringUtils::addJsonAtt(&jsonLstRostros,"y1",to_string(persona->cara.deteccion.ptoSupIzq.y),true);
        
                // jsonLstRostros.append("\"x2\":");
                // jsonLstRostros.append(to_string(persona->cara.deteccion.ptoInfDer.x));
                // jsonLstRostros.append(",\n");
                GStringUtils::addJsonAtt(&jsonLstRostros,"x2",to_string(persona->cara.deteccion.ptoInfDer.x),true);
        
                // jsonLstRostros.append("\"y2\":");
                // jsonLstRostros.append(to_string(persona->cara.deteccion.ptoInfDer.y));
                GStringUtils::addJsonAtt(&jsonLstRostros,"y2",to_string(persona->cara.deteccion.ptoInfDer.y),true,false);
                
                jsonLstRostros.append("},\n");
                jsonLstRostros.append("\"descripcionFacial\":[\n");
        
                for(j=0; j<512; j++)
                {            
                    jsonLstRostros.append(to_string(persona->cara.descriptor[j]));
                    if ( j < 511 ) jsonLstRostros.append(",");
                }
        
                jsonLstRostros.append("\n],\n");

                if ( persona->cara.identificador.getDatosPerIden()->anonimo == false )
                {
                    // jsonLstRostros.append("\"idReconocido\":");
                    // jsonLstRostros.append(persona->cara.identificador.getDatosPerIden()->id);            
                    // jsonLstRostros.append(",\n");    
                    GStringUtils::addJsonAtt(&jsonLstRostros,"idReconocido",persona->cara.identificador.getDatosPerIden()->id,true);
                    jsonLstRostros.append("\"nuevo\":false");    
                }
                else
                {
                    jsonLstRostros.append("\"nuevo\":true");    
                }                                
                jsonLstRostros.append("}\n");
            }
        }   

        if ( jsonLstRostros.size() > 0 )
        {
            // Si hay detecciones nuevas, se debe generar un evento
            string trama;

            bufferImgVisor = GDibujo::encode(imagen, GDIBUJO_ENCODE_JPEG, 80, bufferLen);
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

            trama.append("\"lstRostros\":[");    
            trama.append(jsonLstRostros);
            trama.append("]\n");
            trama.append("}\n");
            
            cout << "Reconocidos: " << nombres << endl;

            // cout << "Trama:" <<  trama << endl;

            if ( bucleTipoPer == 0 ) generadorEventos.agregarTramaIden(trama);
            else generadorEventos.agregarTramaNoIden(trama);
        }
    }
}
