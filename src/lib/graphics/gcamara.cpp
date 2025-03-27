#include "lib/graphics/gcamara.h"

#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/framebuffer_allocator.h>
#include <sys/mman.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define ETAPA_IMG_CAMARA_INICIADO 0
#define ETAPA_IMG_CAMARA_ESPERANDO_IMG 1
#define ETAPA_IMG_CAMARA_IMG_LEIDA 2
#define ETAPA_IMG_CAMARA_LEYENDO_IMAGEN 3
#define ETAPA_IMG_CAMARA_PROCESADA 4

/**
 * Referencia a la unica instancia de GMacara que deberia tener el sistema
 */
GCamara *ptrCamaraActual;

/**
 * Indica si hay una imagen leida de la camara
 */
volatile char etapaCapturaImagenCamara;

/**
 * Indica si se presento un error al leer imagen de la camara
 */
volatile char hayErrorImagenCamara;

/**
 * Retorna el tiempo transcurrido en milisegundos
 */
uint64_t currentTimeMS()
{
    struct timespec tiempoActual;

    clock_gettime(CLOCK_MONOTONIC, &tiempoActual);
    uint64_t milisegundos = (tiempoActual.tv_sec*1000) + (tiempoActual.tv_nsec / 1000000);

    return milisegundos;
}


/**
 * Funcion que procesa la notificacion de que hay imagen lista
 */
static void requestCameraComplete( Request *request )
{
    // cout << currentTimeMS() << " RequestCamaraComplete: " <<  request->status() <<  endl;

    if ( request->status() == Request::RequestCancelled )
    {
        cout << "RequestCamaraComplet : Request Cancelled" << endl;
        return;
    }
    if ( etapaCapturaImagenCamara == ETAPA_IMG_CAMARA_ESPERANDO_IMG )
    {
        // cout << currentTimeMS() << " GCAMARA: Inicia lectura desde HW" << endl;
        etapaCapturaImagenCamara = ETAPA_IMG_CAMARA_LEYENDO_IMAGEN;

        const Request::BufferMap &buffers = request->buffers();
        // cout << "Num Buffers " << buffers.size() << endl;
        for( auto bufferPair: buffers )
        {
            FrameBuffer *buffer1 = bufferPair.second;
            // const FrameMetadata &metadata = buffer->metadata();     
            // cout << "Num Planos " << buffer1->planes().size() << endl;       

            for( const FrameBuffer::Plane &plane : buffer1->planes() )
            {
                int fd = plane.fd.get();
                uint8_t *bufferCam = ptrCamaraActual->getBufferCamara();
                uint8_t *bufferImg = (uint8_t *) mmap(NULL, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //  plane.offset);

                // cout << currentTimeMS() << " GCAMARA: Inicia copia desde HW" << endl;
                std::memcpy(bufferCam, bufferImg, plane.length);                
                munmap(bufferImg, plane.length);
                // cout << currentTimeMS() << " GCAMARA: Fin copia desde HW" << endl;
            }           
        }
        // cout << currentTimeMS() << " GCAMARA: Fin de requestCameraComplete" << endl;
        etapaCapturaImagenCamara = ETAPA_IMG_CAMARA_IMG_LEIDA;        
    }
    else
    {
        // cout << currentTimeMS() << " GCAMARA: Se ignora frame no hay nada solicitado HW" << endl;
    }
    
    if ( ptrCamaraActual->getIniciado() == 1 )
    {
        double exposicion;
        try
        {
            request->reuse(Request::ReuseBuffers);
                       
            if ( ptrCamaraActual->lstParamsVideo->size() > 0 )
            {                
                int i,param,n;
                shared_ptr<GVector> parametro;
                cout << "Actualiza Video " << endl;

                n = ptrCamaraActual->lstParamsVideo->size();

                for(i=0; i<n; i++)
                {
                    parametro = dynamic_pointer_cast<GVector>(ptrCamaraActual->lstParamsVideo->get(i));
                    param = parametro->getInt(0);
                    switch( param )
                    {
                        case PARAM_VIDEO_BRILLO:
                            request->controls().set(controls::Brightness, parametro->getDouble(1));
                            break;

                        case PARAM_VIDEO_CONTRASTE:
                            request->controls().set(controls::Contrast, parametro->getDouble(1));
                            break;

                        case PARAM_VIDEO_EXPOSICION:
                            exposicion = parametro->getDouble(1);
                            cout << "Cambiando exposicion en camara : " << exposicion << endl;
                            if ( exposicion == 0 )
                            {
                                // request->controls().set(controls::AeExposureMode, controls::AeExposureModeEnum::ExposureNormal);
                            }
                            else
                            {
                                // request->controls().set(controls::AeExposureMode, controls::AeExposureModeEnum::ExposureCustom);
                                request->controls().set(controls::ExposureTime, exposicion);
                            }                                                        
                            break;
                    }                
                }
                
                ptrCamaraActual->lstParamsVideo->clear();
            }

            ptrCamaraActual->camera->queueRequest(request);
        }
        catch(const std::exception& e)
        {
            
        }                
    }
}

 /**
 * Constructor
 */
GCamara::GCamara()
{
    setDim(640,480);
    iniciado = 0;
    fd = 0;
    buf = {0};
    errorEnLectura = 0;
    reflejarVerticalmente = 0;
    init = 0;
    usarLibCamara = false;

    lstParamsVideo = make_shared<GVector>();

    ptrCamaraActual = this;
}

/**
 * Destructur
 */
GCamara::~GCamara()
{

}


 /**
 * Indica si esta iniciada o no la camara
 */
int GCamara::getIniciado()
{
    return iniciado;
}

/**
 * Retorna el ancho de las imagenes
*/
 int GCamara::getAncho()
 {
    return ancho;
 }

/**
* Retorna la altura
 */
int GCamara::getAltura()
{
    return altura;
}

 /**
 * Retorna el buffer para almacenar una imagen de la camara
*/
uint8_t *GCamara::getBufferCamara()
{
    return buffer;
}

/**
 * Establece las dimensiones de la imagen que se captura
 */
void GCamara::setDim( int ancho, int altura )
{
    this->ancho = ancho;
    this->altura = altura;
}

/**
 * Establece el nombre del dispositivo con el que se trabaja
 */
void GCamara::setCamara( string name )
{
    nombreCamara = name;
}

/**
 * Inicia la captura
 */
int GCamara::iniciarCaptura()
{
    if ( usarLibCamara == false )
    {
        fd = v4l2_open(nombreCamara.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
        if (fd == -1)
        {        
            return 1;
        }
        if(print_caps())
            return 2;

        if(init_mmap())
            return 3;

        iniciado = 1;

        return 0;
    }

    cout <<  "GCAMARA: Iniciando "  << endl;
    
    cam_manager.start();
    if ( cam_manager.cameras().empty() )
    {
        return 4;
    }

    for( auto cam : cam_manager.cameras() )
    {
        cout << "GCAMARA: " << cam->id() << endl;
    }

    cout <<  "GCAMARA: Buscando Camara "  << endl;
    camera = cam_manager.cameras()[0];
    if ( !camera )
    {
        return 5;
    }

    cout <<  "GCAMARA: Accediendo a la camara "  << endl;
    if ( camera->acquire() )
    {
        return 6;
    }

    cout <<  "GCAMARA: Configurando La Camara " << ancho << " x " << altura  << endl;    

    // configura la camara

    // Funciona hasta con FULL HD
    // std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder});

    // FUnciona hasta con FULL HD
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::VideoRecording});

    // Funciona con FULL HD
    // std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::StillCapture});

    StreamConfiguration &streamConfig = config->at(0);

    streamConfig.size.width = ancho;
    streamConfig.size.height = altura;
    streamConfig.pixelFormat = formats::RGB888;

    cout <<  "GCAMARA: Validando la configuracion "  << endl;
    CameraConfiguration::Status validationStatus = config->validate();
    if ( validationStatus == CameraConfiguration::Invalid)
    {
        camera->release();
        return 7;
    }

    cout << "GCAMARA: Aplicando la configuracion:" << endl;

    if ( camera->configure(config.get()) )
    {
        cout << "GCAMARA: No se pudo configurar" << endl;
    }

    cout <<  "GCAMARA: Reservando buffer "  << endl;
    
    // reserva ram
    allocator = new FrameBufferAllocator(camera);
    for( StreamConfiguration &cfg : *config )
    {
        cout << "Configurando stream " << endl;
        if ( allocator->allocate(cfg.stream()) < 0 )
        {
            camera->release();
            return 8;
        }

        size_t allocated = allocator->buffers(cfg.stream()).size();
        std::cout << "GCAMARA: Allocated " << allocated << " buffers for stream" << std::endl;
    }
    
    cout <<  "GCAMARA: Configurando petisiones "  << endl;
    
    // configura peticiones de frames
    Stream *stream = streamConfig.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    
    for( unsigned int i = 0; i < buffers.size(); ++i )
    {
        cout << "GCAMARA: Creando peticion " << i << endl; 
        std::unique_ptr<Request> request = camera->createRequest();
        if ( !request )
        {
            return 9;
        }

        const std::unique_ptr<FrameBuffer> &bufferCam = buffers[i];
        if ( request->addBuffer(stream, bufferCam.get()) < 0 )
        {
            return 10;
        }

        // ControlList &controls = request->controls();
        // controls.set(controls::FrameDurationLimits, {1000, 333333} );
        // controls.set(controls::ExposureTime, 10000);
        // controls.set(controls::AnalogueGain, 1.0);

        requests.push_back(std::move(request));
    }

    cout <<  "GCAMARA: Reservando RAM para el ultimo buffer "  << endl;    
    buffer = (uint8_t *) malloc( ancho * altura * 3 );
    if ( buffer == NULL )
    {
        return 11;
    }

    cout << "GAMARA: Iniciando" << endl;
    camera->requestCompleted.connect(requestCameraComplete);   
    
    std::unique_ptr<ControlList> camcontrols = std::unique_ptr<ControlList>(new ControlList());
    // camcontrols->set(controls::FrameDurationLimits, Span<const int64_t,2> ({25000,25000}));
    camcontrols->set(controls::FrameDurationLimits, Span<const int64_t,2> ({30000,30000}));
    camcontrols->set(controls::Brightness, 0.10);
    // camcontrols->set(controls::ExposureValue, 0.15);
    // camcontrols->set(controls::ExposureValue, );
    // camcontrols->set(controls::AeExposureMode, controls::AeExposureModeEnum::ExposureShort);
    camcontrols->set(controls::Contrast, 1.05);
    // camcontrols->set(controls::AnalogueGain, 2);
    // camcontrols->set(controls::AeEnable, true);
    camcontrols->set(controls::AwbMode, controls::AwbAuto );

    camera->start(camcontrols.get());
    cout <<  "GCAMARA: Encolando peticiones"  << endl;
    for( std::unique_ptr<Request> &request : requests )
    {       
        camera->queueRequest(request.get());
    }        

    cout <<  "GCAMARA: Iniciando Lectura"  << endl;
    
    etapaCapturaImagenCamara = ETAPA_IMG_CAMARA_INICIADO;
    
    iniciado = 1;

    return 0;
}

/**
 * Finaliza la captura
 */
void GCamara::detenerCaptura()
{
    if ( iniciado == 1 )
    {
        if ( usarLibCamara == false )
        {
            close(fd);
            iniciado = 0;
            init = 0;
        }
        else
        {
            camera->requestCompleted.disconnect(requestCameraComplete);
            camera->release();
            cam_manager.stop();
            delete allocator;
        }
    }
}

/**
 * Lee la siguiente imagen de la camara
 */
// Mat GCamara::leerCamara()
GImage GCamara::leerCamara()
{
    // Mat f2;
    int numEsperas;

    if ( usarLibCamara == false )
    {
        errorEnLectura = 0;

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = 0;
        
        if( -1 == xioctl( VIDIOC_QBUF, &buf) )
        {
            GImage imagen(0,0,GDIBUJO_IMAGEN_MODO_RGB);

            cout << "Error: Query Buffer" << endl;
            errorEnLectura = 1;

            // return f2;
            return imagen;
        }

        if ( init == 0  )
        {
            cout << "Iniciando lectura " << endl;
            if(-1 == xioctl(VIDIOC_STREAMON, &buf.type))
            {
                cout << "Error : Start Capture"  << endl;
                errorEnLectura = 2;

                GImage imagen(0,0,GDIBUJO_IMAGEN_MODO_RGB);

                // return f2;
                return imagen;
            }

            init = 1;
        }

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval tv = {0};
        tv.tv_sec = 2;
        int r = select(fd+1, &fds, NULL, NULL, &tv);
        if(-1 == r)
        {
            cout << "Error: Waiting for Frame"  << endl;
            errorEnLectura = 3;
            // return f2;

            GImage imagen(0,0,GDIBUJO_IMAGEN_MODO_RGB);
            return imagen;
        }

        if(-1 == xioctl( VIDIOC_DQBUF, &buf))
        {
            cout << "Error: Retrieving Frame"  << endl;
            errorEnLectura = 4;
            // return f2;

            GImage imagen(0,0,GDIBUJO_IMAGEN_MODO_RGB);
            return imagen;
        }
        //  printf ("saving image\n");

    
        GImage imagen(ancho,altura,GDIBUJO_IMAGEN_MODO_RGB);

        
        // cv::Mat out_img = cv::Mat(altura,ancho,CV_8UC3, (unsigned*)buffer);
        imagen.setCharData((char *)buffer,ancho,altura);

        if ( reflejarVerticalmente == 0 )
        {
            // return out_img;
            return imagen;
        }

        imagen = GDibujo::flip(imagen,GDIBUJO_FLIP_HOR_VER);
        // flip(out_img,f2,-1);

        // return f2;

        return imagen;
    }
    else
    {                
        // espera la notificacion
        numEsperas = 0;
        // cout << currentTimeMS() << " GCAMARA: Solicita imagen" << endl;
        if ( etapaCapturaImagenCamara != ETAPA_IMG_CAMARA_IMG_LEIDA)
        {
            etapaCapturaImagenCamara = ETAPA_IMG_CAMARA_ESPERANDO_IMG;        
        }
        while( 1 )
        {            
            usleep(200);            
            numEsperas++;
            if ( numEsperas > 10000 )
            {                
                // Timeout
                numEsperas = -1;
                break;
            }

            if ( etapaCapturaImagenCamara == ETAPA_IMG_CAMARA_IMG_LEIDA ) break;
        }
        if ( numEsperas == -1 )
        {
            // cout << currentTimeMS()  << " GMARA: Timeout" << endl;
            errorEnLectura = 5;
            GImage imagen(0,0,GDIBUJO_IMAGEN_MODO_RGB);

            etapaCapturaImagenCamara = ETAPA_IMG_CAMARA_ESPERANDO_IMG;

            return imagen;
        }

        // cout << currentTimeMS() << " GCAMARA: Convierte imagen en formato OpenCV: " << numEsperas << endl;
        
        GImage imagen(ancho,altura,GDIBUJO_IMAGEN_MODO_RGB);

        // cv::Mat out_img = cv::Mat(altura,ancho,CV_8UC3, (unsigned*)buffer);
        imagen.setCharData((char *)buffer,ancho,altura);

        if ( reflejarVerticalmente == 0 )
        {
            // return out_img;
            // cout << currentTimeMS() << " GCAMARA: finaliza leeCamara sin FLIP " << endl;

            etapaCapturaImagenCamara = ETAPA_IMG_CAMARA_ESPERANDO_IMG;
            return imagen;
        }

        imagen = GDibujo::flip(imagen,GDIBUJO_FLIP_HOR_VER);
        // flip(out_img,f2,-1);

        // return f2;
        // cout << currentTimeMS() << " GCAMARA: finaliza leeCamara con FLIP" << endl;
        etapaCapturaImagenCamara = ETAPA_IMG_CAMARA_ESPERANDO_IMG;

        return imagen;
    }
}

int GCamara::xioctl( int request, void *arg)
{
    int r;

    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);

    return r;
}

int GCamara::print_caps()
{
        struct v4l2_capability caps = {};
        if (-1 == xioctl(VIDIOC_QUERYCAP, &caps))
        {
                perror("Querying Capabilities");
                return 1;
        }

        printf( "Driver Caps:\n"
                "  Driver: \"%s\"\n"
                "  Card: \"%s\"\n"
                "  Bus: \"%s\"\n"
                "  Version: %d.%d\n"
                "  Capabilities: %08x\n",
                caps.driver,
                caps.card,
                caps.bus_info,
                (caps.version>>16)&&0xff,
                (caps.version>>24)&&0xff,
                caps.capabilities);



        struct v4l2_cropcap cropcap = {0};
        CLEAR(cropcap);
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl (VIDIOC_CROPCAP, &cropcap))
        {
            cout << "error: Querying Cropping Capabilities" << endl;
            // return 1;
        }
        else
        {
            printf( "Camera Cropping:\n"
                "  Bounds: %dx%d+%d+%d\n"
                "  Default: %dx%d+%d+%d\n"
                "  Aspect: %d/%d\n",
                cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
                cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
                cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);
        }



        // int support_grbg10 = 0;

        struct v4l2_fmtdesc fmtdesc = {0};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        char c, e;
        printf("  FMT : CE Desc\n--------------------\n");
        while (0 == xioctl(VIDIOC_ENUM_FMT, &fmtdesc))
        {
                strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
                // if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
                //    support_grbg10 = 1;
                c = fmtdesc.flags & 1? 'C' : ' ';
                e = fmtdesc.flags & 2? 'E' : ' ';
                printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
                fmtdesc.index++;
        }


        struct v4l2_format fmt = {0};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = ancho;
        fmt.fmt.pix.height = altura;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (-1 == xioctl( VIDIOC_S_FMT, &fmt))
        {
            perror("Setting Pixel Format");
            return 1;
        }

        strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
        printf( "Selected Camera Mode:\n"
                "  Width: %d\n"
                "  Height: %d\n"
                "  PixFmt: %s\n"
                "  Field: %d\n",
                fmt.fmt.pix.width,
                fmt.fmt.pix.height,
                fourcc,
                fmt.fmt.pix.field);
        return 0;
}

int GCamara::init_mmap()
{
    struct v4l2_requestbuffers req = {0};
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(VIDIOC_REQBUFS, &req))
    {
        perror("Error: Requesting Buffer");
        return 1;
    }

    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(VIDIOC_QUERYBUF, &buf))
    {
        cout << "Error : Querying Buffer" << endl;
        return 1;
    }

    buffer = (uint8_t *) mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    printf("Image Length: %d\n", buf.bytesused);

    return 0;
}


/**
 * Ajusta un parametro del video
 * 
 *  parametro: valor definido en constantes PARAM_VIDEO_XXXXXX
 * 
 *  valor : valor double que se asigna al parametro
 */
void GCamara::ajustaVideo( int parametro, double valor )
{
    shared_ptr<GVector>parametroConf = make_shared<GVector>();

    parametroConf->add(parametro);
    parametroConf->add(valor);
    lstParamsVideo->add(parametroConf);
}