
#ifndef GWEBSOCKETCOMMAND_H
#define GWEBSOCKETCOMMAND_H

#include <string>

#include "GSocket.h"
#include "HttpHeaderReader.h"
#include "lib/general/GVector.h"
#include "lib/general/GHashMap.h"
#include "GHttpRequest.h"
#include "GHttpServerCommand.h"
#include "GHttpResponse.h"
#include "GHttpServer.h"
#include "GWebSocketThread.h"

using namespace std;

class GWebSocketSession;
class GWebSocketThread;

/**
 * Configura un comando que procesa clientes con WebSocket
 */
class GWebSocketCommand : public GHttpServerCommand, public std::enable_shared_from_this<GWebSocketCommand>
{
    public:

        /**
         * Cantidad minima de threads
         */
        static int minNumThread;

        /**
         * Cantidad maxima de threads
         */
        static int maxNumThread;

        /**
         * Indica si el pool de conexiones se inicializo
         */
        static bool poolConInicializado;       
                
        /**
         * Pool de conexiones. Es compartido entre todas las instancias de WebSocket
         */
        static GThreadPool thPool;

        /**
         * Mutex para areas criticas del pool
         */
        static std::mutex mtxPool;

        
        /**
         * Constructor simple
         */
        GWebSocketCommand();        
        
         /**
         * Procesa una peticion
         */
        virtual void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response );

        /**
         * Invocado cada vez que un cliente se conecta
         */
        virtual void onNewClient( shared_ptr<GWebSocketSession> session );

        /**
         * Invocado cada vez que un cliente se desconecta
         */
        virtual void onExitClient( shared_ptr<GWebSocketSession> session );

        /**
         * Invocado cada vez que llega un nuevo mensaje de texto
         */
        virtual void onTextMessage( shared_ptr<GWebSocketSession> session, char *buffer, int len );

        /**
         * Invocado cada vez que llega un nuevo mensaje de texto
         */
        virtual void onBinMessage( shared_ptr<GWebSocketSession> session, char *buffer, int len );

        /**
         * Envia un mensaje de texto a un cliente
         */
        void sendTextMessage( shared_ptr<GWebSocketSession> session, char *message, int len );

        /**
         * Retorna la cantidad de sesiones actuales
         */
        int getLstSessionSize();

        /**
         * Retorna una session dada su ubicacion
         * 
         * returna NULL en caso ya no exista la sesion o la referencia a la sesion
         */
        shared_ptr<GWebSocketSession> getSessionAt( int index );

        /**
         * Elimina una sesion dado su ID
         */
        void removeSession( long long idSession );


    private:

        /**
         * ID de la sigueinte sesion
         */
        long long idSgteSession;

        /**
         * Lista de sesiones
         */
        GVector lstSesiones;        

        /**
         * Mutex interno
         */
        std::mutex mtxInterno;


        /**
         * Crea una nueva sesion
         */
        shared_ptr<GWebSocketSession> creaNuevaSession( GSocket socket );        

        /**
         * Retorna un thread para procesar un request
         */
        static shared_ptr<GWebSocketThread> getThread();

        /**
         * Inicializa el pool de threads
         */
        static void initThreadPool();

};

class GWebSocketSession : public GObject,  public std::enable_shared_from_this<GWebSocketSession>
{
    public:

        long long idSession;

        GSocket socket;
        
        shared_ptr<GWebSocketCommand> comandoPadre;

        /**
         * Envia un mensaje de texto
         */
        void sendTextMessage( char *message, int len );


    private:

        /**
         * Mutex interno
         */
        std::mutex mtxInterno;

};

#endif

