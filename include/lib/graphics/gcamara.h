#include <stdio.h>
#include <cmath>
#include <chrono>
// #include <opencv2/opencv.hpp>
// #include <opencv2/highgui.hpp>
// #include <opencv2/imgcodecs.hpp>

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "GDibujo.h"

#include <lib/general/GVector.h>
#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>

#ifndef GCAMARA
#define GCAMARA

// Definicion de constantes que ajustan el video

#define PARAM_VIDEO_BRILLO 1
#define PARAM_VIDEO_CONTRASTE 2
#define PARAM_VIDEO_EXPOSICION 3

using namespace cv;
using namespace libcamera;

/**
 * Representa un lector de camara usando el driver de video de linux VL2
 */
class GCamara
{
    public:

        /**
         * Indica si ocurrio un error en la ultima lectura
         */
        int errorEnLectura;

        /**
         * Indica si se debe reflejar verticalmente las imagenes :
         * 0 : No
         * 1 : Si
         */
        int reflejarVerticalmente;

        /**
         * Indica si se debe usar LibCamara
         */
        bool usarLibCamara;

        /**
         * Referencia a la camara adminsitrada por LIBCAMERA
         */
        std::shared_ptr<Camera> camera;

        /**
         * Lista de vectores donde el 1er elemento es el codigo del parametro de video y el 2do elemento
         * es el valor de ese parametro, el valor del parametro esta definido
         * por las constantes PARAM_VIDEO_XXXX
         */
        shared_ptr<GVector> lstParamsVideo;

        /**
         * Constructor
         */
        GCamara();

        /**
         * Destructur
         */
        ~GCamara();

        /**
         * Establece las dimensiones de la imagen que se captura
         */
        void setDim( int ancho, int altura );

        /**
         * Establece el nombre del dispositivo con el que se trabaja
         */
        void setCamara( string name );

        /**
         * Inicia la captura
         */
        int iniciarCaptura();

        /**
         * Finaliza la captura
         */
        void detenerCaptura();

        /**
         * Indica si esta iniciada o no la camara
         */
        int getIniciado();

        /**
         * Lee la siguiente imagen de la camara
         */
        // Mat leerCamara();
        GImage leerCamara();


        /**
         * Retorna el ancho de las imagenes
         */
        int getAncho();

        /**
         * Retorna la altura
         */
        int getAltura();

        /**
         * Retorna el buffer para almacenar una imagen de la camara
         */
        uint8_t *getBufferCamara();

        /**
         * Ajusta un parametro del video
         * 
         *  parametro: valor definido en constantes PARAM_VIDEO_XXXXXX
         * 
         *  valor : valor double que se asigna al parametro
         */
        void ajustaVideo( int parametro, double valor );        

    private:

        /**
         * Lee los datos desde el driver
         */
        int xioctl( int request, void *arg);

        /**
         * Imprime configuraciones
         */
        int print_caps();

        /**
         * Inicializa el modo de video
         */
        int init_mmap();

        /**
         * Identificador de la camara
         */
        int fd;

        /**
         * Ancho en pixeles de la imagen que captura
         */
        int ancho;

        /**
         * Altura en pixeles de la captura
         */
        int altura;

        /**
         * Nombre del dispositivo que se desea usar
         */
        string nombreCamara;

        /**
         * Indica si se ha iniciado la ejecucion de la camara
         * 0 : No iniciado
         * 1 : Si iniciado
         */
        volatile char iniciado;

        /**
         * Buffer con la configuracion
         */
        struct v4l2_buffer buf;

        /**
         * Inidica si se ha iniciado la captura
         */
        int init;
                        
        /**
         * Descriptor de datos
         */
        fd_set fds;

        /**
         * Buffer en el que se guardan los datos
         */
        uint8_t *buffer;

        /**
         * Administrador de la camara con LIBCAMERA
         */
        CameraManager cam_manager;

        

        /**
         * Flujo de datos desde el que se obtiene una imagen de LIBCAMERA
         */
        Stream *stream;

        /**
         * Creador de bufferes de imagenes para LIBCAMERA
         */
        FrameBufferAllocator *allocator;

        /**
         * Peticiones de imagenes a la camara
         */
        std::vector<std::unique_ptr<Request>> requests;

};


#endif
