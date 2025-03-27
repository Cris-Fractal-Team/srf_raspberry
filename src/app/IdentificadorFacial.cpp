
#include <memory>
#include <iostream>
#include <chrono>
#include <sys/resource.h>

#include "lib/general/GVector.h"
#include "app/IdentificadorFacial.h"
#include "lib/graphics/GDibujo.h"
#include "lib/deeplearning/DetectorBO.h"
#include "lib/deeplearning/DetectorImageSource.h"
#include "lib/deeplearning/ThreadDetector.h"
#include "lib/deeplearning/DetectionTracker.h"
#include "lib/utils/timedate.h"
#include "lib/utils/base64.h"
#include "lib/utils/fileutils.h"
#include "lib/facerec/facerecyn.h"
#include "lib/graphics/imageutils.h"
#include "app/EmisorImagenesHTTP.h"
#include "lib/web/GHttpClient.h"

/**
 * Referencia a la instancia de Identificador facial
 */
IdentificadorFacial *ptrIdentificadorFacial = NULL;

/**
 * Procesa eventos del mouse
 */
void procesaEventoMouseDetFas( int event, int x, int y, int flags, void *userdata )
{
    if ( ptrIdentificadorFacial != NULL )
    {
        ptrIdentificadorFacial->mouseEvent(event,x,y,userdata);
    }
}


/**
 * Constructor
 */
IdentificadorFacial::IdentificadorFacial()
{
    // inicializa variables globales   
    maxRosPorThCalcDesc = 10;    
    lstRostrosConocidos = make_shared<GVector>();
    toleranciaIden = 0.55;
    tiempoRenovacionMs = 1000;
    toleranciaDetec = 0.6;
}


/**
 * Carga los rostros conocidos desde un archivo de texto
 * Donde cada fila tiene los datos de una persona conocida, separados por coma
 * El primer elemento de una fila es un texto que identifica a la persona.
 * Desde el 2do elemento hasta el final de un archivo de texto se tienen
 * los valores del decripctor del rostro.
 */
void IdentificadorFacial::cargaRostrosConocidos( string path )
{
    std::ifstream file(path);

    if ( !file.is_open() )
    {
        cout << "No se pudo leer el archivo con rostros conocidos" << endl;
        return;
    }

    string line,nombre,dato;
    shared_ptr<FaceDescriptor> descriptor;
    float *datos;
    int pos;

    while( getline(file, line))
    {
        descriptor = make_shared<FaceDescriptor>();
        descriptor->setSize(128);
        datos = descriptor->getFloats();

        try
        {
            stringstream ss(line);
            pos = -2;
            while( pos < 128 )
            {
                getline(ss, dato, ',');
                if ( pos == -2 )
                {
                    descriptor->textDesc = dato;
                }
                else
                if ( pos == -1 )
                {
                    descriptor->serverFaceId = dato;
                }
                else
                {
                    datos[pos] = stof(dato);
                }
                pos++;
            }
            lstRostrosConocidos->add(descriptor);
        }
        catch(const std::exception& e)
        {
            cout << "Error al leer el descriptor de rostro conocido " <<  line  << endl;
        }        
    }
    file.close();

    cout << "Se leyeron " << lstRostrosConocidos->size() << " rostros conocidos" << endl;
}


/**
 * Inicia el proceso
 */
void IdentificadorFacial::iniciar()
{
    bool finalizar = false;
    long long idSgteImg = 0;    
    double ratioOscuro, ratioClaro, correccionGamma, anguloGiro;
    GImage imagen, imagenVis, imagenVisWeb;    
    shared_ptr<ImageSource> source = imgFactory->getInstance();

    this->imageSource = source;

    // Inicia el servidor web
    servidorWeb = make_shared<ServidorHttpImagenes>();
    servidorWeb->lstParamsApp = this->lstParamsApp;
    servidorWeb->identificadorFac = this;
    servidorWeb->iniciar();
    
    // Inicia el thread que procesa las descripciones calculadas
    thProcDescFaciales.identificadorFacial = this;    
    thProcDescFaciales.mtxExterno = &this->mtx;
    thProcDescFaciales.start();

    // Inicializa el thread que calcula descripciones de rostros
    thCalculadorDesc.listener = &thProcDescFaciales;
    thCalculadorDesc.mtxExterno = &this->mtx;
    thCalculadorDesc.start();

    // Inicializa el thread que genera eventos al servidor por detecciones
    // thGenEventos.start();

    // Inicializa el thread detector
    ThreadDetector thDetector;     
    thDetector.detector = this->detectorCaras;   

    // Inicializa la fuente de imagenes
    DetectorImageSource imageSource;    
    imageSource.setDimImgModelo(thDetector.detector->anchoImgModelo, thDetector.detector->alturaImgModelo);
    
    // Calcula los factores de escalamiento para visualizar
    factorHorVisualiza = ((float)anchoVisualiza) / ((float)anchoCamara);
    factorVerVisualiza = ((float)alturaVisualiza) / ((float)alturaCamara);

    // Inicia el Thread de detecciones
    thDetector.detector->setDimImgOrig(anchoCamara,alturaCamara);
    thDetector.imageSource = &imageSource;
    cout << "Solicita iniciar Thread detector" << endl;
    thDetector.start();

    // Referencia a las detecciones
    GLinkedList<DeteccionVO> detecciones; 
    GLinkedList<DeteccionVO> deteccionesTmp;
    GLinkedList<DeteccionVO> lstDet;

    // Inicializa el seguidor de rostros
    DetectionTracker tracker;
    tracker.maxCiclosInactivo = 7;

    ptrIdentificadorFacial = this;
    cv::namedWindow("visor", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("visor", procesaEventoMouseDetFas);

    std::chrono::milliseconds intervaloEspera(5);

    lstBorradosTracking = make_shared<GVector>();
    
    cout << "********** Inicia bucle principal" << endl;
    
    // bucle que muestra las imagenes de la camara
    
    long delta;
    long long t1 = 0;
    long long t2 = 0;
    while( finalizar == false )
    {
        imagen = source->getImage();
        if (( imagen.imagenOpencv.empty()) || (( imagen.ancho == 0 ) || ( imagen.altura == 0 )))
        {
            sleep(1);
            continue;
        }

        // cout << "Se tiene una imagen desde camara " << imagen.ancho << " x " << imagen.altura << endl;

        // std:this_thread::sleep_for(intervaloEspera);

        // if ( isBacklit(imagen.imagenOpencv,&ratioOscuro,&ratioClaro, 0.4, 200,50 ) == true )
        // {            
        //     correccionGamma = 1.0 - (1.5 * ( ratioOscuro - ratioClaro) / 2.0);
        //     imagen.imagenOpencv = applyGammaCorrection(imagen.imagenOpencv, 0.75);
        // }

        if ( lstParamsApp->getStringBool("reflejarVerticalmente", false) == true )
        {
            // se debe hacer reflejo vertical
            Mat flipped;
            flip(imagen.imagenOpencv, flipped, -1);
            imagen.imagenOpencv = flipped;
        }

        //  cout << "Imagen que se asigna al ImageSource " << imagen.ancho << " x " << imagen.altura << endl;
        
        // imagen.idImagen = (idSgteImg++);
        imageSource.setImage(imagen);

        detecciones = thDetector.getDetecciones();       

        t2 = TimeDateUtils::getDateTimeMs();
        if ( t1 > 0 )
        {
            delta = t2-t1;            
            if ( delta < 67 )
            {                
                detecciones.reset();
                continue;
            }
            t1 = t2;
        }
        t1 = t2;
            
        // Busca los rostros previos comparando las detecciones nuevas
        lstDet = tracker.analizaRapido(detecciones);
        
        imagenVis = imagen.cloneResize(anchoVisualiza, alturaVisualiza);
        // imagenVisWeb = imagenVis.clone();
        procesaDetecciones(lstDet, imagen, imagenVis);
        dibujaRostros(imagenVis);

        // liberamos ram de las listas ya no requeridas
        lstDet.reset();
        detecciones.reset();
       
        // calcula la RAM usada
        std::ifstream file("/proc/self/status");
        std::string line;
        long mem_kb;

        while (std::getline(file, line)) {
            if (line.find("VmRSS:") != std::string::npos) {  // RAM actual usada
                sscanf(line.c_str(), "VmRSS: %ld kB", &mem_kb);
            }
        }
        mem_kb/= 1024;
        // cout << "<<< RAM Usada: " << mem_kb << " MB N.Obj.Creados: " << endl;            
        
        // cout << "Se pasa la imagen de visualizacion al servidor web" << endl;
        servidorWeb->setImage(imagenVis);
        GDibujo::show(imagenVis, "visor");
        if ( GDibujo::waitForKey(1) == 27 )
        {
            cout << ">>>> ESC Detectado, se debe finalizar " << endl;
            source->stop();
            source->release();
            thCalculadorDesc.finalizar();
            thDetector.finalizar();
            finalizar = true;
            break;
        }
    }    
    
    cout << "Cerrando ventanas" << endl;
    GDibujo::closeAllWindows();
    servidorWeb->finalizar();

    cout << "Finaliza bucla Identificador facial" << endl;
}

/**
 * Dibuja las detecciones
 */
void IdentificadorFacial::dibujaRostros( GImage imagen )
{
    int i,n,ancho,altura;
    FaceDetection det;
    shared_ptr<FaceDescriptor> descriptor;
    string id;
    GColor rojo(255,0,0);
    GColor verde(0,255,0);
    GColor blanco(255,255,255);
    GColor azul(0,0,255);
    GRect region,regionTexto;
    GPoint punto;
    
    n = lstRostros.size();

    // cout << "Dibuja Rostros: " << n << endl;

    mtx.lock();
    for(i=0; i < n; i++)
    {
        det = lstRostros.get(i);
        id = to_string(det.deteccion.id);

        region = det.deteccion.region.getClone();
        region.escala(factorHorVisualiza, factorVerVisualiza);

        // cout << "Dim Rostro " << region.getWidth() << " x " << region.getHeight() << endl;

        if (( region.getWidth() < anchoRostroMinVis ) || ( region.getHeight() < alturaRostroMinVis ))
        {
            // cout << "Dim descartada de rostro " <<  endl;
            continue;
        }
               
        if (( region.getWidth() < anchoRostroMinimo ) && ( region.getHeight() < alturaRostroMinima ))
        {         
            // el rostro tiene dimensiones muy chicas, no se puede hacer reconocimiento
            GDibujo::drawText(imagen, region.x1+10, region.y1 + 20,id , GDIBUJO_FONT_HELVETICA, blanco, 1, 1);
            GDibujo::drawRect(imagen, region, rojo, 3);
            GDibujo::drawLine(imagen, GPoint(region.x1, region.y1),
                GPoint(region.x2, region.y2), rojo, 3);

            // cout << "Dibuja rostro muy chico para reconocimiento" << endl;
        }
        else
        // if ( det->descriptor->getSize() == 0 ) 
        if ( det.fecDescriptor == 0 )
        {
            // aun no se hace la deteccion del rostro
            GDibujo::drawText(imagen,region.x1+10, region.y1 + 20,id , GDIBUJO_FONT_HELVETICA, blanco , 1, 1);
            GDibujo::drawRect(imagen, region, verde, 3);

            // cout << "Dibuja rostro no reconocido" << endl;
        }
        else
        {
            // Ya tiene reconocimiento             
            
            id.append(" ");            
            id.append(det.descriptor.textDesc);
            
            regionTexto.x1 = region.x1-10;
            regionTexto.y1 = region.y1-10;
            regionTexto.x2 = region.x1+300;
            regionTexto.y2 = region.y1+30;            
            
            ancho = region.getWidth()/2;
            altura = region.getHeight()/2;
            GDibujo::drawRect(imagen, region, azul, 6);
         
            GDibujo::fillRect(imagen,regionTexto, blanco);
            GDibujo::drawText(imagen,region.x1+10, region.y1 + 20,id , GDIBUJO_FONT_HELVETICA, azul, 1.3,2) ;            
        }       
    }

    mtx.unlock();
    
}

/**
 * Metodo invocado cuando el thread que calcula las descripciones de rostros ha terminado.
 * Se encarga de actualizar los rostros pendientes de reconocimiento y actualizar la lista
 * de rostros pendientes.
 * 
 *  lstFaceDet : lista de detecciones procesada
 */
void IdentificadorFacial::calculoDescFinalizado( GLinkedList<FaceDetection> lstFaceDet )
{
    int i,n,j,k,sizeProc,iProc,nProc;
    long long idProc,idPend,idProcBucle;
    long long ahora = TimeDateUtils::getDateTimeMs();
    bool encontroPersona, encontroProc;
    FaceDetection detProc,detProcBucle,detBuscaConocido;
    FaceDetection *detPend;
    GLinkedList<FaceDetection> lstFaceDetProc;
    FaceDescriptor faceDesc;
           
    try
    {
        n = lstFaceDet.size();    
        mtx.lock();
        k = lstRostrosPend.size();        
        mtx.unlock();
        cout << ">>> Calculo Desc Finalizado : " << n << " Pend :" << k << endl;

        // rastrea cada rosotro al que se le calcula el descriptor
        for(i=0; i<n; i++)
        {
            detProc = lstFaceDet.get(i);
            idProc = detProc.deteccion.id;
            sizeProc = detProc.descriptor.getSize();
            if ( sizeProc == 0 )
            {
                cout << ">>>>>>>>>>>>> Se detecto cara sin descriptor !!!" << endl;
                continue;
            }

            // busca los rostros pendientes de procesar para asociar el ID calculado
            mtx.lock();
            k = lstRostrosPend.size();
            // cout << "Rostros pendientes : " << k << endl;
            mtx.unlock();
            j = k-1;
            encontroProc = false;
            while( j >= 0 )
            {
                detPend = lstRostrosPend.getAddr(j);
                idPend = detPend->deteccion.id;

                if ( idPend == idProc ) 
                {        
                    // Se encontro el rostro en la lista de rostros pendientes                                                
                    faceDesc = detProc.descriptor;
                    detPend->descriptor = faceDesc;

                    // busca si en la lista de rostros procesados ya se encontro un rostro conocido
                    // es para evitar que se recalcule el rostro
                    nProc = lstFaceDetProc.size();
                    encontroPersona = false;
                    for(iProc=0; iProc<nProc; iProc++)
                    {                        
                        detProcBucle = lstFaceDetProc.get(iProc);
                        idProcBucle = detProcBucle.deteccion.id;
                        
                        if ( idProcBucle == idPend )
                        {                            
                            encontroPersona = true;
                            mtx.lock();                                               
                            detPend->fecDescriptor = ahora;
                            detPend->descriptor.textDesc = detProcBucle.descriptor.textDesc;      
                            detPend->descriptor.serverFaceId = detProcBucle.descriptor.serverFaceId;            
                            // cout << ".. Ya existe DESC DF " <<   detPend->deteccion->id << " " << detPend->descriptor->textDesc << endl;
                            mtx.unlock();
                            break;
                        }                        
                    }

                    if ( encontroPersona == false )
                    {
                        long long iniBusca = TimeDateUtils::getDateTimeMs();                         
                        buscaRostroConocido(&detProc);
                        long long finBusca = TimeDateUtils::getDateTimeMs();                           
                                                
                        mtx.lock();                        
                        detPend->fecDescriptor = ahora;                      
                        detPend->descriptor.textDesc = detProc.descriptor.textDesc;
                        detPend->descriptor.serverFaceId = detProc.descriptor.serverFaceId;
                        mtx.unlock();                       
                    }                                        
                                        
                    lstFaceDetProc.add(*detPend);                      

                    mtx.lock();                                                            
                    lstRostrosPend.remove(j);   
                    mtx.unlock();
                    j--;
                    if ( k == 0 )
                    {
                        break;
                    }
                }
                else
                {
                    j--;                
                }
            }
        }                  
    }
    catch( const exception &e )
    {

    }

    // calcula la RAM usada
    std::ifstream file("/proc/self/status");
    std::string line;
    long mem_kb;

    while (std::getline(file, line)) {
        if (line.find("VmRSS:") != std::string::npos) {  // RAM actual usada
            sscanf(line.c_str(), "VmRSS: %ld kB", &mem_kb);
        }
    }
    mem_kb/= 1024;
    
    mtx.lock();
    k = lstRostrosPend.size();
    cout << "<<< Fin Reconocimiento, pendientes: " << k << " " << mem_kb << " MB " << endl;            
    mtx.unlock();

    k = lstFaceDetProc.size();
    if ( k > 0 )
    {       
        // Actualiza la descripcion de los rostros que se deben dibujar
        mtx.lock();
        n = lstRostros.size();        
        for(i=0;i<n;i++)
        {
            detPend = lstRostros.getAddr(i);
            for(j=0;j<k;j++)
            {
                detProc = lstFaceDetProc.get(j);
                if ( detProc.deteccion.id == detPend->deteccion.id )
                {
                    detPend->descriptor = detProc.descriptor;
                    detPend->fecDescriptor = detProc.fecDescriptor;
                    break;
                }
            }
        }
        mtx.unlock();
        lstFaceDetProc.reset();

        // cout << "Inicia envio de detecciones " << lstFaceDetProc.size() << endl;
        // generaEventoDetecciones(lstFaceDetProc);       
    }  
}

/**
 * Solicita al Thread que genera eventos de notificacion
 * que se notifique al servidor la ocurrencia de detecciones
 * 
 *  lstFaceDetProc : lista on las detecciones (Instancias de FaceDetection) a las que se les hizo el calculo
 *              de descriptores faciales
 */
void IdentificadorFacial::generaEventoDetecciones( GLinkedList<FaceDetection> lstFaceDetProc )
{
    int i,j;
    int numRostros = lstFaceDetProc.size();
    FaceDetection face;
    string url;
    string trama = "{";
    string idEquipo = "SRF-FRAC-001";
    float *descFacial;
    char *bufferImgVisor, *bufferImgVisor64;
    GHttpClient httpClient;
    int bufferLen;
    size_t bufferLen64;

    if ( lstFaceDetProc.size() == 0 )
        return;

    face = lstFaceDetProc.get(0);
    // GDibujo::write(face->imagenVisor, "/home/mchu/imagenVisor.jpg",0);
    bufferImgVisor = GDibujo::encode(face.imagenVisor, GDIBUJO_ENCODE_JPEG, 0, bufferLen);
    bufferImgVisor64 = base64_encode((const unsigned char *)bufferImgVisor, bufferLen, &bufferLen64);
    free(bufferImgVisor);
    
    url = lstParamsApp->getString("urlBaseServidor");

    trama.append("\"cuadro\":\"");
    trama.append(bufferImgVisor64);
    trama.append("\",\n");

    free(bufferImgVisor64);

    trama.append("\"codEquipo\":\"");
    trama.append(idEquipo);
    trama.append("\",\n");

    trama.append("\"lstRostros\":[");      

    for(i=0; i < numRostros; i++ )
    {
        face = lstFaceDetProc.get(i);

        // GDibujo::write(face.deteccion.imagen, "/home/mchu/imagenCara.jpg",0);
        bufferImgVisor = GDibujo::encode(face.deteccion.imagen, GDIBUJO_ENCODE_JPEG, 0, bufferLen);
        bufferImgVisor64 = base64_encode((const unsigned char *)bufferImgVisor, bufferLen, &bufferLen64);
        free(bufferImgVisor);
        
        trama.append("\n{");
        
        trama.append("\"foto\":\"");
        trama.append(bufferImgVisor64);
        trama.append("\",\n");
        free(bufferImgVisor64);

        trama.append("\"coordenadas\":{");

        trama.append("\"x1\":");
        trama.append(to_string(face.deteccion.region.x1));
        trama.append(",\n");

        trama.append("\"y1\":");
        trama.append(to_string(face.deteccion.region.y1));
        trama.append(",\n");

        trama.append("\"x2\":");
        trama.append(to_string(face.deteccion.region.x2));
        trama.append(",\n");

        trama.append("\"y2\":");
        trama.append(to_string(face.deteccion.region.y2));
        
        trama.append("},\n");

        trama.append("\"descripcionFacial\":[\n");
        descFacial = face.descriptor.getFloats();

        for(j=0; j<128; j++)
        {            
            trama.append(to_string(descFacial[j]));
            if ( j < 127 ) trama.append(",");
        }

        trama.append("\n],\n");
        trama.append("\"idReconocido\":");
        if ( face.descriptor.serverFaceId.length() > 0 ) trama.append(face.descriptor.serverFaceId);
        else trama.append("0");
        trama.append(",\n");

        trama.append("\"nuevo\":false");

        trama.append("}\n");

        if ( (i+1) < numRostros ) trama.append(",");
    }

    trama.append("]\n");
    trama.append("}\n");

    // cout << ">>>>>>>>>>>> Trama evento :" << trama << endl;

    httpClient.setHeader("Content-Type","application/json");
    i = httpClient.doHttp(url, "POST", (char *)trama.c_str(), trama.length());

    // cout << "Resultado de Evento: " << i << endl;
}

/**
* Busca si un rostro conocido coincide con un rostro detectado
*/
void IdentificadorFacial::buscaRostroConocido( FaceDetection *face )
{
    int i,n,ndx;
    float delta,delta2,deltaMin,deltaDist;
    shared_ptr<FaceDescriptor> conocido;

    n = lstRostrosConocidos->size();
    ndx = -1;
    deltaMin = 200;

    for(i=0;i<n;i++)
    {
        conocido = dynamic_pointer_cast<FaceDescriptor>(lstRostrosConocidos->get(i));
        // delta = conocido->calculaDiferencia(face->descriptor);
        delta = conocido->calculaDiferenciaSIMD(&face->descriptor);
                
        if (( delta < 1.128 ) && ( delta < deltaMin ))
        {
            ndx = i;
            deltaMin = delta;                        
            break;
        }
    }

    if ( ndx != -1 )
    {
        conocido = dynamic_pointer_cast<FaceDescriptor>(lstRostrosConocidos->get(ndx));
        face->descriptor.textDesc = conocido->textDesc;      
        face->descriptor.serverFaceId = conocido->serverFaceId;               
    }
    else
    {
        face->descriptor.textDesc = "";      
        face->descriptor.serverFaceId = "";          
    }    
}


/**
 * Procesa las detecciones actuales
 * 
 * lstDet : lista de detecciones que se deben procesar
 * 
 * imagen : imagen desde la que se obtuvieron las detecciones
 * 
 * imagenVisor : imagen del visor que se envia cuando ocurre un evento
 */
void IdentificadorFacial::procesaDetecciones( GLinkedList<DeteccionVO> lstDet, GImage imagen, GImage imagenVisor  )
{
    bool encontro;
    FaceDetection rostro,rostroNuevo,*ptrRostro;
    DeteccionVO det;    
    GImage imagenNula,imgTmp;
    int i,n,j,k;
    long long ahora = TimeDateUtils::getDateTimeMs();    

    // numero de rostros que fueron detectados anteriormente
    k = lstRostros.size();
    
    // numero de rostros en la deteccion actual
    n = lstDet.size();

    // cout << ahora << " Proc.Det.: Num Rostros " << k << " Num Det " << n << endl;
    // cout << "Procesa Detecciones, Num. Existentes: " << k << " Num. Detecciones: " << n << endl;

    if ( k == 0 )
    {
        // No hay rostros previos
        try
        {
            for(i=0; i < n; i++ )
            {
                det = lstDet.get(i);

                if (( det.region.getWidth() < anchoRostroMinVis ) || ( det.region.getHeight() < alturaRostroMinVis ))
                {
                    // se descarta la deteccion por ser muy chica
                    continue;
                }

                rostro = FaceDetection();
                rostro.deteccion = det;       
                rostro.fecDetect = ahora;                              

                // opriginalmente una deteccion tiene la imagen de la camara no el elemento detectado
                imgTmp = rostro.deteccion.imagen.getRect(det.region);
                
                // Se reemplaza la imagen de la deteccion por la parte que representa el rostro
                rostro.deteccion.imagen = imgTmp;
                rostro.imagenVisor = imagenVisor;        
                rostro.imagenRostro = rostro.deteccion.imagen.cloneResize(anchoRostroDet,alturaRostroDet);
                
                // Se guarda el rostro para ser procesado                
                lstRostros.add(rostro);
                lstRostrosPend.add(rostro);
            }
        }
        catch( const exception &e )
        {

        }
        
        solicitaCalcularFaceDesc(maxRosPorThCalcDesc,imagenNula);
                
        return;
        // Finaliza el proceso de la primera deteccion desde que se encendio el sensor
    }

    // busca detecciones previas y nuevas
    GLinkedList<FaceDetection> lstRostrosDibujar;
    
    mtx.lock();
    try
    {
        // recorre todas las detecciones nuevas
        for(i=0; i<n; i++)
        {
            det = lstDet.get(i);
            if (( det.region.getWidth() < anchoRostroMinVis ) || ( det.region.getHeight() < alturaRostroMinVis ))
            {
                // se descarta deteccion por ser muy chica
                continue;
            }

            // Creamos el rostro que se debe dibujar
            rostroNuevo.deteccion = det;
            rostroNuevo.fecDetect = ahora;
            rostroNuevo.fecCreacion = 0;
            rostroNuevo.fecDescriptor = 0;

            // opriginalmente una deteccion tiene la imagen de la camara no el elemento detectado
            imgTmp = rostroNuevo.deteccion.imagen.getRect(det.region);                
            // cara con la resolucion original
            rostroNuevo.deteccion.imagen = imgTmp; 
            // imagen de toda la pantalla
            rostroNuevo.imagenVisor = imagenVisor;  
            // cara escalada a la resolucion que necesita el generador de descriptores faciales
            rostroNuevo.imagenRostro = imgTmp.cloneResize(anchoRostroDet,alturaRostroDet);
            
            // recorre los rostros visualizados en el bucle anterior
            encontro = false;
            for(j=0; j<k; j++)
            {
                rostro = lstRostros.get(j);
                if ( rostro.deteccion.id == det.id )
                {
                    // La deteccion nueva coincide con un rostro previo                       
                    if ( rostro.fecDescriptor > 0  )
                    {                          
                        // el rostro ya tiene descriptor facial
                        if (( rostro.fecDetect-rostro.fecDescriptor) > tiempoRenovacionMs )
                        {
                            // Se debe reprocesar el rostro porque ha pasado un tiempo de caducidad y validar
                            // o descartar un dato mal calculado                                    
                            lstRostrosPend.add(rostroNuevo);
                            // cout << "Existe det muy antigua" << endl;                                        
                        }
                        else
                        {
                            // cout << "Existe reciente" << endl; 
                            encontro = true;                    
                        }   

                        rostroNuevo.fecDescriptor = rostro.fecDescriptor;
                        rostroNuevo.descriptor = rostro.descriptor;
                    }                                 
                    break;                
                }
            }
            
            if (( encontro == false ) && 
                (( det.region.getWidth() >= anchoRostroMinimo ) || ( det.region.getHeight() >= alturaRostroMinima )))
            {                                                    
                lstRostrosPend.add(rostroNuevo);
            }

            lstRostrosDibujar.add(rostroNuevo);
        }

        // Copia los descriptores de los rostros anteriores en los nuevos rostros
        // n = lstRostros.size();
        // k = lstRostrosDibujar.size();
        // for(i=0;i<n;i++)
        // {
        //     rostro = lstRostros.get(i);
        //     for(j=0;j<k;j++)
        //     {
        //         ptrRostro = lstRostrosDibujar.getAddr(j);
        //         if ( ptrRostro->deteccion.id == rostro.deteccion.id )
        //         {
        //             ptrRostro->descriptor = rostro.descriptor;
        //             ptrRostro->fecDescriptor = rostro.fecDescriptor;
        //             break;
        //         }
        //     }
        // }

        lstRostros.reset();
        lstRostros.addAll(lstRostrosDibujar);        
        lstRostrosDibujar.reset();
    }
    catch( const exception &e )
    {
    }    
    mtx.unlock();
    
    // cout << ahora << " Proc.Det.: Num Rostros " << k << " Num Det " << n << " " <<   "Num Rostros Finales " << lstRostros->size() << " Pendientes " << lstRostrosPend->size() << endl;
    solicitaCalcularFaceDesc(maxRosPorThCalcDesc,imagenNula);        
}

/**
 * Solicita calcular la descripcion de los rostros
 */
void IdentificadorFacial::solicitaCalcularFaceDesc( int numPeticiones, GImage imagen )
{
    int i,n,numEncolado;
    FaceDetection det;
    GLinkedList<FaceDetection> lstSolicitudes;

    if ( thCalculadorDesc.getCalculando() == true )
    {
        // No solicita nuevos datos porque hay datos pendientes
        // cout << "Se ignora calculo porque TH de DF esta ocupado, pendientes :" << lstRostrosPend.size()  << endl;
        return;
    }

    mtx.lock();
    try
    {
        numEncolado = 0;
        n = lstRostrosPend.size();        
        for(i=n-1; i>=0; i--)
        {
            det = lstRostrosPend.get(i);           
            numEncolado++;           
            lstSolicitudes.add(det);            
            if ( numEncolado >= numPeticiones )
            {
                break;
            }
        }
    }
    catch( const exception &e )
    {

    }
    mtx.unlock();
    
    if ( lstSolicitudes.size() > 0 )
    {
        cout << "Solicita calcular DF " << lstSolicitudes.size() << endl;
        thCalculadorDesc.calcularDescriptores(lstSolicitudes, imagen);        
    }    
}

/**
 * Procesa eventos del mouse
 */
void IdentificadorFacial::mouseEvent( int event, int x, int y, void *userdata )
{
    if ( event == cv::EVENT_FLAG_LBUTTON )
    {
        int i,n,j,k;
        FaceDetection det;
        GRect region;
        
        n = lstRostros.size();
        for(i=0; i < n; i++)
        {
            det = lstRostros.get(i);
            region = det.deteccion.region;
            region.escala(factorHorVisualiza, factorVerVisualiza);

            if (( region.getWidth() < anchoRostroMinimo ) && ( region.getHeight() < alturaRostroMinima ))
            {         
                continue;
            }
            else
            if ( det.descriptor.getSize() == 0 ) 
            {
                continue;
            }
            else
            if ((( x >= region.x1 ) && ( x <= region.x2)) &&
                (( y >= region.y1 ) && ( y <= region.y2)))
            {
                // Ya tiene reconocimiento             
                k = det.descriptor.getSize();
                for(j=0; j<k; j++)
                {
                    cout << det.descriptor.getFloats()[j] << ",";
                }   
                cout << endl << endl;
                break;             
            }
        }
    }
}

/**
 * Lee los parametros de ejecucion 
 */
void IdentificadorFacial::leeParametros( string path )
{
    pathParametros = path;
    lstParamsApp = leeArchivoConfig(pathParametros);
}

/**
* Guarda los parametros modificados
*/
void IdentificadorFacial::guardaParametros()
{
    guardaArchivoConfig(pathParametros, lstParamsApp);
}


