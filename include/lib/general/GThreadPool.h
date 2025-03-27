
#ifndef GTHREADPOOL_H
#define GTHREADPOOL_H

#include <time.h>
#include <string>
#include <thread>
#include <mutex>

#include "GObject.h"
#include "GVector.h"
#include "GThread.h"

class GThreadPool
{
    public:

        /**
         * Constructor
         */
        GThreadPool();

        /**
         * Destructor
         */
        ~GThreadPool();

        /**
         * Establece la cantidad minima de threads
         * 
         * min : cantidad minima de threads que debe tener el thread en todo tiempo
         */
        void setMinThreads( int min);

        /**
         * Retorna la cantidad minima de threads
         */
        int getMinThreads();

        /**
         * Establece el tiempo maximo que un thread puede estar sin
         * uso para ser finalizado
         * 
         * tiempo: cantidad de segundos
         */
        void setMaxTiempoNoUso( double tiempo );

        /**
         * Retorna la cantidad de segundos que un thread puede estar sin
         * uso para ser finalizado
         */
        double getMaxTiempoNoUso();

        /**
         * Agrega un thread al pool
         */
        void addThread( shared_ptr<GThread> th );

        /**
         * Finaliza el pool, finaliza todos los threads.
         * Metodo generalmente llamado por el destructor.
         */
        void clear();

        /**
         * Retorna la cantidad de threads que tiene el pool
         */
        int size();

        /**
         * Retorna un thread libre para que pueda ejecutar un proceso en paralelo
         * retonra el thread o NULL en caso no existan threads libres.
         * Es responsabilidad del invocador de liberar el thread.
         */
        shared_ptr<GThread> getThreadLibre();

        /**
         * Libera la reserva del thread, porque el thread ya termino de ejecutarse.
         * Este metodo es llamado cuando termina la funcionParalela del thread
         */
        void liberaReserva( long long idThread );

        /**
         * Retorna un vector con todos los threads reservados
         */
        shared_ptr<GVector> getThreadsReservados();

    private:

        /**
         * Lista de threads
         */
        GVector lstThreads;

        /**
         * Lista de IDs de los threads que estan reservados
         */
        GVector lstIdThreadReservados;

        /**
         * Cantidad minima de threads que debe tener
         */
        int minThreads;

        /**
         * Puntero al siguiente thread que se debe retornar para ser usado
         */
        int posActual;

        /**
         * Tiempo maximo de no uso de un thread para finalizarlo
         */
        double tiempoNoUsoMax;

        /**
         * Fecha de la ultima purga de threads no usados
         */
        time_t fechaUltimaPurgaTh;

         /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Valida si el ID de un thread esta reservado
         */
        bool isThreadReservado( long long id );

        /**
         * Elimina del pool los threads no usados
         */
        void purgarThreadsNoUsados();

};


#endif
