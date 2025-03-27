
#include <iostream>
#include <cstdlib>

#include "lib/web/GSocket.h"
#include "lib/general/GObject.h"
#include "lib/web/GHttpServer.h"
#include "lib/general/GVector.h"


using namespace std;

/**
 * Constructor
 */
GHttpServer::GHttpServer()
{
    idSgteSession = 0;
    cout << "Constructor servidor HTTP " << endl;
    minNumThread = 2;
    maxNumThread = 400;

    cout << "GHTTP Server ID Lista comandos : " << lstComandos.id_unico << endl;

    initThreadPool();
}

/**
 * Inicia el servidor
 * 
 * ip : ip que monitorea
 * 
 * puerto : puerto IP asociado
 */
void GHttpServer::iniciar( string ip, int puerto )
{   
    GSocket cliente;
    shared_ptr<GHttpServerThread> th;
    
    socketServer.iniciaServidor(ip,puerto);

    do
    {
        // cout << "Esperando clientes" << endl;
        cliente = socketServer.aceptarCliente();
        if ( cliente.estaConectado() == 1 )
        {
            // cout << "Cliente detectado" << endl;
            // ubicaComando(cliente);            
            try
            {
                
                th = getThread();
                if ( th == NULL )
                {
                    cout << "No se pudo obtener un thread se ignora al cliente " << endl;
                    cliente.cerrar();
                }
                else
                {
                    // cout << "El Thread " << th->id_unico << " procesara el request" << endl;
                    th->procesarCliente(cliente);
                }
            }
            catch(const std::runtime_error& e)
            {
                std::cout << "Error al procesar el request " << e.what() << '\n';
            }                        
        }
        else
        {
            cout << "Error al esperar un cliente";
        }
        if ( socketServer.estaConectado() == 0 ) break;
    }
    while( socketServer.estaConectado() == 1 );
    cout << "Servidor HTTP finalizado" << endl;
}

/**
 * Dado el socket de un cliente lo analiza e identifica
 * el comando que debe procesar la peticion
 */
void GHttpServer::ubicaComando( GSocket cliente )
{
    shared_ptr<HttpHeaderReader> reader = make_shared<HttpHeaderReader>();

    // cout << "Buscando el comando que procesa el request" << endl;

    reader->leeCabeceraHttp(cliente);
    reader->leeDatosBasicosPeticion();

    // cout << "Peticion HTTP : " << reader->metodoReq << " URL : " << reader->urlReq << " QueryString : " << reader->queryString << endl;

    int n;
    shared_ptr<GHttpServerCommand> comando;
    bool rpta,encontro;
    string msgRpta;
    int codError;

    n= lstComandos.size();
    // cout << "Hay " << n << " Comandos" << endl;
    encontro = false;
    for(int i=0;i<n;i++)
    {
        comando = dynamic_pointer_cast<GHttpServerCommand>(lstComandos.get(i));
        if ( comando != NULL )
        {
            if ( comando->esProcesador(reader->urlReq,reader->metodoReq) == true )
            {   
                // cout << "El comando debe ser ejecutado" << endl;                
                try
                {
                    rpta = procesarPeticion(reader,comando,cliente);                    
                    // cout << "Fin de ejecucion del comando" << endl;
                    codError = 0;
                }
                catch( ... )
                {
                    cout << "Ocurrio una excepcion" << endl;
                    rpta = false;
                    codError = 500;
                    msgRpta.append("Error interno del servidor al procesar la peticion");
                }
                
                if ( rpta == true )
                {
                    return;
                }
                else
                if ( codError == 0 )
                {
                    cout << "El comando no procesa la peticion" << endl;
                    codError = 500;
                    msgRpta.append("Error interno del servidor, la peticion no fue procesada");
                }
                encontro = true;                
                
                break;
            }            
        }        
    }

    if ( encontro == false )
    {
        cout << "No se encontro un comando asociado al URL" << endl;

        codError = 404;
        msgRpta.append("URL o pagina WEB invalida");
    }
        
    string rptaHtml("<html><body>");
    
    rptaHtml.append(msgRpta);
    rptaHtml.append("</body></html>");

    string rptaHttp;

    rptaHttp.append("HTTP/1.1 ");
    rptaHttp.append(to_string(codError));
    rptaHttp.append(" OK\r\n");
    rptaHttp.append("Content-Type: text/html\r\n");
    rptaHttp.append("Connection: Closed\r\n");
    rptaHttp.append("Content-Length: ");
    rptaHttp.append(to_string(rptaHtml.length()));
    rptaHttp.append("\r\n");
    rptaHttp.append("\r\n");

    rptaHttp.append(rptaHtml);

    // cout << "Respuesta que se envia: " << rptaHttp << endl;

    cliente.write((char *)rptaHttp.c_str(),rptaHttp.length());
    cliente.cerrar();

}


/**
 * Detiene el servidor
 */
void GHttpServer::finalizar()
{
    shared_ptr<GVector>lstRequestPend;
    int i,n;
    shared_ptr<GHttpServerThread> th;

    lstRequestPend = thPool.getThreadsReservados();
    n = lstRequestPend->size();

    for(i=0;i<n;i++)
    {
        th = dynamic_pointer_cast<GHttpServerThread>(lstRequestPend->get(i));
        th->getSocket().cerrar();
    }

    socketServer.cerrar();
}


/**
 * Valida si el servidor esta o no en ejecucion
 */
bool GHttpServer::enEjecucion()
{
    return socketServer.estaConectado();
}


/**
 * Agrega un comando al servidor
 */
void GHttpServer::addCommand( shared_ptr<GHttpServerCommand> comando )
{    
    comando->servidor = this;

    // cout << "Add Comando, al arreglo con ID " << lstComandos.id_unico << endl;
    lstComandos.add(comando);
}


 /**
 * Crea una nueva sesion WEB
 */
shared_ptr<GHttpServerSession> GHttpServer::creaNuevaSesion( shared_ptr<GHttpResponse> response )
{
    shared_ptr<GHttpServerSession> session = make_shared<GHttpServerSession>();

    string id;

    id.append(to_string(rand()%100000));
    id.append("_");
    id.append(to_string(idSgteSession++));
    id.append("_");
    id.append(to_string(rand()%100000));

    session->idSession = id;

    lstSesiones.add(session);

    response->addCookie(GHttpServerCommand::idCookieSession, id, "/", -1);

    return session;
}

/**
 * Dado el request y el ID del cookie en el que se guardar el ID de una sesion
 * retorna la sesion con el ID.
 * 
 * Retorna la sesion o NULL en caso no exista
 */
shared_ptr<GHttpServerSession> GHttpServer::getSession( shared_ptr<GHttpRequest> request )
{
    string id = request->getCookie(GHttpServerCommand::idCookieSession);

    // cout << "ID de sesion = " << id << endl;
    if ( id.length() == 0 )
    {
        // cout << "No existe el ID de Session" << endl;
        return nullptr;
    }

    return getSession(id);
}


/**
 * Dado el ID de una sesion retorna sus datos
 * 
 * id : identificador de la sesion
 * 
 * Retorna la sesion o NULL en caso no exista
 */
shared_ptr<GHttpServerSession> GHttpServer::getSession( string id)
{
    long i,n;
    shared_ptr<GHttpServerSession> sesion;

    n = lstSesiones.size();
    for(i=0;i<n;i++)
    {
        sesion = dynamic_pointer_cast<GHttpServerSession>(lstSesiones.get(i));
        if ( sesion->idSession.compare(id) == 0 )
        {
            return sesion;
        }
    }

    cout << "El ID de sesion " << id << " no existe" << endl;
    
    return nullptr;
}

/**
 * Elimina una sesion
 */
void GHttpServer::delSession( string id )
{
    long i,n;
    GHttpServerSession *sesion;

    n = lstSesiones.size();
    for(i=0;i<n;i++)
    {
        sesion = (GHttpServerSession *)lstSesiones.get(i).get();
        if ( sesion->idSession.compare(id) == 0 )
        {
            lstSesiones.remove(i);
            return;
        }
    }
}


/**
 * Procesa una peticion con un comando
 * 
 * Retorna false si no se proceso, true en caso si se haya procesado
 */
bool GHttpServer::procesarPeticion( shared_ptr<HttpHeaderReader> reader, shared_ptr<GHttpServerCommand> comando, GSocket socket )
{
    shared_ptr<GHttpRequest> request = make_shared<GHttpRequest>(reader,socket);
    shared_ptr<GHttpResponse> response = make_shared<GHttpResponse>(socket);
    

    response->addHeader("Server","GHttpServer V.1.0");

    if ( comando->sesionOblogatoria == true )
    {
        // cout << "La sesion es obligatoria" << endl;
        shared_ptr<GHttpServerSession> sesion = getSession(request);
        if ( sesion == nullptr )
        {
            response->setTextResponse("Acceso No Autorizado","text/plain");
            return true;
        }
    }
    
    // cout << "Iniciando proceso de peticion con comando identificado " << endl;
    
    // cout << "Antes de procesa request , ID: " << comando->id_unico << endl;
    comando->procesaRequest(request,response,false);
    
    // cout << "Fin del proceso request ejecutado" << endl;
    if ( response->getAccion() == HTTP_RESPONSE_ACCION_NONE )
    {
        // cout << "El request no fue procesado" << endl;
        return false;
    }
    // cout << "El request fue procesado" << endl;

    return true;
}

/**
 * Busca el comando que tiene un URL dado y se llama al metodo procesaRequest
 * Este metodo se emplea para redireccionar la logica entre Commands
 */
void GHttpServer::redirecciona( string url, shared_ptr<GHttpRequest>request, shared_ptr<GHttpResponse> response )
{
    int n;
    shared_ptr<GHttpServerCommand> comando;    

    n= lstComandos.size();    
    for(int i=0;i<n;i++)
    {
        comando = dynamic_pointer_cast<GHttpServerCommand>(lstComandos.get(i));
        if ( comando != NULL )
        {
            if ( comando->esProcesador(url,request->getMetodo()) == true )
            {   
                comando->procesaRequest(request, response, true);
                return;
            }
        }
    }
}

/**
 * Retorna un thread para procesar un request
 */
shared_ptr<GHttpServerThread> GHttpServer::getThread()
{    
    shared_ptr<GHttpServerThread> th;

    mtx.lock();

    try
    {
        th = dynamic_pointer_cast<GHttpServerThread>(thPool.getThreadLibre());
        if ( th != NULL ) 
        {
            mtx.unlock();
            return th;
        }
        
        if ( thPool.size() < maxNumThread )
        {
            cout << "GHttpServer Se crea un nuevo thread para el pool" << endl;
            th = make_shared<GHttpServerThread>(this);
            thPool.addThread(th);
            th = dynamic_pointer_cast<GHttpServerThread> (thPool.getThreadLibre());

            mtx.unlock();
            return th;
        }

        cout << "GHttpServer Se tiene que esperar por un thread libre " << endl;
        while( true )
        {
            GThread::sleepMS(2);
            th = dynamic_pointer_cast<GHttpServerThread> (thPool.getThreadLibre());
            if ( th != NULL )
            {   
                cout << "GHttpServer Thread liberado " << endl;
                mtx.unlock();
                return th;
            }
        }

    }    
    catch( ... )
    {
        mtx.unlock();
        return NULL;
    }
    
}

/**
 * Inicializa el pool de threads
 */
void GHttpServer::initThreadPool()
{
    int minTh = minNumThread;
    shared_ptr<GHttpServerThread> th;

    thPool.setMinThreads(minNumThread);

    while( minTh > 0 )
    {
        th = make_shared<GHttpServerThread>(this);
        thPool.addThread(th);
        minTh--;
    }
    

}