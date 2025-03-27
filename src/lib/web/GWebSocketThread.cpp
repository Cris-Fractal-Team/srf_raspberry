

#include <string>
#include <thread>
#include <mutex>
#include <iostream>

#include "lib/web/GSocket.h"
#include "lib/general/GThread.h"
#include "lib/web/GWebSocketThread.h"
#include "lib/web/GWebSocketCommand.h"

using namespace std;

/**
 * Posicion del BIT del primer byte de la cabecera que representa el parametro FIN
 */
#define BIT_MSG_FIN 128

/**
 * Codigo de los BITs de reserva en la cabecera del mensaje que indica si el frame es la continuacion del mensaje anterior
 * Es para casos en los que el mensaje ha sido dividido en varios frames
 */
#define OP_CONTINUA 0

/**
 * Codigo de la operacion en la cabecera del mensaje que indica mensaje de texto
 */
#define OP_MSG_TEXTO 1

/**
 * Codigo de operacion en la cabecera del mensaje que indica mensaje binario
 */
#define OP_MSG_BINARIO 2

/**
 * Codigo de operacion en la cabecera del mensaje que indica el cliente desea finalizar la conexion
 */
#define OP_CLIENTE_FINALIZA 8

/**
 * Codigo de operacion en la cabecera del mensaje que indica mensaje de PING
 */
#define OP_MSG_PING 9

/**
 * Codigo de operacion en la cabecera del mensaje que indica mensaje de PONG
 */
#define RSV_MSG_PONG 10



/**
* Constructor
*/
GWebSocketThread::GWebSocketThread()
{
    lenBufferLectura = 0;
}

/**
    * Indica que se debe procesar el cliente en paralelo
    */
void GWebSocketThread::procesarCliente( shared_ptr<GWebSocketSession> session )
{
    wsSession = session;
    start();
}

/**
* Funcion que ejecuta el proceso en paralelo
*/
void GWebSocketThread::runThread()
{
    unsigned char buffer[2];

    wsSession->comandoPadre->onNewClient(wsSession);
    lenBufferLectura = 0;

    cout << "Inicia proceso de cliente WebSocket ID sesion: " << wsSession->idSession << " ID unico: " << wsSession->id_unico << endl;

    // Debe procesar todos los mensajes en paralelo
    try
    {
        while( true )
        {
            cout << endl;
            cout << "Valiando el estado del socket" << endl;
            if ( wsSession->socket.estaConectado() == 0 )
            {
                cout << "Se finaliza la conexion websocket pues el socket esta cerrado" << endl;
                break;
            }

            cout << "Tratando de leer bytes de la cabecera WebSocket con id: " << id_unico  << endl;
            wsSession->socket.readLen((char *)buffer,2);
            if ( wsSession->socket.estaConectado() == 0 )
            {
                cout << "Error al leer cabecera de mensaje WebSocket:" << endl;
                break; 
            }

            // cout << "Se recibieron dos datos del websocket:" << (int)buffer[0] << " , " << (int)buffer[1] << endl;
            procesaLecturaCabMensaje((char *)buffer);
            if ( errorLectura != 0 )
            {
                if ( errorLectura != -1 )
                    cout << "Se finaliza la sesion WebSocket por error en lectura de datos:" << errorLectura << endl;
                else
                    cout << "Se finaliza la sesion WebSocket a peticion del cliente" << endl;
                break;
            }
        }
        wsSession->comandoPadre->onExitClient(wsSession);
        wsSession->socket.cerrar();
    }
    catch(const std::runtime_error& e)
    {
        wsSession->comandoPadre->onExitClient(wsSession);
        
        std::cerr << e.what() << '\n';        
        cout << "***** Finaliza la sesion WebSocket pues se cerro el socket" << endl;
    }    

    cout << "FInaliza proceso de cliente WebSocket ID sesion: " << wsSession->idSession << " ID unico: " << wsSession->id_unico << endl;
    wsSession->comandoPadre->removeSession(wsSession->idSession);
}

/**
 * Procesa la cabecera de la lectura de un mensaje de WebSocket
 */
void GWebSocketThread::procesaLecturaCabMensaje( char *buffer )
{
    char cabFin,operacion;

    cout << "LenBuffer lectura en procesa Lectura Cab Mensaje : " << lenBufferLectura << endl;

    errorLectura = 0;
    bufferLectura = NULL;

    cabFin = buffer[0] & BIT_MSG_FIN;
    operacion = ( buffer[0] & 15);

    cout << "Fin :" << (int)cabFin << endl;
    cout << "Operacion :" << (int)operacion << endl;
    
    if ( operacion == OP_CONTINUA )
    {
        procesaContinuacionMsgPrevio(cabFin,buffer);
    }
    else
    if ( operacion == OP_MSG_TEXTO )
    {
        procesaMsgTexto(cabFin,buffer);
    }
    else
    if ( operacion == OP_MSG_BINARIO )
    {
        procesaMsgBinario(cabFin,buffer);
    }
    else
    if ( operacion == OP_MSG_PING )
    {
        procesaMsgPing(cabFin,buffer);
    }
    else
    if ( operacion == OP_CONTINUA )
    {
        procesaMsgPong(cabFin,buffer);
    }
    else
    if ( operacion == OP_CLIENTE_FINALIZA )
    {
        errorLectura = -1;
    }
    else
    {
        cout << "Codigo de operacion en cabecera WebSocket no reconocido:" << operacion << endl;
        errorLectura = 1;
    }

    if ( cabFin == BIT_MSG_FIN )
    {
        lenBufferLectura = 0;
        free(bufferLectura);
    }
}

/**
 * Procesa la continuacion de un mensaje previo
 */
void GWebSocketThread::procesaContinuacionMsgPrevio( char bitFin, char *buffer )
{

}

/**
 * Procesa la continuacion de un mensaje de texto
 */
void GWebSocketThread::procesaMsgTexto( char bitFin, char *buffer )
{
    unsigned int msgLen;
    char mascara;
    unsigned char mascaraMensaje[4];

    mascara = buffer[1] & 128;
    msgLen = (buffer[1] & 127 );

    // cout << "Longitud de la cadena inicial :" << msgLen << endl;

    if ( msgLen == 126 )
    {
        msgLen = leeLongitudMsg16Bits();
        // cout << "Longitud final:" << msgLen << endl;
        if ( msgLen == 0 )
        {
            errorLectura = 2;
            return;
        }
    }
    else
    if ( msgLen == 127 )
    {
        msgLen = leeLongitudMsg64Bits();
        // cout << "Longitud final:" << msgLen << endl;
        if ( msgLen == 0 )
        {
            errorLectura = 2;
            return;
        }
    }

    if ( mascara != 0 )
    {
        wsSession->socket.readLen((char *)mascaraMensaje,4);
        // cout << "Mascara: " << (int) mascaraMensaje[0] << " " << (int) mascaraMensaje[1] << " " << (int) mascaraMensaje[2] << " " << (int) mascaraMensaje[3] << endl;

        if ( wsSession->socket.estaConectado() == 0 )
        {
            errorLectura = 3;
            return;
        }
        leeDatos(msgLen,mascaraMensaje);
        if ( errorLectura != 0 ) return;

        wsSession->comandoPadre->onTextMessage(wsSession,bufferLectura,msgLen);
    }
    else
    {
        leeDatos(msgLen,NULL);
        if ( errorLectura != 0 ) return;
        wsSession->comandoPadre->onTextMessage(wsSession,bufferLectura,msgLen);
    }    
}

/**
 * Procesa el envio de un mensaje binario
 */
void GWebSocketThread::procesaMsgBinario( char bitFin, char *buffer )
{
    unsigned int msgLen;
    char mascara;
    unsigned char mascaraMensaje[4];

    mascara = buffer[1] & 128;
    msgLen = (buffer[1] & 127 );

    if ( msgLen == 126 )
    {
        msgLen = leeLongitudMsg16Bits();
        if ( msgLen == 0 )
        {
            errorLectura = 2;
            return;
        }
    }
    else
    if ( msgLen == 127 )
    {
        msgLen = leeLongitudMsg64Bits();
        if ( msgLen == 0 )
        {
            errorLectura = 2;
            return;
        }
    }

    if ( mascara != 0 )
    {
        wsSession->socket.readLen((char *)mascaraMensaje,4);
        if ( wsSession->socket.estaConectado() == 0 )
        {
            errorLectura = 3;
            return;
        }
        leeDatos(msgLen,mascaraMensaje);
        if ( errorLectura != 0 ) return;

        wsSession->comandoPadre->onBinMessage(wsSession,bufferLectura,msgLen);
    }
    else
    {
        leeDatos(msgLen,NULL);
        if ( errorLectura != 0 ) return;

        wsSession->comandoPadre->onBinMessage(wsSession,bufferLectura,msgLen);
    }
    
}

/**
 * Lee los datos enviados via un mensaje
 */
void GWebSocketThread::leeDatos(unsigned int msgLen,unsigned char *mascaraMensaje)
{
    unsigned int posIni;
            
    posIni = lenBufferLectura;
    lenBufferLectura+= msgLen;

    if ( posIni > 0 )
    {        
        bufferLectura = (char *)realloc(bufferLectura,lenBufferLectura);
    }
    else
    {
        bufferLectura = (char *)malloc(lenBufferLectura);
    }

    wsSession->socket.readLen(&bufferLectura[posIni],msgLen);
    if ( wsSession->socket.estaConectado() == 0 )
    {
        errorLectura = 4;
    }

    if (  mascaraMensaje != NULL )
    {
        for(unsigned int i=0;i<msgLen;i++)
        {
            bufferLectura[posIni] = bufferLectura[posIni]  ^ mascaraMensaje[i&3];
            posIni++;
        }
    }
}

/**
 * Procesa el envio de un mensaje PING
 */
void GWebSocketThread::procesaMsgPing( char bitFin, char *buffer )
{
    
}

/**
 * Procesa el envio de un mensaje PONG
 */
void GWebSocketThread::procesaMsgPong( char bitFin, char *buffer )
{
    
}


/**
 * Lee la longitud del mensaje desde la cabecera donde los siguientes 2 bytes representan 
 * la cantidad de bytes del mensaje         
 */
unsigned int GWebSocketThread::leeLongitudMsg16Bits()
{
    unsigned char buffer[2];
    unsigned int rpta;

    wsSession->socket.readLen((char *)buffer,2);
    if ( wsSession->socket.estaConectado() == 0 )     
    {
        rpta = 0;
    }
    else
    {
        cout << "Lee longitud 16Bits " << (unsigned int) buffer[0] << " " << (unsigned int) buffer[1] << endl; 
        rpta = ((unsigned int)buffer[0]<<8) + ((unsigned int)buffer[1])  ;
    }
    return rpta;
}

/**
 * Lee la longitud del mensaje desde la cabecera donde los siguientes 8 bytes representan 
 * la cantidad de bytes del mensaje         
 */
unsigned int GWebSocketThread::leeLongitudMsg64Bits()
{
    char buffer[8];
    unsigned int rpta;

    wsSession->socket.readLen(buffer,8);
    if ( wsSession->socket.estaConectado() == 0 )     
    {
        rpta = 0;
    }
    else
    {
        // se ignoran los ultimos 4 bytes, consideramos que es demasiada data
        rpta = ((unsigned int)buffer[0]) + (((unsigned int)buffer[1]) << 8 ) + (((unsigned int)buffer[2]) << 16 ) + (((unsigned int)buffer[3]) << 24 );
    }
    return rpta;
}


