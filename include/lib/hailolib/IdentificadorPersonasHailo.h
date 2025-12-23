

#ifndef _IDENTIFICADOR_PERSONAS_HAILO_
#define _IDENTIFICADOR_PERSONAS_HAILO_

#include <stdio.h>

#include <memory>
#include <string>

#include "lib/general/GLinkedList.h"
#include "lib/general/GThread.h"
#include "lib/hailolib/FaceRecHailo.h"
#include "lib/hailolib/IdentificadorPersonasHailo.h"

using namespace std;

/**
 * Definifion de clases para usarlas desde otras que se declaran antes
 */
class GestorThIdentificacionPersonas;

/**
 * Clase que se encarga de buscar coincidenias
 * entre un rostro detectado y una lista de personas
 * ya conocidas
 */
class IdentificadorPerHailo 
{
    public:        
        /**
         * ID de la siguiente persona no desconocida
         */
        long long idSgtePerDesc;
        
        /**
         * Constructor
         */
        IdentificadorPerHailo();

        /**
         * Destructor
         */
        ~IdentificadorPerHailo();

        /**
         * Lista de personas 100% identificadas
         */
        GLinkedList<DescPersonaExterno> lstPerIdentificadas;

        /**
         * Gestor de los threads que hacen la busqueda de personas
         */
        GestorThIdentificacionPersonas *gestorThreads;

        /**
         * Lista de personas no identifiadas
         */
        GLinkedList<DescPersonaExterno> lstPerNoIdent;

        /**
         * Establece el numero de threads que se deben usar para identificar
         * personas conocidas
         */
        void setNumThreadsIdentificacion( int num );

        /**
         * Carga la BD de personas conocidas
         *
         *      path:
         *          Ruta del archivo que se lee
         *
         *      unificarDesc:
         *          Indica si se deben unificar los descriptores con el mismo ID
         * externo
         */
        void cargarpPerConocidas(string path, bool unificarDesc);

        /**
         * Reiniciar el universo de personas conocidas, leídas en BD
         */
        void resetUniverso();

        /**
         * Dado un descriptor facial, se retorna una referencia en caso
         * se encuentre a la descripcion de una persona externa
         *
         *      descriptor:
         *          Descriptor facial que se evalua
         *
         *      tolerancia:
         *          Valor limite para considerar que una persona conocida coincide
         *          con el descriptor, si la distancia ecuclidiana es mayor no es la
         *          misma persona
         *
         *      distEucli :
         *          Puntero en el que se guarda la distancia euclidiana entre el
         *          descriptor y la persona detectada
         * 
         *      usarDistEcucli:
         *          Indica si se debe usar (true) o no distancia euclidieana como criterio de 
         *          equivalencia o similaridad entre dos personas.
         */
        DescPersonaExterno *buscaPersonaCon(SIMD_TYPE *descriptor, float tolerancia, float *distEucli, bool usarDistEucli );

        /**
         * Dado un descriptor facial, se retorna una referencia en caso
         * se encuentre a la descripcion de una persona no identiciada o desconocida
         *
         *      descriptor:
         *          Descriptor facial que se evalua
         *
         *      tolerancia:
         *          Valor limite para considerar que una persona conocida coincide
         *          con el descriptor, si la distancia ecuclidiana es mayor no es la
         * misma persona
         *
         *      distEucli :
         *          Puntero en el que se guarda la distancia euclidiana entre el
         * descriptor y la persona detectada
         */
        DescPersonaExterno *buscaPersonaDesc( SIMD_TYPE *descriptor, float tolareancia, float *distEucli, bool usarDistEuclideana);

        /**
         * Función para calcular la distancia euclidiana usando NEON SIMD
         * entre dos arreglos que contiene descripciones faciales
         */
        float calculaDiferenciaSIMD( SIMD_TYPE *desc1, SIMD_TYPE *desc2);

        /**
         * Calcula el descriptor facial mas cernado de una lista de personas
         * conocidas con una descriptor de alguien recien encontrado
         *
         *       lstUniv:
         *           Vector con descriptores de personas conocidas
         *
         *       desce:
         *           Desctriptor facial que se usa para bucar a la persona conocida
         *
         *       tolerancia:
         *           Valor maximo para conciderar que una persona coincide con el
         * vector desc2 si la distancia euclideana es mayor a tolerancia se ignora
         *
         *       indice :
         *           Puntero a un entero en el que se retorna el indice de la
         * descripcion de persona externa encontrada
         *
         *      distEucli :
         *          Puntero en el que se guarda la distancia euclidiana entre el
         * descriptor y la persona detectada
         *
         *   Retorna un puntero a la definicion de la persona externa dentro de
         * lstUniv si no se encuentra se retorna NULL
         */
        DescPersonaExterno *encuentraPerCercana(
            GLinkedList<DescPersonaExterno> *lstUniv, SIMD_TYPE *desc2, 
            float tolerancia, int *indice, float *distEucli);

        /**
         * Calcula el descriptor facial mas cernado de una lista de personas
         * conocidas con una descriptor de alguien recien encontrado
         *
         *       lstUniv:
         *           Vector con descriptores de personas conocidas
         *
         *       desce:
         *           Desctriptor facial que se usa para bucar a la persona conocida
         *
         *       tolerancia:
         *           Valor maximo para conciderar que una persona coincide con el
         * vector desc2 si la similaridad de coceno es mayor a tolerancia se ignora
         *
         *       indice :
         *           Puntero a un entero en el que se retorna el indice de la
         * descripcion de persona externa encontrada
         *
         *      distEucli :
         *          Puntero en el que se guarda la distancia euclidiana entre el
         * descriptor y la persona detectada
         *
         *   Retorna un puntero a la definicion de la persona externa dentro de
         * lstUniv si no se encuentra se retorna NULL
         */
        DescPersonaExterno *encuentraPerCerCos(
            GLinkedList<DescPersonaExterno> *lstUniv, SIMD_TYPE *desc,
            float tolerancia, int *indice, float *distEucli);

        /**
         * Retorna el promedio de los descriptores
         */
        DescPersonaExterno calculaPromedio(GLinkedList<DescPersonaExterno> *lst);

        /**
         * Elimina las personas no reconocidas que tengan mas de una cantidad
         * de segundos que no son reconocidas
         */
        void eliminaDesAntiguos(long tiempoMaxNoReconocido);
};


/**
 * Clase que gestiona threads para la ubicacion de deteccion de personas
 */
class ThIdentificadorPersonas : public GThread 
{
   public:       
        
        /**
         * Referencia al gestor padre del thread
         */
        GestorThIdentificacionPersonas *gestorPadre;

        /**
         * Constructor
         */
        ThIdentificadorPersonas();

        /**
         * Destructor
         */
        ~ThIdentificadorPersonas();

        /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Busca una persona
         * 
         *      descriptor :
         *          Vector descriptor del rostro de una persona que se busca
         * 
         * 
         *      tolerancie :
         *          Tolerancia en la busqueda.         
         * 
         *      usarDistEuclidiana>
         *          Indica si se debe usar la distancia ecuclidiana (true) o la similaridad de coceno (false)
         *          para hacer la busqueda
         *      
         * 
         */
        void buscarPersona(SIMD_TYPE *descriptor, float tolerancia, bool usarDistEuclidiana );

        /**
         * Agrega una persona externa al thread
         */
        void addPersonaExterna( DescPersonaExterno persona);

        /**
         * Indica si encontro o no una coincidencia
         */
        bool getEncontro();

        /**
         * Retorna una referencia a la coincidencia
         */
        DescPersonaExterno *getCoincidencia();

        /**
         * Retorna la distancia encontrada
         */
        float getDistanciaEncontrada();

         /**
         * Reinicia la lista de personas
         */
        void resetPersonas();

        /**
         * Se invoca para indicar que el Thread debe finalizar su bucle de ejecucion
         */
        void finalizar() override;

   private:

        /**
         * Lista de datos de personas
         */
        GLinkedList<DescPersonaExterno> lstDatos;

        /**
         * Descriptor buscado
         */
        SIMD_TYPE *descBuscado;

        /**
         * Distancia calculada por el thread al hacer la busqueda
         */
        float distanciaCalculada;

        /**
         * Indice encontrado
         */
        int indiceEncontrado;

        /**
         * Indica si se termino la busqueda
         */
        bool busquedaFinalizada;

        /**
         * Flag que indica si hay datos o no que buscar
         */
        bool hayDatos;

        /**
         * Indica si se debe o no usar la distancia euclidiana 
         */
        bool usarDistanciaEuclidiana;

        /**
         * Variable de condicion para controlar la ejeucion de la busqueda o no
         */
        std::condition_variable cv;

        /**
         * Tolenrancia de la busqueda
         */
        float tolerancia;
};


/**
 * Clase que se encarga de adminsitrar la BD de personas conocidas
 * y buscar coincidencias con Threads en paralelo
 */
class GestorThIdentificacionPersonas 
{
   public:

        /**
         * Referencia al identificador que contiene al gestor
         */
        IdentificadorPerHailo *ptrIdentificadorPadre;

        /**
         * Contructor
         */
        GestorThIdentificacionPersonas();

        /**
         * Establec el numreo de thread
         */
        void setNumThreads(int num);

        /**
         * Meotodo llamado por un thread indicando que ha terminado de hacer los calculos
         */
        void notificaFinCalculo();

        /**
         * Agrega una persona a la lista de personas conocidas sobre las que se haria la busqueda
         */
        void addPersona( DescPersonaExterno persona );

        /**
         * Reiniciar base de datos de personas
         */
        void resetPersonas();

        /**
         * Destructor
         */
        ~GestorThIdentificacionPersonas();

        /**
         * Calcula el descriptor facial mas cernado de una lista de personas
         * conocidas con una descriptor de alguien recien encontrado
         *
         *       lstUniv:
         *           Vector con descriptores de personas conocidas
         *
         *       desce:
         *           Desctriptor facial que se usa para bucar a la persona conocida
         *
         *       tolerancia:
         *           Valor maximo para conciderar que una persona coincide con el
         *           vector desc2 si la distancia euclideana es mayor a tolerancia se ignora
         *
         *      distEucli :
         *          Puntero en el que se guarda la distancia euclidiana entre el
         *          descriptor y la persona detectada
         * 
         *      usarDistEuclidiana :
         *          Indica si se debe usar distancia euclidiana (true) o similaridad de coceno (false)
         *
         *
         *   Retorna un puntero a la definicion de la persona externa dentro de
         *      lstUniv si no se encuentra se retorna NULL
         */
        DescPersonaExterno *encuentraPerCercana(SIMD_TYPE *desc2, float tolerancia, float *distEucli, bool usarDistEuclidiana );


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
         * Retorna un lock sobre el mutex
         */
        std::unique_lock<std::mutex> getLock();


    private:

        /**
         * Lista con los threads que hacen la busqueda
         */
        GLinkedList<ThIdentificadorPersonas *> lstThreads;

        /**
         * Numero de threads que han finalizado su trabajo de busqueda
         */
        int numThFinalizado;

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Cantidad de personas que se han agregado
         */
        int numPersonas;

        /**
         * Variable de condicion para controlar la espera a los threads para que terminen de hacer su busqueda
         */
        std::condition_variable cv;        

        /**
         * Indica si los threads 
         */
        bool threadsIniciados;
};

#endif