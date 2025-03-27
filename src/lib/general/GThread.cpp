

#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <time.h>
#include <unistd.h>
#include <csignal>

#include "lib/general/GThread.h"
#include "lib/general/GThreadPool.h"

using namespace std;

void sigpipe_handler_thread(int unused)
{
    cout << "***** ERROR CON PIPE en THREAD " << unused << endl;
}

/**
 * Constructor
 */
GThread::GThread()
{
    fechaUltimoUso = time(NULL);
    threadPool = NULL;
    th = NULL;

    // signal(SIGPIPE, sigpipe_handler_thread);
        
    // th = new thread( &GThread::bucleThread,this);
    thName = "";
}

/**
 * Constructor
 */
GThread::GThread( string nombre )
{
    fechaUltimoUso = time(NULL);
    threadPool = NULL;
    thName = nombre;
    th = NULL;

    // signal(SIGPIPE, sigpipe_handler_thread);
        
    // th = new thread( &GThread::bucleThread,this);
}

/**
 * Genera una pausa de milisegundos
 * milisegundos: tiempo que se detiene
 */
void GThread::sleepMS(int pausa)
{ 
    if (pausa >= 1000)
      sleep(pausa / 1000);

    usleep((pausa % 1000) * 1000);
}

/**
 * Genera una pausa de microsegundos
 * pausa: tiempo que se detiene
 */
void GThread::sleepUS( int pausa )
{     
    usleep(pausa);
}

/**
 * Genera una pausa en segundos
 * pausa: tiempo que se detiene
 */
void GThread::sleepSeg( int pausa )
{     
    sleep(pausa);
}

/**
 * Bucle del thread que se ejecuta en paralelo y llama a la funcion : funcionThread
 */
void GThread::bucleThread()
{
    mtxBloquea();
    finalizado = false;
    // ocupado = false;
    mtxLibera();

    std::signal(SIGPIPE,sigpipe_handler_thread);
    
    cout << "Thread " << id_unico << " " << thName << " Iniciado " << endl;

    while( isFinalizado() == false )
    {
        // espera hasta que el flag ocupado este marcado
        if ( isOcupado() == false )
        {
            sleepMS(1);
            continue;
        }

        cout << "Thread " << id_unico << " ejecuta funcion en paralelo " << endl;
        fechaUltimoUso = time(NULL);
        try
        {
            runThread();
        }
        catch(const std::exception& e)
        {
            cout << "Error en la ejeucion del threat " << id_unico << " : " << e.what() << '\n';            
        }
        fechaUltimoUso = time(NULL);
        // cout << "Thread " << id_unico << " finaliza la ejecucion de la funcion en paralelo " << endl;
        
        mtxBloquea();
        ocupado = false;        
        mtxLibera();

        if ( threadPool != NULL )
        {
            threadPool->liberaReserva(id_unico);
        }
        else
        {
            // se termina la ejecucion pues no pertenece a un threadPool
            break;
        }
    }

    // cout << "Thread " << id_unico << " finalizado " << finalizado << endl;
    mtxBloquea();
    finalizado = true;
    mtxLibera();
}


/**
 * Indica que se desea iniciar la ejecucion en paralelo la funcion : funcionThread
 * Retorna true en caso se haya podido tomar el thread e iniciar la ejecucion
 * de la funcion, false en caso otro procesa haya ejecutado primero la funcion.
 */
void GThread::start()
{    
    mtxBloquea();
    ocupado = true;
    mtxLibera();
    if ( th == NULL )
    {
        th = new thread( &GThread::bucleThread,this);
    }
    else
    {
        cout << "No se crea THREAD OS porque ya existe" << endl;
    }
}

/**
 * Reporta si esta ocupado o no
 */
bool GThread::isOcupado()
{
    bool rpta;

    mtxBloquea();
    rpta = ocupado;
    mtxLibera();

    return rpta;
}

/**
 * Reporta si ha terminado su ejecucion o no
 */
bool GThread::isFinalizado()
{
    bool rpta;

    mtxBloquea();
    rpta = finalizado;
    mtxLibera();

    return rpta;
}

/**
 * Se invoca para indicar que el Thread debe finalizar su bucle de ejecucion
 */
void GThread::finalizar()
{
    mtxBloquea();
    finalizado = true;
    mtxLibera();
}

/**
 * Metodo que se debe invocar cuando se desea iniciar un codigo critico
 * que solo un proceso a la vez debe hacer
 */
void GThread::mtxBloquea()
{
    mtx.lock();
}

/**
 * Metodo que se debe invocar cuando se indica que ya se termino de ejecutar
 * un codigo critico
 */
void GThread::mtxLibera()
{
    mtx.unlock();
}

/**
 * Funcion que se ejecuta en paralelo desde el thread
 */
void GThread::runThread()
{

}


/**
 * Retorna la cantidad de segundos que no se ha usado el thread
 */
double GThread::getTiempoSinUso()
{
    return difftime(time(NULL),fechaUltimoUso);
}