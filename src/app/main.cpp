
#include "app/ProcRecFacial.h"
#include "lib/graphics/ImageSource.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/systemUtils.h"
#include "app/ExtractorFacialArchivo.h"
#include "app/ComparadorFacial.h"
#include "app/PreProcesadorDataSet.h"

#include <opencv2/opencv.hpp>
#include <iostream>

  

/**
 * Punto de inicio de ejecucion de la aplicacion
 */
int main( int argc, char *argv[])
{
    ProcesoRecFacial proc;

    // lee la configuracion
    // proc.leeParametros("./config/params.txt");    
    // shared_ptr<GHashMap> proc.lstParamsApp = leeArchivoConfig("./config/config.txt");
    proc.leeParametros("./config/config.txt");    

    // analiza parametros de linea de comandos
    for(int i=0; i<argc; i++)
    {
        cout << "Parametro " << i << " : " << argv[i] << endl; 

        if (( strcmp(argv[i],"-preprocesa-dataset") == 0 ) && ( (i+3) < argc ))
        {
            // Preprocesa un dataset
            PreProcesadorDataSet preprocesador;
            hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr;
            float similaridad;

            // inicializa el dispositivo
            hailort::Expected<std::unique_ptr<hailort::VDevice>> device = hailort::VDevice::create();    
            if (!device) 
            {
                cerr << "Error: No se pudo inicializar el dispositivo Hailo." << endl;        
                return -1;
            }
            devicePtr = &device;

            similaridad = std::stof(argv[i+3]);
            preprocesador.eliminaNoCaras(devicePtr, argv[i+1]);
            preprocesador.agrupaCaras(devicePtr, argv[i+1], argv[i+2], similaridad, proc.lstParamsApp);

            return 0;
        }
        else
        if (( strcmp(argv[i],"-compara-simple") == 0 ) && ( (i+2) < argc ))
        {
            // compara dos rostros a la resolucion del modelo
            ComparadorFacial comparador;

            float comparacion = comparador.comparaSimple(argv[i+1], argv[i+2], proc.lstParamsApp);
            cout << "Resultado de la comparacion: " << comparacion << endl << endl;
            return 0; 
        }
        else    
        if (( strcmp(argv[i],"-compara-busca") == 0 ) && ( (i+2) < argc ))
        {
            // compara dos rostros a la resolucion del modelo
            ComparadorFacial comparador;

            float comparacion = comparador.comparaBusca(argv[i+1], argv[i+2], proc.lstParamsApp);
            cout << "Resultado de la comparacion: " << comparacion << endl << endl;
            return 0; 
        }
        else
        if (( strcmp(argv[i],"-procesa") == 0 ) && ( (i+3) < argc ))
        {
            cout << "Se debe generar identificadores faciales"  << endl;
            cout << "Archivo con datos: " << argv[i+1] << endl;
            cout << "Directorio con fotos: " << argv[i+2] << endl;
            cout << "Archivo con descriptores: " << argv[i+3] << endl;
            cout << endl;

            ExtractorFacialArchivo extractor;

            extractor.pathArchivoDatos = argv[i+1];
            extractor.pathFotos = argv[i+2];
            extractor.pathArchivoBD = argv[i+3];

            extractor.alturaRostroMinima = proc.lstParamsApp->getStringLong("anchoMinCaraRec", 90);
            extractor.anchoRostroMinimo = proc.lstParamsApp->getStringLong("alturaMinCaraRec", 90);  
            extractor.toleranciaDetec = proc.lstParamsApp->getStringDouble("presicionDeteccion",0.40);
            extractor.toleranciaIden = proc.lstParamsApp->getStringDouble("deltaRostroMax",0.60);
           
            extractor.paramContraste = proc.lstParamsApp->getStringDouble("paramContraste", 0);
            extractor.pixelSuavizado = proc.lstParamsApp->getStringLong("pixelSuavizado", 5);
            extractor.alturaImagenBase = proc.lstParamsApp->getStringLong("alturaImagenBase",0);
            extractor.jpegSuavizado = proc.lstParamsApp->getStringLong("jpegSuavizado",100);
            extractor.resizeSuavizado = proc.lstParamsApp->getStringDouble("resizeSuavizado",1);
            extractor.guardarCarasFrontalesAlineadas = proc.lstParamsApp->getStringBool("guardarCarasFrontalesAlineadas",false);
            extractor.encuadrarRostros = proc.lstParamsApp->getString("encuadrarRostros");

            extractor.pathModeloDescFacial = proc.lstParamsApp->getString("pathModeloDescFacial");
            extractor.nombreCapaSalidaRedFacial = proc.lstParamsApp->getString("nombreUltimaCapaRedNeuronal");
            
            extractor.previsualizaImgReconocimiento = proc.lstParamsApp->getStringBool("previsualizaImgReconocimiento",false);
            extractor.esperarPrevImgReconocimiento = proc.lstParamsApp->getStringBool("esperarPrevImgReconocimiento",false);
    
            extractor.ejecutar();
            
            return 0;
        }
        else
        if (( strcmp(argv[i],"-extraeCaras") == 0 ) && ( (i+1) < argc ))
        {
            cout << "Se extraen caras de fotos con un solo rostro"  << endl;
            cout << "Archivo con descriptores: " << argv[i+i] << endl;
            cout << endl;

            ExtractorFacialArchivo extractor;
            
            extractor.alturaRostroMinima = proc.lstParamsApp->getStringLong("anchoMinCaraRec", 90);
            extractor.anchoRostroMinimo = proc.lstParamsApp->getStringLong("alturaMinCaraRec", 90);  
            extractor.toleranciaDetec = proc.lstParamsApp->getStringDouble("presicionDeteccion",0.40);
            extractor.toleranciaIden = proc.lstParamsApp->getStringDouble("deltaRostroMax",0.60);
           
            extractor.paramContraste = proc.lstParamsApp->getStringDouble("paramContraste", 0);
            extractor.pixelSuavizado = proc.lstParamsApp->getStringLong("pixelSuavizado", 5);
            extractor.alturaImagenBase = proc.lstParamsApp->getStringLong("alturaImagenBase",0);
            extractor.jpegSuavizado = proc.lstParamsApp->getStringLong("jpegSuavizado",100);
            extractor.resizeSuavizado = proc.lstParamsApp->getStringDouble("resizeSuavizado",1);
            extractor.guardarCarasFrontalesAlineadas = true;
            extractor.encuadrarRostros = proc.lstParamsApp->getString("encuadrarRostros");

            extractor.pathModeloDescFacial = proc.lstParamsApp->getString("pathModeloDescFacial");
            extractor.nombreCapaSalidaRedFacial = proc.lstParamsApp->getString("nombreUltimaCapaRedNeuronal");
            
            extractor.previsualizaImgReconocimiento = proc.lstParamsApp->getStringBool("previsualizaImgReconocimiento",false);
            extractor.esperarPrevImgReconocimiento = proc.lstParamsApp->getStringBool("esperarPrevImgReconocimiento",false);
    
            extractor.extreCarasDirectorio(argv[i+1]);
            
            return 0;
        }           
        else
        if ( strcmp(argv[i],"--help") == 0 )
        {
            cout << endl << "Ejecutar el programa sin parametros para funcionar como detector" << endl << endl ;
            
            cout << endl << "-procesa [archivoDatos] [directorioFotos] [archivoSalida]" << endl << endl;
            cout << "Para que se genere un [archivoSalida] usando los datos del [archivoDatos] " << endl;
            cout << "[archivoDatos] es un CSV con datos: idPersona,nombre,foto" << endl;
            cout << "Foto en el archivo CSV es el nombre de un archivo dentro del [directorioFotos]" << endl << endl;

            cout << endl << "-compara-simple [path_cara1] [path_cara2]" << endl << endl;
            cout << "Para que comparar y ver la similitud entre la foto de dos caras centradas frontalmente" << endl;
            cout << "con la misma resolucion que espera la red neuronal especificada en config.txt" << endl << endl;

            cout << endl << "-compara-busca [path_foto1] [path_foto2]" << endl << endl;
            cout << "Para buscar una cara en cada foto y luego compararlas" << endl;
            cout << "con la misma resolucion que espera la red neuronal especificada en config.txt" << endl << endl;
            
            cout << endl << "-preprocesa-dataset [path_dirfotos] [path_dircaras] [silimaridad_caras]" << endl << endl;
            cout << "Busca un directorio con fotos (path_dirfotos) y borra todos los archivos que no tienen caras" << endl;
            cout << "Agrupa todas las fotos restantes del archivo de fotos por cara en base a similaridad entre ellas " << endl;
            cout << "crea un subdirectorio de la carpeta path_caras por cada grupo de caras y las copia en ellos" << endl;

            cout << endl << "-extraeCaras [path_fotos]" << endl << endl;
            cout << "Detecta una cara de cada foto de un directorio, y las guarda alineadas y escaladas" << endl;
            cout << "con la misma resolucion que espera la red neuronal especificada en config.txt" << endl << endl;
           
            return 0;
        }
    }

    // parametros que se pueden cambiar desde el app web    
    proc.idEquipo = SystemUtils::getRaspberryPiSerial();
    cout << "Serie del equipo: " << proc.idEquipo << endl;
            
    // inicializa en base a parametros de configurarion    
    proc.alturaRostroMinima = proc.lstParamsApp->getStringLong("anchoMinCaraRec", 90);
    proc.anchoRostroMinimo = proc.lstParamsApp->getStringLong("alturaMinCaraRec", 90);

    proc.anchoRostroMinVis = proc.lstParamsApp->getStringLong("anchoMinCaraVis", 70);
    proc.alturaRostroMinVis = proc.lstParamsApp->getStringLong("alturaMinCaraVis", 70);
    
    proc.anchoCamara = proc.lstParamsApp->getStringLong("anchoImgFuente", 1920);
    proc.alturaCamara = proc.lstParamsApp->getStringLong("alturaImgFuente", 1080);

    proc.anchoRostroDet = proc.lstParamsApp->getStringLong("anchoCaraRec", 112);
    proc.alturaRostroDet = proc.lstParamsApp->getStringLong("alturaCaraRec", 112);
    
    proc.anchoVisualiza = proc.lstParamsApp->getStringLong("anchoImgVisualizacion", 1280);
    proc.alturaVisualiza = proc.lstParamsApp->getStringLong("alturaImgVisualizacion", 720);
    proc.compresionJpeg = proc.lstParamsApp->getStringLong("compresionJpeg", 70);
    
    proc.toleranciaDetec = proc.lstParamsApp->getStringDouble("presicionDeteccion",0.40);
    proc.toleranciaIden = proc.lstParamsApp->getStringDouble("deltaRostroMax",0.60);

    proc.regionInteresInicioX = proc.lstParamsApp->getStringDouble("regionInteresInicioX", 0);
    proc.regionInteresFinX = proc.lstParamsApp->getStringDouble("regionInteresFinX", proc.anchoCamara);

    proc.usarDistEuclideana = proc.lstParamsApp->getStringBool("usarDistEcuclideana", true);
    proc.universoPersonas.setNumThreadsIdentificacion(proc.lstParamsApp->getStringLong("numThreadsIdentificacion",1));
    proc.tiempoReEvento = proc.lstParamsApp->getStringLong("tiempoReEvento",30) * 1000;
    proc.tiempoMaxNoReconocido = proc.lstParamsApp->getStringLong("tiempoMaxNoReconocido",30);
    proc.reportarDesconocidos = proc.lstParamsApp->getStringBool("reportarDesconocidos", false);
    proc.pathModeloDescFacial = proc.lstParamsApp->getString("pathModeloDescFacial");
    proc.nombreCapaSalidaRedFacial = proc.lstParamsApp->getString("nombreUltimaCapaRedNeuronal");
    proc.paramContraste = proc.lstParamsApp->getStringDouble("paramContraste", 2.0);
    proc.encuadrarRostros = proc.lstParamsApp->getString("encuadrarRostros");
            
    // COnfigura la fuente de imagenes del sensor
    shared_ptr<ImageSourceFactory> imageSource;    
    string fuenteImages = proc.lstParamsApp->getString("source");

    cout << "Fuente de imagenes: " << fuenteImages <<  endl;

    if ( fuenteImages.compare("onvif") == 0 )
    {
        shared_ptr<OnvifCameraFactory>onvifFactory = make_shared<OnvifCameraFactory>();
        imageSource =  dynamic_pointer_cast<ImageSourceFactory>(onvifFactory);

        imageSource->lstParams.putString(OnvifCamera::PARAM_IP_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_IP_SERVIDOR));
        imageSource->lstParams.putString(OnvifCamera::PARAM_PUERTO_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_PUERTO_SERVIDOR));
        imageSource->lstParams.putString(OnvifCamera::PARAM_USUARIO_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_USUARIO_SERVIDOR));
        imageSource->lstParams.putString(OnvifCamera::PARAM_PASSWORD_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_PASSWORD_SERVIDOR));

        imageSource->lstParams.putString(OnvifCamera::PARAM_URL_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_URL_SERVIDOR));       
    }
    else
    if ( fuenteImages.compare("video") == 0 )
    {
        shared_ptr<VideoCameraFactory> videoImgFac = make_shared<VideoCameraFactory>();
        imageSource =  dynamic_pointer_cast<ImageSourceFactory>(videoImgFac);        
        imageSource->lstParams.putString(VideoCamera::PARAM_PATH_VIDEO, proc.lstParamsApp->getString(VideoCamera::PARAM_PATH_VIDEO));
        imageSource->lstParams.putString(VideoCamera::PARAM_VELOCIDAD_VIDEO, proc.lstParamsApp->getString(VideoCamera::PARAM_VELOCIDAD_VIDEO));
        imageSource->lstParams.putString(VideoCamera::PARAM_CUADRO_INICIAL, proc.lstParamsApp->getString(VideoCamera::PARAM_CUADRO_INICIAL));
        imageSource->lstParams.putString(VideoCamera::PARAM_INFINITO, proc.lstParamsApp->getString(VideoCamera::PARAM_INFINITO));
    }
    else
    {
        shared_ptr<InterImgSourceFactory> intImgFac = make_shared<InterImgSourceFactory>();
        imageSource =  dynamic_pointer_cast<ImageSourceFactory>(intImgFac);        
    }

    // valida si se debe o no invertir verticalmente la imagen de la camara
    string invVertical = proc.lstParamsApp->getString("reflejarVerticalmente");
    if ( invVertical.compare("S") ) proc.invertirVertical = true;
    else proc.invertirVertical = false;
      
    // configura la fuente de imagenes y carga las persona conocidas
    proc.setImageFactory(imageSource);    

    string paramUnificarDescr = proc.lstParamsApp->getString("unificarDescriptores");
    bool unifidarDescr = false;
    if ( paramUnificarDescr.compare("S") == 0 ) unifidarDescr = true;

    proc.universoPersonas.cargarpPerConocidas(proc.lstParamsApp->getString("pathBDPersonas"), unifidarDescr);
    proc.generadorEventos.urlServidorIden = proc.lstParamsApp->getString("urlBaseServidorIden");
    proc.generadorEventos.urlServidorNoIden = proc.lstParamsApp->getString("urlBaseServidorNoIden");
    proc.generadorEventos.urlServidorUnificado = proc.lstParamsApp->getString("urlServidorUnificado");
    proc.generadorEventos.pathLogEventos = proc.lstParamsApp->getString("logEventos");
    proc.generadorEventos.generarLogEventos = proc.lstParamsApp->getStringBool("generarLogEventos", false);
    proc.generadorEventos.usarEndpointUnificado = proc.lstParamsApp->getStringBool("usarEndPointUnificado", false);

    proc.iniciar();

    return 0;
}
