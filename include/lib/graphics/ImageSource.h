

#ifndef _IMAGE_SOURCE_
#define _IMAGE_SOURCE_

#include <string>

#include "lib/graphics/GDibujo.h"
#include "lib/general/GVector.h"
#include "lib/general/GThread.h"
#include "lib/graphics/gcamara.h"
#include "lib/general/GHashMap.h"
#include "lib/web/GSocket.h"


using namespace std;

/**
 * Clase que representa un proveedor de imagenes que sirve para
 * desacoplar la dependencia entre diferentes fuenes como camaras web, camaras fisicas,
 * broadcast web, archivos de videos.
 */
class ImageSource
{
    public:

        /**
         * Descripcion del ultimo error
         */
        string ultimoErrorDesc;

        /**
         * Codigo del ultimo error
         */
        int ultimoErrorCodigo;

        /**
         * Fecha de la ultima captura de imagen
         */
        long long fechaUltimaCaptura;

        /**
         * Lista de parametros.
         * Los elementos de esta lista son punteros a GNamedVector donde el primer elemento
         * es el nombre del parametro y el segundo el valor
         */
        GHashMap lstParams;

        /**
         * Constructor
         */
        ImageSource()
        {
            fechaUltimaCaptura = -1;
        }

        /**
         * Destructor
         */
        ~ImageSource()
        {

        }

        /**
         * Inicializa el proveedor
         * Debe retornar 0 en caso de exito, otro valor en caso de error
         */ 
        virtual int init() = 0;

        /**
         * Detiene el proceso de captura de imagenes
         */
        virtual void stop() = 0;

        /**
         * Libera el uso de recursos
         */
        virtual void release() = 0 ;

        /**
         * Retonra la siguiente imagen desde el proveedor
         */
        virtual GImage getImage() = 0;

        /**
         * Actualiza la configuracion del dispositivo fisico en base a una lista
         * de parametros.
         * 
         * @param paramNames lista de los nombres de parametros que se desea se actualicen,
         * estos nombres de parametros van separados por coma, no poner espacios entre ellos
         */
        virtual void updateConfig( string paramNames ) = 0;

        /**
         * Ajusta un parametro del video
         * 
         *  parametro: valor definido en constantes PARAM_VIDEO_XXXXXX
         * 
         *  valor : valor double que se asigna al parametro
         */
        virtual void ajustaVideo( int parametro, double valor ); 


        /**
         * Establece el valor de un parametro del tipo string
         */
        virtual void setStringParam( int param, string valor );

        /**
         * Se emplea para reiniciar a la camara usando la configuracion actual
         */
        virtual void restart() = 0;
};

/**
 * Clase base de un creador de imagesources
 * El objetivo es que las aplicaciones sean disenadas usando esta clase
 * para que se mantenga independencia de los image sources y 
 */
class ImageSourceFactory
{
    public:

        /**
         * Lista de parametros.
         * Los elementos de esta lista son punteros a GNamedVector donde el primer elemento
         * es el nombre del parametro y el segundo el valor
         */
        GHashMap lstParams;

        /**
         * Destructor
         */
        ~ImageSourceFactory()
        {

        }

        /**
         * Metodo que retorna una instancia de ImageSource
         */
        virtual shared_ptr<ImageSource> getInstance() = 0;

        /**
         * Metodo que retorna una instancia de ImageSource
         * Recibe como parametro un archivo con la configuracion del image source
         */
        virtual shared_ptr<ImageSource> getInstance( string configPath ) = 0;
};


/**
 * Proveedor de imagenes desde la camara interna del Raspberry PI
 */
class InternalCameraSource : public ImageSource
{
    public:

        /**
         * Nombre del parametro en el que se define la resolucion horizontal
         */
        static const string PARAM_RES_HORIZONTAL;

        /**
         * Nombre del parametro en el que se define la resolucion vertical
         */
        static const string PARAM_RES_VERTICAL;

        /**
         * Nombre del dispositivo que representa la camara
         */
        static const string PARAM_DEV_CAMARA;

        /**
         * Nombre del parametro entero que si tiene el valor 1 significa
         * que la imagen se debe reflejar verticalmente
         */
        static const string PARAM_REFLEJAR_VERTICALMENTE;

        /**
         * Camara con la que se trabaja
         */
        GCamara camara;

        /**
         * Destructor
         */
        ~InternalCameraSource()
        {

        }

        /**
         * Inicializa el proveedor
         */
        int init() override ;

        /**
         * Detiene el proceso de captura de imagenes
         */
        void stop() override;

        /**
         * Libera el uso de recursos
         */
        void release() override;

        /**
         * Retonra la siguiente imagen desde el proveedor
         */
        GImage getImage() override;

        /**
         * Actualiza la configuracion del dispositivo fisico en base a una lista
         * de parametros.
         * 
         * @param paramNames lista de los nombres de parametros que se desea se actualicen,
         * estos nombres de parametros van separados por coma, no poner espacios entre ellos
         */
        void updateConfig( string paramNames ) override;

        /**
         * Ajusta un parametro del video
         * 
         *  parametro: valor definido en constantes PARAM_VIDEO_XXXXXX
         * 
         *  valor : valor double que se asigna al parametro
         */
        void ajustaVideo( int parametro, double valor );        

        /**
         * Establece el valor de un parametro del tipo string
         */
        void setStringParam( int param, string valor ) override;

        /**
         * Se emplea para reiniciar a la camara usando la configuracion actual
         */
        void restart() override;
};


/**
 * Creador de ImageSource con la camara interna
 */
class InterImgSourceFactory : public ImageSourceFactory
{
    public:
         
        /**
         * Destructor
         */
        ~InterImgSourceFactory()
        {

        }

        /**
         * Se encarga de crear la fuente
         */
        shared_ptr<ImageSource> getInstance() override;

        /**
         * Metodo que retorna una instancia de ImageSource
         * Recibe como parametro un archivo con la configuracion del image source
         */
        shared_ptr<ImageSource> getInstance( string configPath ) override;
};


/**
 * Representa una camara 
 */
class Esp32SocketCamera : public ImageSource
{
    public:

        /**
         * Nombre del parametro string con la IP de la camara
         */
        static const string PARAM_IP;

        /**
         * Destructor
         */
        ~Esp32SocketCamera()
        {

        }

        /**
         * Inicializa el proveedor
         */
        int init() override ;

        /**
         * Detiene el proceso de captura de imagenes
         */
        void stop() override;

        /**
         * Libera el uso de recursos
         */
        void release() override;

        /**
         * Retonra la siguiente imagen desde el proveedor
         */
        GImage getImage() override;

        /**
         * Actualiza la configuracion del dispositivo fisico en base a una lista
         * de parametros.
         * 
         * @param paramNames lista de los nombres de parametros que se desea se actualicen,
         * estos nombres de parametros van separados por coma, no poner espacios entre ellos
         */
        void updateConfig( string paramNames ) override;


        /**
         * Establece el valor de un parametro del tipo string
         */
        void setStringParam( int param, string valor ) override;

        /**
         * Se emplea para reiniciar a la camara usando la configuracion actual
         */
        void restart() override;

    private:

        /**
         * Socket con el que accede a la camara
         */
        GSocket socket;    

        /**
         * Mascara con la que se ofuscan los datos que se envian
         */
        char mascaraOfuscadora[16];

        /**
         * Lee la mascara que ofusca los datoss
         */
        void leeMascaraOfuscadora();
};


class Esp32SocketCamFactory : public ImageSourceFactory
{
    public:
    
        /**
         * Destructor
         */
        ~Esp32SocketCamFactory()
        {

        }

        /**
         * Se encarga de crear la fuente
         */
        shared_ptr<ImageSource> getInstance() override;

        /**
         * Metodo que retorna una instancia de ImageSource
         * Recibe como parametro un archivo con la configuracion del image source
         */
        shared_ptr<ImageSource> getInstance( string configPath ) override;        

        
};


/**
 * Image source que representa a una camara ONVIF
 */
class OnvifCamera : public ImageSource , GThread
{
    public:

        /**
         * Parametro que contiene la IP del servidor
         */
        static const string PARAM_IP_SERVIDOR;

        /**
         * Parametro que contiene el puerto del servidor
         */
        static const string PARAM_PUERTO_SERVIDOR;

        /**
         * Parametro que contiene el usuario del servidor
         */
        static const string PARAM_USUARIO_SERVIDOR;

        /**
         * Parametro que contiene la password del usuario del servidor
         */
        static const string PARAM_PASSWORD_SERVIDOR;

        /**
         * Parametro que contiene el sufijo o parte final del URL para obtener
         */
        static const string PARAM_URL_SERVIDOR;

        /**
         * URL para acceder a la camara
         */
        string urlFinal;

        /**
         * Referencia a la camara
         */
        std::unique_ptr<cv::VideoCapture> cap;

        /**
         * Constructor
         */
        OnvifCamera();

        /**
         * Destructor
         */
        ~OnvifCamera()
        {

        }
    
        /**
         * Inicializa el proveedor
         */
        int init() override ;

        /**
         * Detiene el proceso de captura de imagenes
         */
        void stop() override;

        /**
         * Libera el uso de recursos
         */
        void release() override;

        /**
         * Retonra la siguiente imagen desde el proveedor
         */
        GImage getImage() override;

        /**
         * Actualiza la configuracion del dispositivo fisico en base a una lista
         * de parametros.
         * 
         * @param paramNames lista de los nombres de parametros que se desea se actualicen,
         * estos nombres de parametros van separados por coma, no poner espacios entre ellos
         */
        void updateConfig( string paramNames ) override;

        /**
         * Establece el valor de un parametro del tipo string
         */
        void setStringParam( int param, string valor ) override;

        /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Se emplea para reiniciar a la camara usando la configuracion actual
         */
        void restart() override;

    private:

        /**
         * Reinicia la camara
         */
        void reiniciaCamara();

        /**
         * IP del servidor
         */
        string ipServidor;

        /**
         * Puerto del servidor
         */
        string puertoServidor;

        /**
         * usuario del servidor
         */
        string usuarioServidor;

        /**
         * Password del servidor
         */
        string passwordServidor;

        /**
         * Sufijo del URL del servidor
         */
        string urlServidor;

        /**
         * Ultima imagen detectada
         */
        GImage imagenDet;

        /**
         * Valida si las imagenes a y b son iguales
         */
        bool frame_changed( cv::Mat &a, cv::Mat &b );        
};


/**
 * Factory que retorna una instancia de OnvifCamera
 */
class OnvifCameraFactory: public ImageSourceFactory
{
    public:

        /**
         * Destructor
         */
        ~OnvifCameraFactory()
        {

        }

        /**
         * Se encarga de crear la fuente
         */
        shared_ptr<ImageSource> getInstance() override;

        /**
         * Metodo que retorna una instancia de ImageSource
         * Recibe como parametro un archivo con la configuracion del image source
         */
        shared_ptr<ImageSource> getInstance( string configPath ) override;   
        
};



/**
 * Image source que representa a un video al que se le dese hacer analitica
 */
class VideoCamera : public ImageSource , GThread
{
    public:

        /**
         * Parametro que contiene la ruta del video
         */
        static const string PARAM_PATH_VIDEO;

        /**
         * Parametro que contiene la velocidad en ms del video
         */
        static const string PARAM_VELOCIDAD_VIDEO;

        /**
         * Nombre del parametro que contiene el parametro a partir del que se procesa el video
         */
         static const string PARAM_CUADRO_INICIAL;

         /**
         * Nombre del parametro que contiene el parametro que indica si se debe reproducir 
         * el video de forma infinita
         */
        static const string PARAM_INFINITO;
        
        /**
         * Referencia a la camara
         */
        std::unique_ptr<cv::VideoCapture> cap;

        /**
         * Cada cuentos ms debe generarse un video
         */
        long periodoVideo;

        /**
         * Ultima vez que se genero una imagen de video
         */
        long long ultimaVezGenVideo;

        /**
         * Reproduccion infinita
         */
        bool reproduccionInfinita;

        /**
         * NUmero de errores en la lectura de frames
         */
        int numErrores;

        /**
         * Constructor
         */
        VideoCamera();

        /**
         * Destructor
         */
        ~VideoCamera();
    
        /**
         * Inicializa el proveedor
         */
        int init() override ;

        /**
         * Detiene el proceso de captura de imagenes
         */
        void stop() override;

        /**
         * Libera el uso de recursos
         */
        void release() override;

        /**
         * Retonra la siguiente imagen desde el proveedor
         */
        GImage getImage() override;

        /**
         * Actualiza la configuracion del dispositivo fisico en base a una lista
         * de parametros.
         * 
         * @param paramNames lista de los nombres de parametros que se desea se actualicen,
         * estos nombres de parametros van separados por coma, no poner espacios entre ellos
         */
        void updateConfig( string paramNames ) override;

        /**
         * Establece el valor de un parametro del tipo string
         */
        void setStringParam( int param, string valor ) override;

        /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Se emplea para reiniciar a la camara usando la configuracion actual
         */
        void restart() override;

    private:

        /**
         * IP del servidor
         */
        string pathVideo;
        
        /**
         * Ultima imagen detectada
         */
        GImage imagenDet;
};



/**
 * Factory para image sources a partir de un video
 */
class VideoCameraFactory : public ImageSourceFactory
{
    public:

        /**
         * Destructor
         */
        ~VideoCameraFactory()
        {

        }

        /**
         * Se encarga de crear la fuente
         */
        shared_ptr<ImageSource> getInstance() override;

        /**
         * Metodo que retorna una instancia de ImageSource
         * Recibe como parametro un archivo con la configuracion del image source
         */
        shared_ptr<ImageSource> getInstance( string configPath ) override;   
};

#endif