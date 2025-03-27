
#include "app/ProcRecFacial.h"
#include "lib/graphics/GDibujo.h"
#include "lib/graphics/ImageSource.h"
#include "lib/utils/timedate.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/base64.h"
#include "lib/utils/GStringUtils.h"
#include "lib/web/GHttpClient.h"
#include "lib/hailolib/DetectionTrackerHailo.h"

#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <stdio.h>
#include <iostream>

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
    vector<TrackedDetectionHailo *> lstCaras;
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
        lstCaras = detTracker.analizaRapido(&lstDet);
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

        // Notifica al servidor web las incidencias o detecciones
        notificaDetecciones(imagenCapturada, &lstCaras);
        auto finNotifica = std::chrono::high_resolution_clock::now();

        imagenVisor = dibujaCaras(imagenCapturada,&lstCaras);
        auto finDibuja = std::chrono::high_resolution_clock::now();

        // calcula los tiempos transcurridos
        auto durCaptura = std::chrono::duration_cast<std::chrono::microseconds>(finCaptura - inicio);
        auto durDetecta = std::chrono::duration_cast<std::chrono::microseconds>(finDeteccion - finCaptura);
        auto durDescriptor = std::chrono::duration_cast<std::chrono::microseconds>(finDescriptores - finDeteccion);
        auto durIdentificacion = std::chrono::duration_cast<std::chrono::microseconds>(finIdentificacion - finDescriptores);
        auto durNotifica = std::chrono::duration_cast<std::chrono::microseconds>(finNotifica - finIdentificacion);
        auto durDibuja = std::chrono::duration_cast<std::chrono::microseconds>(finDibuja - finNotifica);
        auto durTotal = std::chrono::duration_cast<std::chrono::microseconds>(finDibuja - inicio);

        //cout << "Tiempos microsegundos " << endl << endl;
        //cout << "Captura      : " << durCaptura.count() << endl;
        //cout << "Detecta      : " << durDetecta.count() << endl;
        //cout << "Descriptores : " << durDescriptor.count() << endl;
        //cout << "Identifica : " << durIdentificacion.count() << endl;
        //cout << "notifica : " << durNotifica.count() << endl;
        //cout << "Dibujo       : " << durDibuja.count() << endl;
        //cout << "** TOTA      : " << durTotal.count() << endl;
        //cout << endl;

        // Envia la imagen al servidor web de configuracion
        ProcesoRecFacial::lstUltCarasDet = lstCaras;
        servidorWeb->setImage(imagenVisor);
        
        // Envia la imagen a la pantalla local
        GDibujo::show(imagenVisor, "Visor");        
        
        // configura el procesar de eventos del mouse
        cv::setMouseCallback("Visor", onMouse, this);

        // Lee el teclada para finalziar si presiona ESC
        if ( GDibujo::waitForKey(1) == 27 )
        {
            break;
        }
    }
    
    cout << "Finaliza Bucle reconocimiento" << endl;
    GDibujo::closeAllWindows();    
    imageSource->stop();
    generadorEventos.finalizar();
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
        if (( deteccion->cara.deteccion.getAncho() < anchoRostroMinimo ) && ( deteccion->cara.deteccion.getAltura() < alturaRostroMinima ))
        {
            // la cara es muy chica
            deteccion->cara.descCalculado = false;
            // deteccion->cara.personaIdent = false;
        }
        else 
        {            
            deteccion->cara.detRelCara = deteccion->cara.deteccion;            
            deteccion->cara.detRelCara.desplazaPtosCara(-deteccion->cara.deteccion.ptoSupIzq.x, -deteccion->cara.deteccion.ptoSupIzq.y);

            deteccion->cara.descCalculado = true;        
            deteccion->cara.calculaFotoCara(foto);
            
            deteccion->cara.descriptor = generador->calculaDescriptor(deteccion->cara.fotoCara, deteccion->cara.detRelCara);
        }
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
        personaConocida = universoPersonas.buscaPersonaCon(&deteccion->cara.descriptor, toleranciaIden, &distEucli);
        if ( personaConocida != NULL )
        {
            IdentificacionPersona iden;

            iden.fecDet = fecDet;
            iden.vecDescripcion = deteccion->cara.descriptor;
            iden.comparacion = distEucli;
            deteccion->cara.personaIdent = true;            
            deteccion->cara.identificador.agregaIdentif(personaConocida, iden , fecDet);
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
    CaraDescrita cara;
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
        cara = det->cara;

        caja.x1 = cara.deteccion.ptoSupIzq.x * escalaX;
        caja.y1 = cara.deteccion.ptoSupIzq.y * escalaY;
        caja.x2 = cara.deteccion.ptoInfDer.x * escalaX;
        caja.y2 = cara.deteccion.ptoInfDer.y * escalaY;

        if ( cara.descCalculado == false )
        {
            GDibujo::drawRect(imagenVisor, caja, amarillo, 2);        
        }
        else
        if ( cara.personaIdent == false )
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
            }                
            GDibujo::drawRect(imagenVisor, caja, verde, 2);
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
                cout << track->cara.descriptor.at(j) << ",";
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
    for(i=0; i<n; i++)
    {
        persona = lstCaras->at(i);
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
        
            jsonLstRostros.append("\"foto\":\"");
            jsonLstRostros.append(bufferImgVisor64);
            jsonLstRostros.append("\",\n");
            free(bufferImgVisor64);
    
            jsonLstRostros.append("\"coordenadas\":{");
    
            jsonLstRostros.append("\"x1\":");
            jsonLstRostros.append(to_string(persona->cara.deteccion.ptoSupIzq.x));
            jsonLstRostros.append(",\n");
    
            jsonLstRostros.append("\"y1\":");
            jsonLstRostros.append(to_string(persona->cara.deteccion.ptoSupIzq.y));
            jsonLstRostros.append(",\n");
    
            jsonLstRostros.append("\"x2\":");
            jsonLstRostros.append(to_string(persona->cara.deteccion.ptoInfDer.x));
            jsonLstRostros.append(",\n");
    
            jsonLstRostros.append("\"y2\":");
            jsonLstRostros.append(to_string(persona->cara.deteccion.ptoInfDer.y));
            
            jsonLstRostros.append("},\n");
    
            jsonLstRostros.append("\"descripcionFacial\":[\n");
    
            for(j=0; j<512; j++)
            {            
                jsonLstRostros.append(to_string(persona->cara.descriptor[j]));
                if ( j < 511 ) jsonLstRostros.append(",");
            }
    
            jsonLstRostros.append("\n],\n");
            jsonLstRostros.append("\"idReconocido\":");
            jsonLstRostros.append(persona->cara.identificador.getDatosPerIden()->id);            
            jsonLstRostros.append(",\n");
    
            jsonLstRostros.append("\"nuevo\":true");    
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
        trama.append("\"cuadro\":\"");
        trama.append(bufferImgVisor64);
        trama.append("\",\n");

        free(bufferImgVisor64);

        trama.append("\"serieEquipo\":\"");
        trama.append(GStringUtils::trim(idEquipo));
        trama.append("\",\n");

        trama.append("\"lstRostros\":[");    
        trama.append(jsonLstRostros);
        trama.append("]\n");
        trama.append("}\n");
        
        cout << "Reconocidos: " << nombres << endl;
        generadorEventos.agregarTrama(trama);
    }
}
