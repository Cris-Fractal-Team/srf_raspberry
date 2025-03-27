

#ifndef GWEBSOCKETTHREAD_H
#define GWEBSOCKETTHREAD_H

#include <string>
#include <thread>
#include <mutex>

#include "GSocket.h"
#include "HttpHeaderReader.h"
#include "lib/general/GVector.h"
#include "lib/general/GHashMap.h"
#include "GHttpResponse.h"
#include "GHttpRequest.h"
#include "GWebSocketCommand.h"
#include "lib/general/GThread.h"

using namespace std;

class GWebSocketSession;


/**
 * Representa un thread que esta ejecutandose en un servidor GHttpServer.
 * Procesa peticiones
 **/
class GWebSocketThread : public GThread
{
    public:

        /**
         * Constructor
         */
        GWebSocketThread();

        /**
         * Indica que se debe procesar el cliente en paralelo
         */
        void procesarCliente( shared_ptr<GWebSocketSession> session );

        /**
         * Funcion que ejecuta el proceso en paralelo
         */
        virtual void runThread();
        

    private:
        
        /**
         * Sesion del cliente con el que trabaja
         */
        shared_ptr<GWebSocketSession> wsSession;
                
        /**
         * Buffer en el que se guarda el mensaje actual
         */
        char *bufferLectura;


        /**
         * Cantidad de bytes del buffer, se usa cuando el cliente envia mensajes en bloques
         */
        unsigned int lenBufferLectura;

        /**
         * Indica que se presento un error en la lectura y se debe terminar
         * 0: Sin error.
         * 1: codigo de operacion no reconocido
         * 2: error al leer la cantidad de bytes del mensaje
         * 3: error al leer la mascara
         * 4: error de lectura de los datos
         */
        int errorLectura;

        /**
         * Procesa la cabecera de la lectura de un mensaje de WebSocket
         */
        void procesaLecturaCabMensaje( char *buffer );

        /**
         * Lee la longitud del mensaje desde la cabecera donde los siguientes 2 bytes representan 
         * la cantidad de bytes del mensaje         
         */
        unsigned int leeLongitudMsg16Bits();

        /**
         * Lee la longitud del mensaje desde la cabecera donde los siguientes 8 bytes representan 
         * la cantidad de bytes del mensaje         
         */
        unsigned int leeLongitudMsg64Bits();

        /**
         * Procesa la continuacion de un mensaje previo
         */
        void procesaContinuacionMsgPrevio( char bitFin, char *buffer );

        /**
         * Procesa la continuacion de un mensaje de texto
         */
        void procesaMsgTexto( char bitFin, char *buffer );

        /**
         * Procesa el envio de un mensaje binario
         */
        void procesaMsgBinario( char bitFin, char *buffer );

        /**
         * Procesa el envio de un mensaje PING
         */
        void procesaMsgPing( char bitFin, char *buffer );

        /**
         * Procesa el envio de un mensaje PONG
         */
        void procesaMsgPong( char bitFin, char *buffer );

        /**
         * Lee los datos enviados via un mensaje
         */
        void leeDatos(unsigned int msgLen,unsigned char *mascaraMensaje);
        
};

#endif