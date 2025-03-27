
#ifndef GVECTOR_H
#define GVECTOR_H

#include <stdio.h>
#include <vector>
#include <memory>
#include <iostream>

#include "GObject.h"

using namespace std;


/**
 * Declaraciones para poder usar los tipos antes de declararlos completamente
 */
class GNamedVector;

/**
 * Clase base que representa una lista de punteros GPointer
 */
class GList
{
    public :

        /**
         * Retorna la cantidad de elementos
         */
        virtual long size() = 0;

        /**
         * Agrega un elemento a la lista
         * @param elemento elemento que se agrega
         */
        virtual void add( shared_ptr<GObject> elemento ) = 0;

        /**
         * Retorna un elemento de una posicion derterminada
         * @param indice el indice del elemento
         * @return elemeneto encontrado o NULL en caso no exista el indice
         */
        virtual shared_ptr<GObject> get( long indice ) = 0;

        /**
         * Limpia el arreglo
         */
        virtual void clear() = 0;

        /**
         * Elimina un elemento de la lista
         * @param indice posicion del elemento
         */
        virtual void remove( long indice ) = 0;

        /**
         * Establece el puntero almacenado en una posicion
         * 
         * @param indice posicion del elemento
         * 
         * @param elemento puntero que se guarda
         */
        virtual void set( long indice, shared_ptr<GObject> elemento ) = 0;

        /**
         * Ordena una lista c on la tecnica quicksort
         * Se le debe pasar la lista y una funcion que acepte dos punteros 
         * a funcion debera retornar un valor < 0 si el primer parametro es menor al segundo,
         * > 0 si el primer parametro es mayor que el segundo, 0 si son iguales
         * 
         */
        void sort( int (* comparador)( shared_ptr<GObject>, shared_ptr<GObject>));

    private:

        /**
         * Ordena una lista
         * 
        */
        void sortQuickSort( int (* comparador)( shared_ptr<GObject>, shared_ptr<GObject>), long low, long high);

        /**
         * Calcula el indica de la particion del algoritmo quicksort
         */
        long sortPartition( long low, long hig, int (* comparador)( shared_ptr<GObject>, shared_ptr<GObject>) );
};


/**
 * Representa un vector de puntor a objetos
 */
class GVector : public GObject, public GList
{
    public:

        /**
         * Lista de datos del gector
         */
        shared_ptr<vector<shared_ptr<GObject>>> lstDatos;

        /**
         * Constructor
         */
        GVector();

        /**
         * Constructor
         * 
         * capacidadIni : cantidad de elementos que se deben poder almacenar
         */
        GVector( long capacidadIni );

        /**
         * Asegura el tamao del buffer interno para almacenar
         * una cantgidad maxima de objetos.
         * 
         * capacidad : cantidad de objetos que se dese asegurar poder almacenar.
         */
        void aseguraCapacidad( long capacidad );
        
        virtual ~GVector();

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el elemento de una posicion dada.
         * Este elemento es el almacenado por el Gpointer
         * es equivalente a llamara a vector.get(indice).get()
         */
        void * elementAt( long indice );


        /**
         * Retorna un elemento de una posicion derterminada
         * @param indice el indice del elemento
         * @return elemeneto encontrado o NULL en caso no exista el indice
         */
        shared_ptr<GObject> get( long indice ) override;

        
        /**
         * Retorna el ultimo elemento
         */
        shared_ptr<GObject> getLast();

        /**
         * Agrega un entero
         */
        void add( int valor );

        /**
         * Agrega un long
         */
        void add( long valor );

        /**
         * Agrega un long long
         **/
        void add( long long valor );

        /**
         * Agrega un long long
         **/
        void add( double valor );

        /**
         * Agrega un long long
         **/
        void add( string valor );

        /**
         * Retorna un entero
         */
        int getInt( long index ); 

        /**
         * Retorna un long
         */
        long getLong( long index );

        /**
         * Retorna un long
         */
        long long getLongLong( long index );

        /**
         * Retorna un double
         */
        double getDouble( long index );

        /**
         * Retorna un string
         */
        string getString( long index );

        /**
         * Elimina un elemento de la lista
         * @param indice posicion del elemento
         */
        void remove( long indice )  override;

         /**
         * Elimina un elemento de la lista
         * @param ptr elemento que se elimina
         */
        void remove( shared_ptr<GObject> ptr  );

        /**
         * Retorna el indice de un elemento
         * @param ptr elemento que se busca, retorna -1 si no existe
         */
        long indexOf( shared_ptr<GObject> ptr );

        /**
         * Limpia el arreglo
         */
        void clear() override;

        /**
         * Retorna la cantidad de elementos del vector
         */
        long size() override;

        /**
         * Agrega un elemento a la lista
         * @param elemento elemento que se agrega
         */
        void add( shared_ptr<GObject> elemento )  override;


        /**
         * Establece el puntero almacenado en una posicion
         * 
         * @param indice posicion del elemento
         * 
         * @param elemento puntero que se guarda
         */
        void set( long indice, shared_ptr<GObject> elemento ) override;


    protected:

    private:

        /**
         * Cantidad de elementos que tiene la lista
         */
        long capacidadLista;

        /**
         * Posicion del siguente elemento
         */
        long posActual;

        /**
         * Inicializa con un tamano el vector
         */
        void initSize( long len );

};

/**
 * Representa un vector que tiene un atributo nombre
 * Es util para poder identificar un vector como una lista de valores
 * con nombre o un parametro
 */
class GNamedVector : public GVector
{
    public:
        /**
         * Nombre del vector
         */
        string name;
};

#endif // GVECTOR_H
