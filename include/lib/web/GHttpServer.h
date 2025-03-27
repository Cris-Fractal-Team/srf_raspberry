

#ifndef GHTTPSERVER_H
#define GHTTPSERVER_H

#include <string>
#include <thread>
#include <mutex>

#include "GSocket.h"
#include "HttpHeaderReader.h"
#include "lib/general/GVector.h"
#include "lib/general/GHashMap.h"
#include "GHttpResponse.h"
#include "GHttpRequest.h"
#include "GHttpServerCommand.h"
#include "GHttpServerThread.h"
#include "lib/general/GThreadPool.h"


using namespace std;

class GHttpServerCommand;


/**
 * Session HTTP de un usuario
 */
class GHttpServerSession : public GObject
{
    public:

        /**
         * ID de la sesion
         */
        string idSession;

        /**
         * Lista con las variables almacenadas
         */
        GHashMap lstValores;

};




/**
 * Servidor http simple
 */
class GHttpServer
{
    public:

        /**
         * Lista de comandos almacenados
         */
        GVector lstComandos;

        /**
         * Lista de sesiones WEB
         */
        GVector lstSesiones;

        /**
         * Path de componentes estaticos sin seguridad
         */
        string pathEstatico;

        /**
         * Constructor
         */
        GHttpServer();

        /**
         * Inicia el servidor
         * 
         * ip : ip que monitorea
         * 
         * puerto : puerto IP asociado
         */
        void iniciar( string ip, int puerto );

        /**
         * Detiene el servidor
         */
        void finalizar();

        /**
         * Agrega un comando al servidor
         */
        void addCommand( shared_ptr<GHttpServerCommand > puntero );

        /**
         * Crea una nueva sesion WEB
         * 
         * reader : letor de la cabecera desde el que se crea la sesion
         */
        shared_ptr<GHttpServerSession> creaNuevaSesion( shared_ptr<GHttpResponse> response );

        /**
         * Dado el ID de una sesion retorna sus datos
         * 
         * id : identificador de la sesion
         * 
         * Retorna la sesion o NULL en caso no exista
         */
        shared_ptr<GHttpServerSession> getSession( string id);

        /**
         * Dado el request y el ID del cookie en el que se guardar el ID de una sesion
         * retorna la sesion con el ID.
         * 
         * Retorna la sesion o NULL en caso no exista
         */
        shared_ptr<GHttpServerSession> getSession( shared_ptr<GHttpRequest> request );

        /**
         * Busca el comando que tiene un URL dado y se llama al metodo procesaRequest
         */
        void redirecciona( string url, shared_ptr<GHttpRequest>request, shared_ptr<GHttpResponse> response );

        /**
         * Dado el socket de un cliente lo analiza e identifica
         * el comando que debe procesar la peticion
         */
        void ubicaComando( GSocket cliente );

        /**
         * Elimina una sesion
         */
        void delSession( string id );

        /**
         * Valida si el servidor esta o no en ejecucion
         */
        bool enEjecucion();
        
    private:

        /**
         * Servidor socke con el que se trabaja
         */
        GSocket socketServer;

        /**
         * ID de la sigueinte sesion
         */
        long long idSgteSession;

        /**
         * Cantidad minima de threads
         */
        int minNumThread;

        /**
         * Cantidad maxima de threads
         */
        int maxNumThread;

        /**
         * Pool de conexiones
         */
        GThreadPool thPool;

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Procesa una peticion con un comando
         * Retorna false si no se proceso, true en caso si se haya procesado
         */
        bool procesarPeticion( shared_ptr<HttpHeaderReader> reader, shared_ptr<GHttpServerCommand> comando, GSocket socket );

        

        /**
         * Retorna un thread para procesar un request
         */
        shared_ptr<GHttpServerThread> getThread();

        /**
         * Inicializa el pool de threads
         */
        void initThreadPool();


        
};


#endif
