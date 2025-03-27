
#ifndef COMANDO_TEST
#define COMANDO_TEST 

#include "lib/web/GSocket.h"
#include "lib/general/GVector.h"
#include "lib/general/GObject.h"
#include "lib/general/GHashMap.h"
#include "lib/web/GHttpServer.h"
#include "lib/web/GWebSocketCommand.h"
#include "lib/graphics/gcamara.h"


class ComandoTest : public GHttpServerCommand
{
    public:

        /**
         * Constructor simple
         */
        ComandoTest();        
        
         /**
         * Procesa una peticion
         */
        virtual void procesaRequest( GHttpRequest *request, GHttpResponse *response );
};

class WSCommandTest;

class BroadCastImagen : public GThread
{
    public:

        /**
         * Contenedor al que se notifica o envia las imagenes capturadas
         */
        WSCommandTest *contenedor;

        /**
         * Constructor
         */
        BroadCastImagen();

        /**
         * Inicializa la camara
         */
        void inicializa();

        /**
         * Funcion que ejecuta el proceso en paralelo
         */
        virtual void runThread();

        /**
         * Envia la imagen
         */
        void envia( GImage imagen );

    private:

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Imagen que se debe enviar
         */
        GImage imagenNueva;

        /**
         * Indica si hay una imagen nueva o no
         */
        bool hayImagenNueva;
};

class BroadCastCamara: public GThread
{
    public:

        /**
         * Contenedor al que se notifica o envia las imagenes capturadas
         */
        WSCommandTest *contenedor;

        /**
         * Camara con la que trabaja
         */
        GCamara camara;

        /**
         * Objeto que envia los datos de las imagenes
         */
        BroadCastImagen broadcaster;

        int anchoVideo;

        int alturaVideo;

        /**
         * Constructor
         */
        BroadCastCamara();

        /**
         * Inicializa la camara
         */
        void inicializa();

        /**
         * Funcion que ejecuta el proceso en paralelo
         */
        virtual void runThread();
};


/**
 * Ejemplo de clase que procesa mensajes WebSocket
 */
class WSCommandTest: public GWebSocketCommand
{
    public:

        /**
         * Objeto que le envia las imagenes
         */
        BroadCastCamara broadcaster;

        /**
         * Cadena que se emplea para generar la data con las imagenes que se envian
         */
        string cadenaBase;

    
        /**
         * Constructor
         */
        WSCommandTest();

        /**
         * Metodo que envia a todos los clientes las imagenes de la camara
         */
        void notificaImagen( char *buffer64, int len );

        /**
         * Invocado cada vez que un cliente se conecta
         */
        virtual void onNewClient( GWebSocketSession *session );

        /**
         * Invocado cada vez que un cliente se desconecta
         */
        virtual void onExitClient( GWebSocketSession *session );

        /**
         * Invocado cada vez que llega un nuevo mensaje de texto
         */
        virtual void onTextMessage( GWebSocketSession *session, char *buffer, int len );

        /**
         * Invocado cada vez que llega un nuevo mensaje de texto
         */
        virtual void onBinMessage( GWebSocketSession *session, char *buffer, int len );
};

#endif