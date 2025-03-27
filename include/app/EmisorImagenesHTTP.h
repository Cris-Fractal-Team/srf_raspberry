

#ifndef _EMISOR_IMAGENES_HTTP_
#define _EMISOR_IMAGENES_HTTP_

#include <thread>
#include <mutex>

#include "lib/web/GHttpServer.h"
#include "lib/web/GHttpServerCommand.h"
#include "lib/web/GHttpRequest.h"
#include "lib/web/GHttpResponse.h"
#include "lib/graphics/GDibujo.h"
// #include "app/IdentificadorFacial.h"


class IdentificadorFacial;
class ServidorHttpImagenes;
class ProcesoRecFacial;

/**
 * Clase que procesa el comando que envia una imagen de lo enfocado por la camara
 * EL URL al que este comendo atiende es : /getimage
 */
class GetImageCommand : public GHttpServerCommand
{
    public:

        /**
         * Referencia al objeto que gestiona la WEB
         */
        ServidorHttpImagenes *serverImg;

        /**
         * Constructor
         */
        GetImageCommand();

        /**
         * Destructor
         */
        ~GetImageCommand();

        /**
         * Procesa una peticion
         */
        void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion ) override;
};

/**
 * Clase que procesa el comando que envia una imagen de lo enfocado por la camara
 * EL URL al que este comendo atiende es : /getimage
 */
class GetVideoCommand : public GHttpServerCommand
{
    public:

        /**
         * Referencia al objeto que gestiona la WEB
         */
        ServidorHttpImagenes *serverImg;

        /**
         * Constructor
         */
        GetVideoCommand();

        /**
         * Destructor
         */
        ~GetVideoCommand();
      

        /**
         * Procesa una peticion
         */
        void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion ) override;

   
};


/**
 * Clase que procesa el comando que envia una imagen de lo enfocado por la camara
 * en una resolucion baja de 600 puntos maximo
 * EL URL al que este comendo atiende es : /getimage
 */
class GetPreviewCommand : public GHttpServerCommand
{
    public:

        /**
         * Referencia al objeto que gestiona la WEB
         */
        ServidorHttpImagenes *serverImg;

        /**
         * Constructor
         */
        GetPreviewCommand();

        /**
         * Destructor
         */
        ~GetPreviewCommand();
      

        /**
         * Procesa una peticion
         */
        void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion ) override;

   
};

/**
 * Clase que procesa el comando de login
 * EL URL al que este comendo atiende es : /login
 */
class LoginCommand : public GHttpServerCommand
{
    public:

        /**
         * Referencia al objeto que gestiona la WEB
         */
        ServidorHttpImagenes *serverImg;

        /**
         * Constructor
         */
        LoginCommand();

        /**
         * Destructor
         */
        ~LoginCommand();

        /**
         * Procesa una peticion
         */
        void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion ) override;
        
};


/**
 * Clase que representa un servidor web que recibe comandos de configuracion
 * y adicionalmente muestra lo enfocado por la camara
 */
class ServidorHttpImagenes
{
    public:

        /**
         * Lista de parametros de la aplicacion
         */
        shared_ptr<GHashMap>lstParamsApp;

        /**
         * Referencia al identificador facial V1
         */
        IdentificadorFacial *identificadorFac;

        /**
         * Referencia al identificador facial V2 basado en Hailo
         */
        ProcesoRecFacial *procRecFacial;

        /**
         * Constructor
         */
        ServidorHttpImagenes();

        /**
         * Inicia el servidor
         */
        void iniciar();

        /**
         * Finaliza el servidor
         */
        void finalizar();

        /**
         * Establece la imagen actual del sensor
         */
        void setImage( GImage imagen );

              
        /**
         * Retorna el buffer de la ultima imagen
         */
        std::vector<uchar> getJpegBuffer();
        
        /**
         * Agrega un response para que se envie video
         * EL video puede ser en una resolucion mas baja como una previsualizacion
         */
        void addVideoResponse( shared_ptr<GHttpResponse> response, bool esPreview );
        
    protected:

        /**
         * Metodo que envia las imagenes a los visores
         * Se ejecuta dentro de un Thread
         */
        void bucleEnvioImagenes();


        /**
         * Metodo que ejecuta el bucle del servidor
         */
        void ejecutaServidor();

        /**
         * Imagen actual
         */
        GImage currentImage;

    private:

        /**
         * Servidor con el que se trabaja
         */
        GHttpServer servidor;

        /**
         * Indica que hay una imagen nueva para previsualizar
         */
        volatile bool imagenNueva;

        /**
         * Lista de responses a los que se debe mandar la imagen actualizada en tamanio grande
         */
        GVector lstVideoResponses;

        /**
         * Lista de responses a los que se debe mandar la imagen actualizada en modo previsualizacion
         */
        GVector lstPreviewResponses;

        /**
         * Buffer de la ultima imagen JPEG recibida
         */
        std::vector<uchar> jpegBuffer;

        /**
         * Buffer de la ultima imagen JPEG recibida
         */
        std::vector<uchar> jpegPreviewBuffer;

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;
};

#endif

