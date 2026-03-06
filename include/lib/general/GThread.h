
#ifndef GTHREAD_H
#define GTHREAD_H

#include <time.h>
#include <string>
#include <thread>
#include <mutex>

#include "GObject.h"


class GThreadPool;


/**
 * Representa un Thread que puede ser usado en un Pool de Threads
 */
class GThread : public GObject
{
    public:

        /**
         * Numero de nucleo asociado al thread
         * -1 indica no har preferencia
         */
        int nucleoAsociado;

        /**
         * Indica si el thread es un demonio
         */
        bool esDemonio;

        /**
         * Puntero al threadPool al que pertenece
         */
        GThreadPool *threadPool;

        /**
         * Constructor
         */
        GThread(); 

        /**
         * Destructor
         */
        ~GThread(); 

         /**
         * Constructor
         */
        GThread( string nombre ); 

        /**
         * Genera una pausa de milisegundos
         * pausa: tiempo que se detiene
         */
        static void sleepMS( int pausa );

        /**
         * Genera una pausa de microsegundos
         * pausa: tiempo que se detiene
         */
        static void sleepUS( int pausa );

        /**
         * Genera una pausa en segundos
         * pausa: tiempo que se detiene
         */
        static void sleepSeg( int pausa );

        /**
         * Indica que se desea iniciar la ejecucion en paralelo la funcion : runThread         
         */
        void start();

        /**
         * Reporta si esta ocupado o no
         */
        bool isOcupado();

        /**
         * Reporta si ha terminado su ejecucion o no
         * 
         *      bloquear:
         *          Indica si se debe o no bloquear el mutex para leer el valor
         */
        bool isFinalizado( bool bloquear = true );

        /**
         * Se invoca para indicar que el Thread debe finalizar su bucle de ejecucion
         */
        virtual void finalizar();

        /**
         * Metodo que se debe invocar cuando se desea iniciar un codigo critico
         * que solo un proceso a la vez debe hacer
         */
        void mtxBloquea();

        /**
         * Metodo que se debe invocar cuando se indica que ya se termino de ejecutar
         * un codigo critico
         */
        void mtxLibera();

        /**
         * Bucle del thread que se ejecuta en paralelo y llama a la funcion : funcionThread
         */
        void bucleThread();

        /**
         * Funcion que se ejecuta en paralelo desde el thread
         */
        virtual void runThread();

        /**
         * Retorna la cantidad de segundos que no se ha usado el thread
         */
        double getTiempoSinUso();

        /**
         * Nombre del thread
         */
        string thName;

        /**
         * Retorna un lock sobre el mutex
         */
        std::unique_lock<std::mutex> getLock();
            
    
        /**
         * Retorna el mutex
         */
        std::mutex& getMutexRef();


        void finalizarYEsperar();

    private:

        /**
         * Indica si ya se creo el Thread a nivel de sistema operativo
         */
        bool thCreado;

        /**
         * Indica si el thread esta ocupado
         */
        bool ocupado;

        /**
         * Indica que el thread ha finalizado su bucle en 2do plano
         */ 
        bool finalizado;
    
        /**
         * Thread en el que se ejecuta
         */
        std::thread *th;

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        

        /**
         * Fecha de la ultima vez que se uso el thread
         */
        time_t fechaUltimoUso;
                

        /**
         * Asocia el thread a que se ejecute en un nucleo en particular
         */
        void asociaThreadNucleo();
};


#endif


