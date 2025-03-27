

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
         * Lista de parametros.
         * Los elementos de esta lista son punteros a GNamedVector donde el primer elemento
         * es el nombre del parametro y el segundo el valor
         */
        GHashMap lstParams;

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

    private:

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

#endif