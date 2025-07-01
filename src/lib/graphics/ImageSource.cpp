
#include "lib/utils/timedate.h"
#include "lib/graphics/ImageSource.h"


/**
 * Nombre del parametro en el que se define la resolucion horizontal
 */
const string InternalCameraSource::PARAM_RES_HORIZONTAL = "res_horizontal";

/**
 * Nombre del parametro en el que se define la resolucion vertical
 */
const string InternalCameraSource::PARAM_RES_VERTICAL = "res_vertical";

/**
 * Nombre del dispositivo que representa la camara
 */
const string InternalCameraSource::PARAM_DEV_CAMARA = "dev_camara";

/**
 * Nombre del parametro entero que si tiene el valor 1 significa
 * que la imagen se debe reflejar verticalmente
 */
const string InternalCameraSource::PARAM_REFLEJAR_VERTICALMENTE = "reflejar_verticalmente";

/**
 * Ajusta un parametro del video
 * 
 *  parametro: valor definido en constantes PARAM_VIDEO_XXXXXX
 * 
 *  valor : valor double que se asigna al parametro
 */
void ImageSource::ajustaVideo( int parametro, double valor )
{
    
}

/**
 * Establece el valor de un parametro del tipo string
 */
void ImageSource::setStringParam( int param, string valor )
{

}

/**
 * Inicializa el proveedor
 */
int InternalCameraSource::init()
{
    string idCamara;

    camara.usarLibCamara = true;
    camara.setDim( lstParams.getInt(InternalCameraSource::PARAM_RES_HORIZONTAL,1920), lstParams.getInt(InternalCameraSource::PARAM_RES_VERTICAL,1080));
    // camara.setDim( lstParams.getInt(InternalCameraSource::PARAM_RES_HORIZONTAL,2592), lstParams.getInt(InternalCameraSource::PARAM_RES_VERTICAL,1944));
    // camara.setDim( lstParams.getInt(InternalCameraSource::PARAM_RES_HORIZONTAL,1296), lstParams.getInt(InternalCameraSource::PARAM_RES_VERTICAL,972));
    
    idCamara = lstParams.getString(InternalCameraSource::PARAM_DEV_CAMARA);
    if ( idCamara.length() == 0 ) idCamara = "/dev/video0";
    
    camara.setCamara(idCamara);
    camara.reflejarVerticalmente = false; // lstParams.getInt( InternalCameraSource::PARAM_REFLEJAR_VERTICALMENTE, 1);

    cout << ">>> Refleja Verticalmente " << camara.reflejarVerticalmente << endl;
 
    if ( camara.iniciarCaptura() != 0)
    {
        cout << "Error al inicializar la camara" << endl;
        return 1;
    }

    return 0;
}

/**
 * Detiene el proceso de captura de imagenes
 */
void InternalCameraSource::stop()
{

}

/**
 * Libera el uso de recursos
 */
void InternalCameraSource::release()
{
    camara.detenerCaptura();
}

/**
 * Retonra la siguiente imagen desde el proveedor
 */
GImage InternalCameraSource::getImage()
{
    GImage img = camara.leerCamara();

    ultimoErrorCodigo = camara.errorEnLectura;
    ultimoErrorDesc = "Error al obtener imagen";

    return img;
}


/**
 * Ajusta un parametro del video
 * 
 *  parametro: valor definido en constantes PARAM_VIDEO_XXXXXX
 * 
 *  valor : valor double que se asigna al parametro
 */
void InternalCameraSource::ajustaVideo( int parametro, double valor )
{
    camara.ajustaVideo(parametro, valor);
}


/**
 * Actualiza la configuracion del dispositivo fisico en base a una lista
 * de parametros.
 * 
 * @param paramNames lista de los nombres de parametros que se desea se actualicen,
 * estos nombres de parametros van separados por coma, no poner espacios entre ellos
 */
void InternalCameraSource::updateConfig( string paramNames )
{
    ultimoErrorCodigo = 0;
}

/**
 * Establece el valor de un parametro del tipo string
 */
void InternalCameraSource::setStringParam( int param, string valor )
{

}


/**
 * Se encarga de crear la fuente
 */
shared_ptr<ImageSource> InterImgSourceFactory::getInstance()
{
    shared_ptr<InternalCameraSource> source = make_shared<InternalCameraSource>();

    source->lstParams = this->lstParams;
    source->init();

    return dynamic_pointer_cast<ImageSource>(source);
}

/**
 * Metodo que retorna una instancia de ImageSource
 * Recibe como parametro un archivo con la configuracion del image source
 */
shared_ptr<ImageSource> InterImgSourceFactory::getInstance( string configPath )
{
    shared_ptr<InternalCameraSource> source = make_shared<InternalCameraSource>();

    source->lstParams = this->lstParams;
    source->init();

    return dynamic_pointer_cast<ImageSource>(source);
}


/**
 * IP de la camara
 */
const string Esp32SocketCamera::PARAM_IP = "ip";


/**
 * Inicializa el proveedor
 */
int Esp32SocketCamera::init()
{
    string ip = lstParams.getString(PARAM_IP);

    try
    {
        socket.abrirConexion(ip, 5000, 0);
        socket.setTimeout(5);
        socket.writeUint32(1);
        socket.writeSmallString("SensorFacial");
        leeMascaraOfuscadora();      

        return 0;
    }
    catch(const std::exception& e)
    {
        cout << "Error de conexion" << endl;
        return 1;
    }    
}

 /**
 * Lee la mascara que ofusca los datoss
 */
void Esp32SocketCamera::leeMascaraOfuscadora()
{
    uint8_t buffer[32];
    int i,j;

    socket.read((char *)buffer, 32);

    j = 1;
    for(i=0;i<16;i++)
    {
        mascaraOfuscadora[i] = buffer[j++];
        j++;
    }      
}

/**
 * Detiene el proceso de captura de imagenes
 */
void Esp32SocketCamera::stop()
{
    if ( socket.estaConectado() == 1 )
    {
        socket.cerrar();
    }
}

/**
 * Libera el uso de recursos
 */
void Esp32SocketCamera::release()
{

}

/**
 * Establece el valor de un parametro del tipo string
 */
void Esp32SocketCamera::setStringParam( int param, string valor )
{

}

/**
 * Retonra la siguiente imagen desde el proveedor
 */
GImage Esp32SocketCamera::getImage()
{
    int reintentos = 0;
    GImage rpta;
    char buffer[8];
    uint8_t *dataJpeg;
    uint32_t lenImagen,len2;
    uint32_t *ptr32;
    // uint32_t i,posMascara = 0;
    
    dataJpeg = NULL;
    lenImagen = 0;
    try
    {
        
        while( true )
        {        
            socket.writeUint32(2);
            socket.readLen(buffer,8);
            if ( socket.estaConectado() == false )
            {
                init();
                return rpta;
            }
                    
            ptr32 = (uint32_t*)buffer;
            lenImagen = *ptr32;
            ptr32++;
            len2 = *ptr32;

            if ( len2 != lenImagen )
            {
                cout << "Error de integridad de datos" << endl;
                reintentos++;
                if ( reintentos == 5 )
                {
                    cout << "Numero excesivo de reintentos de lectura de imagen" << endl;
                    socket.cerrar();
                    init();

                    return rpta;
                }                
                socket.clearInputBuffer();
            }
            else
            {
                break;
            }
        }

        dataJpeg = (uint8_t*)malloc(lenImagen);

        // lee el archivo jpeg
        socket.readLen((char*)dataJpeg, lenImagen);

        // quita ofuscacion
        // for(i=0;i<lenImagen;i++)
        // {            
        //     dataJpeg[i]^= mascaraOfuscadora[posMascara&15];
        //     posMascara++; 
        // }
        // cout << endl;

        leeMascaraOfuscadora();

        // decodifica el jpeg
        rpta.decodeJPEGImage(dataJpeg, lenImagen);
    }
    catch(const std::exception& e)
    {
        cout << "Error de conexion con la camara: " << lenImagen <<  endl;
        socket.cerrar();
    }

    if ( dataJpeg != NULL )
    {
        free(dataJpeg);
    }

    if ( socket.estaConectado() == 0 )
    {
        init();
    }

    return rpta;
}


/**
 * Actualiza la configuracion del dispositivo fisico en base a una lista
 * de parametros.
 * 
 * @param paramNames lista de los nombres de parametros que se desea se actualicen,
 * estos nombres de parametros van separados por coma, no poner espacios entre ellos
 */
void Esp32SocketCamera::updateConfig( string paramNames )
{

}

/**
 * Se encarga de crear la fuente
 */
shared_ptr<ImageSource> Esp32SocketCamFactory::getInstance()
{
    shared_ptr<Esp32SocketCamera> camara = make_shared<Esp32SocketCamera>();

    camara->lstParams = this->lstParams;
    camara->init();

    return dynamic_pointer_cast<ImageSource>(camara);
}

/**
 * Metodo que retorna una instancia de ImageSource
 * Recibe como parametro un archivo con la configuracion del image source
 */
shared_ptr<ImageSource> Esp32SocketCamFactory::getInstance( string configPath )
{
    shared_ptr<Esp32SocketCamera> camara = make_shared<Esp32SocketCamera>();

    camara->lstParams = this->lstParams;
    camara->init();

    return dynamic_pointer_cast<ImageSource>(camara);
}



/**
 * Parametro que contiene la IP del servidor
 */
const string OnvifCamera::PARAM_IP_SERVIDOR = "onvifIP";

/**
 * Parametro que contiene el puerto del servidor
 */
const string OnvifCamera::PARAM_PUERTO_SERVIDOR = "onvifPuerto";

/**
 * Parametro que contiene el usuario del servidor
 */
const string OnvifCamera::PARAM_USUARIO_SERVIDOR = "onvifLogin";

/**
 * Parametro que contiene la password del usuario del servidor
 */
const string OnvifCamera::PARAM_PASSWORD_SERVIDOR = "onvifPassword";

/**
 * Parametro que contiene el sufijo o parte final del URL para obtener
 */
const string OnvifCamera::PARAM_URL_SERVIDOR = "urlServidor";

/**
 * Constructor
 */
OnvifCamera::OnvifCamera()
{

}

/**
 * Inicializa el proveedor
 */
int OnvifCamera::init()
{
    // string urlFinal = "rtps://";
    string urlFinal = "rtsp://";

    ipServidor = lstParams.getString(OnvifCamera::PARAM_IP_SERVIDOR);
    puertoServidor = lstParams.getString(OnvifCamera::PARAM_PUERTO_SERVIDOR);
    usuarioServidor = lstParams.getString(OnvifCamera::PARAM_USUARIO_SERVIDOR);
    passwordServidor = lstParams.getString(OnvifCamera::PARAM_PASSWORD_SERVIDOR);
    urlServidor = lstParams.getString(OnvifCamera::PARAM_URL_SERVIDOR);

    urlFinal.append(usuarioServidor);
    urlFinal.append(":");
    urlFinal.append(passwordServidor);
    urlFinal.append("@");
    urlFinal.append(ipServidor);
    urlFinal.append(":");
    urlFinal.append(puertoServidor);
    urlFinal.append(urlServidor);

    string gst_pipeline("rtspsrc location=");
    gst_pipeline.append(urlFinal);
    gst_pipeline.append("  latency=100 ! decodebin ! videoconvert ! appsink");

    cout << "Accediento a la camara ONVIF: " << urlFinal << endl;    
    cap = std::make_unique<cv::VideoCapture>();
    
    if ( !cap->open(urlFinal, cv::CAP_FFMPEG))    
    {
        std::cerr << "No se pudo conectar a la cámara ONVIF en " << urlFinal << std::endl;
        return -1;
    }

    start();

    cout << "Exito accediendo a la camara ONVIF: " << urlFinal << endl;
    return 0; 
}

/**
 * Detiene el proceso de captura de imagenes
 */
void OnvifCamera::stop()
{
    finalizar();
    if (cap) 
    {
        cap->release();
    }
}

/**
 * Libera el uso de recursos
 */
void OnvifCamera::release()
{
    
}

/**
 * Retonra la siguiente imagen desde el proveedor
 */
GImage OnvifCamera::getImage()
{
    GImage imagen;
    // cv::Mat frame;

    // *cap >> frame;
    // imagen.imagenOpencv = frame;
    // imagen.ancho = frame.cols;
    // imagen.altura = frame.rows;

    mtxBloquea();
    imagen = imagenDet.clone();
    mtxLibera();


    return imagen;
}

/**
 * Actualiza la configuracion del dispositivo fisico en base a una lista
 * de parametros.
 * 
 * @param paramNames lista de los nombres de parametros que se desea se actualicen,
 * estos nombres de parametros van separados por coma, no poner espacios entre ellos
 */
void OnvifCamera::updateConfig( string paramNames )
{

}

/**
 * Establece el valor de un parametro del tipo string
 */
void OnvifCamera::setStringParam( int param, string valor )
{
}

/**
 * Bucle del thread
 */
void OnvifCamera::runThread()
{
    GImage imagen;

    while( isFinalizado() == false )
    {        
        cv::Mat frame;

        *cap >> frame;
        imagen.imagenOpencv = frame;
        imagen.ancho = frame.cols;
        imagen.altura = frame.rows;

        mtxBloquea();
        imagenDet = imagen.clone();
        mtxLibera();
    }
    cout << "Thread Onvif finalizado" << endl;
}


/**
 * Se encarga de crear la fuente
 */
shared_ptr<ImageSource> OnvifCameraFactory::getInstance()
{
    cout << "Creando camara ONVIF" << endl;
    shared_ptr<OnvifCamera> camera = make_shared<OnvifCamera>();
    camera->lstParams = this->lstParams;
    camera->init();

    return dynamic_pointer_cast<ImageSource>(camera);
}

/**
 * Metodo que retorna una instancia de ImageSource
 * Recibe como parametro un archivo con la configuracion del image source
 */
shared_ptr<ImageSource> OnvifCameraFactory::getInstance( string configPath )
{
    cout << "Creando camara ONVIF" << endl;
    shared_ptr<OnvifCamera> camera = make_shared<OnvifCamera>();
    camera->lstParams = this->lstParams;
    camera->init();

    return dynamic_pointer_cast<ImageSource>(camera);
}


/**
 * Nombre del parametro que contiene la RUTA del video
 */
string const VideoCamera::PARAM_PATH_VIDEO = "videoPath";

/**
 * Nombre del parametro que contiene la velocidad de reproduccion en imagenes
 * por segundo del video
 */
string const VideoCamera::PARAM_VELOCIDAD_VIDEO = "videoVelocidadImgPorSegundo";

/**
 * Nombre del parametro que contiene el parametro a partir del que se procesa el video
 */
string const VideoCamera::PARAM_CUADRO_INICIAL = "videoCuadroInicial";

/**
 * Nombre del parametro que contiene el parametro que indica si se debe reproducir 
 * el video de forma infinita
 */
string const VideoCamera::PARAM_INFINITO = "videoInfinito";


/**
 * Constructor
 */
VideoCamera::VideoCamera()
{
    reproduccionInfinita = false;
}

/**
 * Destructor
 */
VideoCamera::~VideoCamera()
{

}

/**
 * Inicializa el proveedor
 */
int VideoCamera::init()
{
    string ruta = lstParams.getString(PARAM_PATH_VIDEO);
    string infinito = lstParams.getString(PARAM_INFINITO);

    if ( infinito.compare("S") == 0 ) reproduccionInfinita = true;

    cap = std::make_unique<cv::VideoCapture>(ruta);

    if ( ruta.length() == 0 )
    {
        std::cerr << "No se ha especificado la ruta del video: " << ruta << std::endl;
        return -1;
    }
    
    if ( !cap->isOpened())    
    {
        std::cerr << "No se pudo abrir el video " << ruta << std::endl;
        return -2;
    }

    long velocidad = lstParams.getStringLong(PARAM_VELOCIDAD_VIDEO, 30);

    if (( velocidad <= 0 ) || ( velocidad > 1000 ))
    {
        velocidad = 30;
    }

    long pos = lstParams.getStringLong(VideoCamera::PARAM_CUADRO_INICIAL,0);
    if ( pos < 0 ) pos = 0;
    
    cap->set(cv::CAP_PROP_POS_FRAMES, pos);
    
    periodoVideo = 1000 / velocidad;
    ultimaVezGenVideo = -1;

    cout << "Video cargado: " << ruta << endl;
    
    return 0;
}

/**
 * Detiene el proceso de captura de imagenes
 */
void VideoCamera::stop()
{
    release();
}

/**
 * Libera el uso de recursos
 */
void VideoCamera::release()
{
    if ( cap != nullptr )
    {
        cap->release();
    }
}

/**
 * Retonra la siguiente imagen desde el proveedor
 */
GImage VideoCamera::getImage()
{
    cv::Mat frame;
    bool exito = cap->read(frame);
    // *cap >> frame;

    if ( ultimaVezGenVideo == -1 )
    {
        ultimaVezGenVideo = TimeDateUtils::getDateTimeMs();
    }
    else
    {
        long long delta = periodoVideo - (TimeDateUtils::getDateTimeMs()-ultimaVezGenVideo);
        if ( delta > 0 )
        {
            usleep(delta*1000);
        }
        ultimaVezGenVideo = TimeDateUtils::getDateTimeMs();
    }

    if ( exito == false )
    {
        if ( reproduccionInfinita == true )
        {
            cap->set(cv::CAP_PROP_POS_FRAMES, 0);
        }
        else
        {
            return imagenDet;    
        }
    }

    if ( frame.empty() )
    {
        return imagenDet;
    }

    GImage imagen = GImage();
    imagen.imagenOpencv = frame;
    imagen.ancho = frame.rows;
    imagen.altura = frame.cols;

    imagenDet = imagen;

    return imagen;
}

/**
 * Actualiza la configuracion del dispositivo fisico en base a una lista
 * de parametros.
 * 
 * @param paramNames lista de los nombres de parametros que se desea se actualicen,
 * estos nombres de parametros van separados por coma, no poner espacios entre ellos
 */
void VideoCamera::updateConfig( string paramNames )
{

}

/**
 * Establece el valor de un parametro del tipo string
 */
void VideoCamera::setStringParam( int param, string valor )
{    
}

/**
 * Bucle del thread
 */
void VideoCamera::runThread()
{

}




/**
 * Se encarga de crear la fuente
 */
shared_ptr<ImageSource> VideoCameraFactory::getInstance() 
{
    cout << "Creando camara Video" << endl;
    shared_ptr<VideoCamera> camera = make_shared<VideoCamera>();
    camera->lstParams = this->lstParams;
    camera->init();

    return dynamic_pointer_cast<ImageSource>(camera);
}

/**
 * Metodo que retorna una instancia de ImageSource
 * Recibe como parametro un archivo con la configuracion del image source
 */
shared_ptr<ImageSource> VideoCameraFactory::getInstance( string configPath )
{
    cout << "Creando camara Video" << endl;
    shared_ptr<VideoCamera> camera = make_shared<VideoCamera>();
    camera->lstParams = this->lstParams;
    camera->init();

    return dynamic_pointer_cast<ImageSource>(camera);
}
