

#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>    
#include <time.h> 

#include <openssl/sha.h>

#include "lib/web/GSocket.h"
#include "lib/general/GVector.h"
#include "lib/general/GObject.h"
#include "lib/general/GHashMap.h"
#include "lib/web/GHttpServer.h"
#include "lib/web/GWebSocketCommand.h"
#include "lib/utils/base64.h"

using namespace std;


int GWebSocketCommand::minNumThread = 2;

int GWebSocketCommand::maxNumThread = 40;

bool GWebSocketCommand::poolConInicializado = false;

GThreadPool GWebSocketCommand::thPool;

std::mutex GWebSocketCommand::mtxPool;

/**
 * Constructor simple
 */
GWebSocketCommand::GWebSocketCommand()
{
    idSgteSession = 0;
    initThreadPool();
}

/**
 * Procesa una peticion
 */
void GWebSocketCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response )
{
    string connection = request->getHeader("Connection");
    string upgrade = request->getHeader("Upgrade");
    string socketVersion = request->getHeader("Sec-WebSocket-Version");
    string wsKey = request->getHeader("Sec-WebSocket-Key");
    string protocolo = request->getHeader("Protocols");

    // cout << "Procesando el request inicial de WebSocket" << endl;
    // cout << "Connection :" << connection << endl;
    // cout << "Upgrade :" << upgrade << endl;
    // cout << "SocketVersion :" << socketVersion << endl;
    // cout << "WsKey :" << wsKey << endl;
    // cout << "Protocols :" << protocolo << endl;

    if ((( connection.length() == 0 ) || ( upgrade.length() == 0 )) || (( socketVersion.length() == 0 ) || ( wsKey.length() == 0 )))
    {
        // cout << "Error 406" << endl;

        response->setHttpCode(406);
        response->setHtmlResponse("<html><body>Datos de cabecera para WebSocket Incorrectos</body></html>");
        return;
    }

    shared_ptr<GWebSocketThread> th = getThread();

    if ( th == NULL )
    {
        // cout << "Error 429" << endl;

        response->setHttpCode(429);
        response->setHtmlResponse("<html><body>Muchos clientes WebSocket conectados</body></html>");
        return;
    }
    
    string cadenaClave(wsKey);
    unsigned char hash[21]; 
    unsigned char hashFinal[128]; 
    size_t len;
    cadenaClave.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    // cout << "Antes de generar clave hash:" << cadenaClave << endl;

    SHA1((const unsigned char*)cadenaClave.c_str(),cadenaClave.length(),hash);

    hash[20] = 0;
    string clave((char *)hash);

    base64_encode_intoBuffer((char *)clave.c_str(),clave.length(),(char *)hashFinal,&len);
    hashFinal[len] = 0;

    // cout << "Longitud del hash generado:" << len << endl;
    // cout << "Hash geneardo:" << hashFinal << endl;

    string claveFinal((char *)hashFinal);

    response->addHeader("Upgrade","websocket");
    response->addHeader("Connection","Upgrade");
    response->addHeader("Sec-WebSocket-Accept",claveFinal);
    response->addHeader("Sec-WebSocket-Version","13");
    // response->addHeader("Sec-WebSocket-Version","13");

    string rpta("HTTP/1.1 101 Switching Protocols\r\n");

    response->setCustomResponse(rpta,NULL,0);

    // cout << "Respuesta enviada, debe crear una nueva sesion" << endl;

    shared_ptr<GWebSocketSession> session = creaNuevaSession(response->getSocket());

    // cout << "Nueva sesion creada" << endl;
    th->procesarCliente(session); 
}

/**
 * Inicializa el pool de threads
 */
void GWebSocketCommand::initThreadPool()
{
    int minTh = minNumThread;
    shared_ptr<GWebSocketThread> th;

    if ( poolConInicializado == true ) return;

    GWebSocketCommand::thPool.setMinThreads(minNumThread);

    while( minTh > 0 )
    {
        th = make_shared<GWebSocketThread>();
        GWebSocketCommand::thPool.addThread(th);
        minTh--;
    }
    

    poolConInicializado = true;
}

/**
 * Retorna un thread para procesar un cliente
 */
shared_ptr<GWebSocketThread> GWebSocketCommand::getThread()
{    
    shared_ptr<GWebSocketThread> th;

    GWebSocketCommand::mtxPool.lock();

    try
    {
        th = dynamic_pointer_cast<GWebSocketThread> (GWebSocketCommand::thPool.getThreadLibre());
        if ( th != NULL ) 
        {
            GWebSocketCommand::mtxPool.unlock();
            return th;
        }
        
        if ( GWebSocketCommand::thPool.size() < maxNumThread )
        {
            // cout << "GHttpServer Se crea un nuevo thread para el pool" << endl;
            th = make_shared<GWebSocketThread>();
            GWebSocketCommand::thPool.addThread(th);
            th = dynamic_pointer_cast<GWebSocketThread>(GWebSocketCommand::thPool.getThreadLibre());

            GWebSocketCommand::mtxPool.unlock();
            return th;
        }

        // cout << "GWebSocketCommand Se tiene que esperar por un thread libre " << endl;
        while( true )
        {
            GThread::sleepMS(2);
            th = dynamic_pointer_cast<GWebSocketThread>(GWebSocketCommand::thPool.getThreadLibre());
            if ( th != NULL )
            {   
                // cout << "GWebSocketCommand Thread liberado " << endl;
                GWebSocketCommand::mtxPool.unlock();
                return th;
            }
        }

    }    
    catch( ... )
    {
        GWebSocketCommand::mtxPool.unlock();
        return NULL;
    }
    
}

/** 
 * Crea una nueva sesion
*/
shared_ptr<GWebSocketSession> GWebSocketCommand::creaNuevaSession( GSocket socket )
{
    shared_ptr<GWebSocketSession> session = make_shared<GWebSocketSession>();

    mtxInterno.lock();

    idSgteSession = idSgteSession+1;

    session->idSession = idSgteSession;
    session->socket = socket;
    session->comandoPadre = shared_from_this();

    lstSesiones.add(session);

    mtxInterno.unlock();

    return session;
}

/**
 * Invocado cada vez que un cliente se conecta
 */
void GWebSocketCommand::onNewClient( shared_ptr<GWebSocketSession> session )
{
    
}

/**
 * Invocado cada vez que un cliente se desconecta
 */
void GWebSocketCommand::onExitClient( shared_ptr<GWebSocketSession> session )
{

}

/**
 * Invocado cada vez que llega un nuevo mensaje de texto
 */
void GWebSocketCommand::onTextMessage( shared_ptr<GWebSocketSession> session, char *buffer, int len )
{

}

/**
 * Invocado cada vez que llega un nuevo mensaje binario
 */
void GWebSocketCommand::onBinMessage( shared_ptr<GWebSocketSession> session, char *buffer, int len )
{

}




/**
 * Retorna la cantidad de sesiones actuales
 */
int GWebSocketCommand::getLstSessionSize()
{
    int rpta;

    mtxInterno.lock();
    rpta = lstSesiones.size();
    mtxInterno.unlock();

    return rpta;
}


/**
 * Elimina una sesion dado su ID
 */
void GWebSocketCommand::removeSession( long long idSession )
{
    int i,n;
    GWebSocketSession *s;
    GPointer ptr;

    mtxInterno.lock();

    n = lstSesiones.size();
    for(i=0;i<n;i++)
    {
        ptr = lstSesiones.get(i);
        s = (GWebSocketSession *)ptr.get();
        if ( s->idSession == idSession )
        {
            cout << "Eliminando session WebSocket " << idSession << endl;
            lstSesiones.remove(i);
            break;
        }
    }

    mtxInterno.unlock();
}

/**
 * Retorna una session dada su ubicacion
 * 
 * returna NULL en caso ya no exista la sesion o la referencia a la sesion
 */
shared_ptr<GWebSocketSession> GWebSocketCommand::getSessionAt( int index )
{
    shared_ptr<GWebSocketSession> rpta;
    if ( index < 0 ) return NULL; 

    mtxInterno.lock();
    if ( index > lstSesiones.size() )
    {
        rpta = NULL;
    }
    else
    {
        rpta = dynamic_pointer_cast<GWebSocketSession>(lstSesiones.get(index));
    }
    mtxInterno.unlock();

    return rpta;
}


/**
 * Envia un mensaje de texto a un cliente
 */
void GWebSocketSession::sendTextMessage( char *message, int len )
{
    unsigned char cabecera[8];
    
    cabecera[0] = 129;

    if ( socket.estaConectado() != 1 )
    {
        cout << "NO se envia mensaje porque el socket no esta conectado " << endl;
        return;
    }

    mtxInterno.lock();
    try
    {
        if ( len < 126 )
        {
            cabecera[1] = len;
            socket.write((char *)cabecera,2);        
        }
        else
        if ( len < 65536 )
        {
            cabecera[1] = 126;
            socket.write((char *)cabecera,2);

            cabecera[1] = (len & 255);
            cabecera[0] = (len >> 8);

            socket.write((char *)cabecera,2);        
        }
        else
        {
            cabecera[1] = 127;
            socket.write((char *)cabecera,2);
            
            cabecera[7] = (len & 255);
            cabecera[6] = ((len >> 8)&255);
            cabecera[5] = ((len >> 16)&255);
            cabecera[4] = ((len >> 24)&255);
            cabecera[3] = 0;
            cabecera[2] = 0;
            cabecera[1] = 0;
            cabecera[0] = 0;
            socket.write((char *)cabecera,8);        
        }

        socket.write(message,len);
    }
    catch( int id )
    {
        cout << "Error al enviar datos via WebSocket con id unico " << id_unico << " ID Sesion: " << idSession << endl;
    }

    mtxInterno.unlock();
}