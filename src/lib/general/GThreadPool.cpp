

#include <time.h>
#include <string>
#include <thread>
#include <mutex>
#include <iostream>

#include "lib/general/GObject.h"
#include "lib/general/GVector.h"
#include "lib/general/GThread.h"
#include "lib/general/GThreadPool.h"

/**
 * Constructor
 */
GThreadPool::GThreadPool()
{
    minThreads = 4;
    tiempoNoUsoMax = 30;
    posActual = 0;
    fechaUltimaPurgaTh = time(NULL);
}

/**
 * Destructor
 */
GThreadPool::~GThreadPool()
{
}

/**
 * Establece la cantidad minima de threads
 * 
 * min : cantidad minima de threads que debe tener el thread en todo tiempo
 */
void GThreadPool::setMinThreads( int min)
{   
    mtx.lock();
    minThreads = min;
    mtx.unlock();
}

/**
 * Retorna la cantidad minima de threads
 */
int GThreadPool::getMinThreads()
{
    int rpta;

    mtx.lock();
    rpta = minThreads;
    mtx.unlock();

    return rpta;
}

/**
 * Establece el tiempo maximo que un thread puede estar sin
 * uso para ser finalizado
 * 
 * tiempo: cantidad de segundos
 */
void GThreadPool::setMaxTiempoNoUso( double tiempo )
{
    mtx.lock();
    tiempoNoUsoMax = tiempo;
    mtx.unlock();
}

/**
 * Retorna la cantidad de segundos que un thread puede estar sin
 * uso para ser finalizado
 */
double GThreadPool::getMaxTiempoNoUso()
{
    double rpta;

    mtx.lock();
    rpta = tiempoNoUsoMax;
    mtx.unlock();

    return rpta;
}

/**
 * Agrega un thread al pool
 */
void GThreadPool::addThread( shared_ptr<GThread> th )
{

    cout << "GThreadPool.addThread " << th->id_unico << endl;
    mtx.lock();
    th->threadPool = this;
    lstThreads.add(th);
    mtx.unlock();
}

/**
 * Retorna la cantidad de threads que tiene el pool
 */
int GThreadPool::size()
{
    int rpta;

    mtx.lock();
    rpta = lstThreads.size();
    mtx.unlock();

    return rpta;
}

/**
 * Retorna un thread libre para que pueda ejecutar un proceso en paralelo
 * retonra el thread o NULL en caso no existan threads libres.
 */
shared_ptr<GThread> GThreadPool::getThreadLibre()
{
    int posIni,n;
    shared_ptr<GThread> th,rpta;

    mtx.lock();

    if ( difftime(time(NULL),fechaUltimaPurgaTh) > tiempoNoUsoMax )
    {
        purgarThreadsNoUsados();
    }

    rpta = NULL;
    n = lstThreads.size();

    if ( posActual >= n ) posActual = 0;

    posIni = posActual;

    // cout << "Pos Actual "  << posActual << " se busca hasta el final, Num Threads " << lstThreads.size() << endl;

    // busca los threads a partir de la posicion actual
    while( posActual < n )
    {
        th = dynamic_pointer_cast<GThread> (lstThreads.get(posActual));
        if ( isThreadReservado(th->id_unico) == false )
        {
            lstIdThreadReservados.add(th->id_unico);
            rpta = th;
            posActual++; 
            break;
        }
        else
        {
            // cout << "El thread " << th->id_unico << " esta reservado" << endl;
        }
        posActual++;        
    }

    if ( rpta == NULL )
    {
        // cout << "Busca threads desde el inicio hasta " << posIni << " Num threads " << lstThreads.size() << endl;
        // busca los threads a partir del inicio
        posActual = 0;
        while( posActual < posIni )
        {
            th = dynamic_pointer_cast<GThread>(lstThreads.get(posActual));
            if ( isThreadReservado(th->id_unico) == false )
            {
                lstIdThreadReservados.add(th->id_unico);
                rpta = th;
                break;
            }
            else
            {
                // cout << "El thread " << th->id_unico << " esta reservado" << endl;
            }
            posActual++;        
        }
    }

    if ( posActual >= lstThreads.size() )
    {
        posActual = 0;
    }

    mtx.unlock();

    return rpta;
}

/**
 * Retorna un vector con todos los threads reservados
 */
shared_ptr<GVector> GThreadPool::getThreadsReservados()
{
    shared_ptr<GVector> rpta = make_shared<GVector>();
    shared_ptr<GThread>th;

    int i,n;
    string id;

    n = lstIdThreadReservados.size();
    for(i=0;i<n;i++)
    {
        th = dynamic_pointer_cast<GThread>(lstThreads.get(i));
        if ( isThreadReservado(th->id_unico) == true )
        {
            rpta->add(th);
        }
    }

    return rpta;
}


/**
 * Valida si el ID de un thread esta reservado
 */
bool GThreadPool::isThreadReservado( long long id )
{
    int i,n;

    n = lstIdThreadReservados.size();
   
    // cout << "GhreadPool isThreadReservado Num Th reservados " << n << endl;
    for(i=0;i<n;i++)
    {
        if ( lstIdThreadReservados.getLongLong(i) == id ) return true;
    }
    return false;
}

/**
 * Libera la reserva del thread, porque el thread ya termino de ejecutarse.
 * Este metodo es llamado cuando termina la funcionParalela del thread
 */
void GThreadPool::liberaReserva( long long idThread )
{
    int i,n;

    mtx.lock();

    n = lstIdThreadReservados.size();
    // cout << "GThreadPool.liberaReserva Num Th reservados " << n << endl;
    for(i=0;i<n;i++)
    {
        if ( lstIdThreadReservados.getLongLong(i) == idThread )
        {
            // cout << "GThreadPool se libera la reserva del thread " << idThread << endl;
            lstIdThreadReservados.remove(i);
            // cout << "GThreadPool num thrads reservados " << lstIdThreadReservados.size() << endl;
            break;
        }
    }

    mtx.unlock();
}

/**
 * Elimina del pool los threads no usados
 */
void GThreadPool::purgarThreadsNoUsados()
{
    int i;
    shared_ptr<GThread> th;

    i = lstThreads.size()-1;
    if ( i <= minThreads ) return;

    while( i >= 0 )
    {
        th = dynamic_pointer_cast<GThread> (lstThreads.get(i));
        if ( isThreadReservado(th->id_unico) == false )
        {
            if ( th->getTiempoSinUso() > tiempoNoUsoMax )
            {
                cout << "GThreadPool descartando thread " << th->id_unico << " por timeout" << endl;
                th->finalizar();
                lstThreads.remove(i);
            }            
        }
        i--;
        if ( lstThreads.size() == minThreads ) return;
    }
}

/**
 * Finaliza el pool, finaliza todos los threads.
 * Metodo generalmente llamado por el destructor.
 */
void GThreadPool::clear()
{

}