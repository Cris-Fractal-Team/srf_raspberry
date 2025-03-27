

#ifndef GHTTPSERVERTHREAD_H
#define GHTTPSERVERTHREAD_H

#include <string>
#include <thread>
#include <mutex>

#include "GSocket.h"
#include "HttpHeaderReader.h"
#include "lib/general/GVector.h"
#include "lib/general/GHashMap.h"
#include "GHttpResponse.h"
#include "GHttpRequest.h"
#include "GHttpServer.h"
#include "GHttpServerCommand.h"
#include "lib/general/GThread.h"

using namespace std;


class GHttpServer;


/**
 * Representa un thread que esta ejecutandose en un servidor GHttpServer.
 * Procesa peticiones
 **/
class GHttpServerThread : public GThread
{
    public:

        /**
         * Constructor
         */
        GHttpServerThread( GHttpServer *servidorPdr );

        /**
         * Indica que se debe procesar el cliente en paralelo
         */
        void procesarCliente( GSocket cliente );

        /**
         * Funcion que ejecuta el proceso en paralelo
         */
        virtual void runThread();

        /**
         * Retorna referencia al socket
         */
        GSocket getSocket();
        

    private:
        
        /**
         * Socket del cliente
         */
        GSocket sockCliente;

        /**
         * Referencia al servidor
         */
        GHttpServer *servidor;

        
};


#endif
