

#include "app/ProcRecFacial.h"
#include "lib/graphics/ImageSource.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/systemUtils.h"
#include "app/ExtractorFacialArchivo.h"

#include <opencv2/opencv.hpp>
#include <iostream>

/**
 * Punto de inicio de ejecucion de la aplicacion
 */
int main( int argc, char *argv[])
{
    ProcesoRecFacial proc;

    // lee la configuracion
    proc.leeParametros("./config/params.txt");    
    shared_ptr<GHashMap> lstParams = leeArchivoConfig("./config/config.txt");

    // analiza parametros de linea de comandos
    for(int i=0; i<argc; i++)
    {
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

            extractor.alturaRostroMinima = lstParams->getStringLong("anchoMinCaraRec", 90);
            extractor.anchoRostroMinimo = lstParams->getStringLong("alturaMinCaraRec", 90);  
            extractor.toleranciaDetec = lstParams->getStringDouble("presicionDeteccion",0.40);
            extractor.toleranciaIden = lstParams->getStringDouble("deltaRostroMax",0.60);
    
            extractor.ejecutar();
            
            return 0;
        }
        else
        if ( strcmp(argv[i],"--help") == 0 )
        {
            cout << endl << "Ejecutar el programa sin parametros para funcionar como detector" << endl << endl ;
            cout << endl << "Pasar el parametro -procesa [archivoDatos] [directorioFotos] [archivoSalida]" << endl;
            cout << "Para que se genere un [archivoSalida] usando los datos del [archivoDatos] " << endl;
            cout << "[archivoDatos] es un CSV con datos: idPersona,nombre,foto" << endl;
            cout << "Foto en el archivo CSV es el nombre de un archivo dentro del [directorioFotos]" << endl;
            
            return 0;
        }
    }

    // parametros que se pueden cambiar desde el app web    
    proc.idEquipo = SystemUtils::getRaspberryPiSerial();
    cout << "Serie del equipo: " << proc.idEquipo << endl;
            
    // inicializa en base a parametros de configurarion    
    proc.alturaRostroMinima = lstParams->getStringLong("anchoMinCaraRec", 90);
    proc.anchoRostroMinimo = lstParams->getStringLong("alturaMinCaraRec", 90);

    proc.anchoRostroMinVis = lstParams->getStringLong("anchoMinCaraVis", 70);
    proc.alturaRostroMinVis = lstParams->getStringLong("alturaMinCaraVis", 70);
    
    proc.anchoCamara = lstParams->getStringLong("anchoImgFuente", 1920);
    proc.alturaCamara = lstParams->getStringLong("alturaImgFuente", 1080);

    proc.anchoRostroDet = lstParams->getStringLong("anchoCaraRec", 112);
    proc.alturaRostroDet = lstParams->getStringLong("alturaCaraRec", 112);
    
    proc.anchoVisualiza = lstParams->getStringLong("anchoImgVisualizacion", 1280);
    proc.alturaVisualiza = lstParams->getStringLong("alturaImgVisualizacion", 720);
    proc.compresionJpeg = lstParams->getStringLong("compresionJpeg", 70);
    
    proc.toleranciaDetec = lstParams->getStringDouble("presicionDeteccion",0.40);
    proc.toleranciaIden = lstParams->getStringDouble("deltaRostroMax",0.60);

    proc.usarDistEuclideana = lstParams->getStringBool("usarDistEcuclideana", true);
    proc.universoPersonas.setNumThreadsIdentificacion(lstParams->getStringLong("numThreadsIdentificacion",1));
    proc.tiempoReEvento = proc.lstParamsApp->getStringLong("tiempoReEvento",30) * 1000;
    proc.tiempoMaxNoReconocido = proc.lstParamsApp->getStringLong("tiempoMaxNoReconocido",30);
    proc.reportarDesconocidos = lstParams->getStringBool("reportarDesconocidos", false);
    
            
    // COnfigura la fuente de imagenes del sensor
    shared_ptr<ImageSourceFactory> imageSource;    
    string fuenteImages = lstParams->getString("source");

    cout << "Fuente de imagenes: " << fuenteImages <<  endl;

    if ( fuenteImages.compare("onvif") == 0 )
    {
        shared_ptr<OnvifCameraFactory>onvifFactory = make_shared<OnvifCameraFactory>();
        imageSource =  dynamic_pointer_cast<ImageSourceFactory>(onvifFactory);

        imageSource->lstParams.putString(OnvifCamera::PARAM_IP_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_IP_SERVIDOR));
        imageSource->lstParams.putString(OnvifCamera::PARAM_PUERTO_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_PUERTO_SERVIDOR));
        imageSource->lstParams.putString(OnvifCamera::PARAM_USUARIO_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_USUARIO_SERVIDOR));
        imageSource->lstParams.putString(OnvifCamera::PARAM_PASSWORD_SERVIDOR, proc.lstParamsApp->getString(OnvifCamera::PARAM_PASSWORD_SERVIDOR));

        imageSource->lstParams.putString(OnvifCamera::PARAM_URL_SERVIDOR, lstParams->getString(OnvifCamera::PARAM_URL_SERVIDOR));       
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

    proc.universoPersonas.cargarpPerConocidas(lstParams->getString("pathBDPersonas"), unifidarDescr);
    proc.generadorEventos.urlServidorIden = proc.lstParamsApp->getString("urlBaseServidorIden");
    proc.generadorEventos.urlServidorNoIden = proc.lstParamsApp->getString("urlBaseServidorNoIden");
    proc.generadorEventos.urlServidorUnificado = proc.lstParamsApp->getString("urlServidorUnificado");
    proc.generadorEventos.pathLogEventos = lstParams->getString("logEventos");
    proc.generadorEventos.generarLogEventos = lstParams->getStringBool("generarLogEventos", false);
    proc.generadorEventos.usarEndpointUnificado = lstParams->getStringBool("usarEndPointUnificado", false);

    proc.iniciar();

    return 0;
}
