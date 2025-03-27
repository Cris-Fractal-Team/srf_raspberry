
#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <sstream>

#include "lib/web/GSocket.h"
#include "lib/general/GThread.h"
#include "lib/web/GHttpServer.h"

using namespace std;

/**
 * Constructor
 */
GHttpServerThread::GHttpServerThread( GHttpServer *servidorPdr )
{
    servidor = servidorPdr;
}

/**
 * Indica que se debe procesar el cliente en paralelo
 */
void GHttpServerThread::procesarCliente( GSocket cliente )
{
    sockCliente = cliente;
    start();
}

/**
 * Retorna referencia al socket
 */
GSocket GHttpServerThread::getSocket()
{
    return sockCliente;
}

/**
 * Funcion que ejecuta el proceso en paralelo
 */
void GHttpServerThread::runThread()
{
    try
    {
        servidor->ubicaComando(sockCliente);
    }
    catch( const std::runtime_error &er )
    {
        cout << "Error procesando request :" << er.what() << endl; 
        sockCliente.cerrar();
    }
    
}