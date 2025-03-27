#include "lib/general/GVector.h"
#include <iostream>
#include <stdio.h>

using namespace std;

/**
 * Tamano inicial de una lista
 */
#define TAM_INICIAL_LISTA 64

/**
 * Incremento que se da a una lista cuando se queda sin espacio
 */
#define INC_LISTA_LLENA 32

 /**
 * Ordena una lista
 * 
*/
void GList::sortQuickSort( int (* comparador)( shared_ptr<GObject>, shared_ptr<GObject>), long low, long high)
{
    if ( low < high )
    {
        long pivotIndex = sortPartition(low,high, comparador);
        sortQuickSort( comparador, low, pivotIndex-1);
        sortQuickSort( comparador, pivotIndex+1, high);
    }
}


/**
 * Ordena una lista
 * 
*/
void GList::sort( int (* comparador)( shared_ptr<GObject>, shared_ptr<GObject>))
{
    sortQuickSort( comparador, 0, size()-1);
}

/**
 * Calcula el indica de la particion del algoritmo quicksort
 */
long GList::sortPartition( long low, long high, int (* comparador)( shared_ptr<GObject>, shared_ptr<GObject>) )
{
    shared_ptr<GObject> pi,pj;
    shared_ptr<GObject> pivot = get(high);
    long i = low-1;

    for (int j = low; j < high; j++) 
    {
        pj = get(j);
        if ( comparador(pj,pivot) < 0 ) 
        {
            i++;
            pi = get(i);
            set(i, pj);
            set(j, pi);
        }
    }
    i++;
    pi = get(i);
    set(i, get(high));
    set(high, pi);
   
    return i; // Devolver el �ndice del pivote
}


/**
 * Constructor
 */
GVector::GVector()
{    
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << endl;
    #endif

    initSize(TAM_INICIAL_LISTA);
}

/**
 * Constructor
 * 
 * capacidadIni : cantidad de elementos que se deben poder almacenar
 */
GVector::GVector( long capacidadIni )
{    
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << endl;
    #endif

    initSize(capacidadIni);
}

/**
 * Inicializa con un tamano el vector
 */
void GVector::initSize( long len )
{
    lstDatos = make_shared<vector<shared_ptr<GObject>>>();

    capacidadLista = len;
    posActual = 0;
    lstDatos->reserve(len);

    // cout << "GVector Dir Apuntado LstDatos = " << *lstDatos.get() << " " << ptrDatos  << endl;

    // cout << "Vector.initSize con " << capacidadLista << " elementos " << " ID Unico " << id_unico << endl;
}


/**
 * Asegura el tamao del buffer interno para almacenar
 * una cantgidad maxima de objetos.
 * 
 * capacidad : cantidad de objetos que se dese asegurar poder almacenar.
 */
void GVector::aseguraCapacidad( long capacidad )
{
    // cout << "Vector.aseguraCapacidad Capacidad actual:" << capacidadLista << " capacidad validada :" << capacidad << " Id Unico :" << id_unico << endl;
    // if ( capacidad > capacidadLista )
    // {
    //     lstDatos->resize(capacidad);
    //     capacidadLista = capacidad;
    // }
}


GVector::~GVector()
{
    #ifdef GOBJECT_VERBOSE
    cout << "Destructor :" << getType() << " ID:" << id_unico << " Ref: " << lstDatos.use_count() << endl;
    #endif    
    lstDatos->clear();
    // cout << "Fin desctructor GVector" << endl;
}


/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GVector::getType()
{
    return string("GVector");
}


/**
 * Retorna el elemento de una posicion dada.
 * Este elemento es el almacenado por el Gpointer
 * es equivalente a llamara a vector.get(indice).get()
 */
void * GVector::elementAt( long indice )
{
    if ( indice >= posActual ) return NULL;

    shared_ptr<GObject> ptr = lstDatos->at(indice);

    return ptr.get();
}

/**
 * Retorna un elemento de una posicion derterminada
 * @param indice el indice del elemento
 * @return elemeneto encontrado o NULL en caso no exista el indice
 */
shared_ptr<GObject> GVector::get( long indice )
{
    if ( indice >= posActual ) return NULL;

    shared_ptr<GObject> ptr = lstDatos->at(indice);

    return ptr;
}


/**
 * Retorna el ultimo elemento
 */
shared_ptr<GObject> GVector::getLast()
{
    if ( posActual == 0 ) return NULL;    
    return get(posActual-1);
}

/**
 * Elimina un elemento de la lista
 * @param indice posicion del elemento
 */
void GVector::remove( long indice )
{    
    if ( indice >= posActual ) return;
    if ( indice < 0 ) return;

    posActual--;
    lstDatos->erase( lstDatos->begin() + indice, lstDatos->begin() + indice+1);
}

/**
 * Elimina un elemento de la lista
 * @param ptr elemento que se elimina
 */
void GVector::remove( shared_ptr<GObject> ptr )
{
    long ndx = indexOf(ptr);
    if ( ndx < 0 ) return;

    remove(ndx);
}

/**
 * Retorna el indice de un elemento
 * @param ptr elemento que se busca, retorna -1 si no existe
 */
long GVector::indexOf( shared_ptr<GObject> ptr )
{
    std::shared_ptr<GObject> sptr;
    long i,n;

    n=size();
    for(i=0; i<n; i++)
    {
        sptr = lstDatos->at(i);
        if ( ptr == sptr )
        {
            return i;
        }
    }

    return -1;
}

/**
 * Establece el puntero almacenado en una posicion
 * 
 * @param indice posicion del elemento
 * 
 * @param elemento puntero que se guarda
 */
void GVector::set( long indice, shared_ptr<GObject> elemento )
{
    if (( indice < 0 ) || ( indice >= posActual )) 
    {
        return;
    }

    lstDatos->at(indice) = elemento;
}


/**
 * Limpia el arreglo
 * @param borraPuntero indica si se debe llamar a delte en el elemento
 */
void GVector::clear()
{    
    lstDatos->clear();
    posActual = 0;    
}

/**
 * Retorna la cantidad de elementos del vector
 */
long GVector::size()
{
    // cout << "GVector size: " << lstDatos->size() << endl;
    return posActual;
}


/**
 * Agrega un elemento a la lista
 * @param elemento elemento que se agrega
 */
void GVector::add( shared_ptr<GObject> elemento )
{
    // cout << "GVector.add GPointer " << " ID: " << id_unico << endl;

    if ( posActual == capacidadLista )
    {
        aseguraCapacidad(capacidadLista+INC_LISTA_LLENA);
    }
    
    lstDatos->push_back(elemento);
    posActual++;
}



/**
 * Agrega un entero
 */
void GVector::add( int valor )
{
    shared_ptr<GIntObject> val = make_shared<GIntObject>(valor);
    add(val);
}


/**
 * Agrega un long
 */
void GVector::add( long valor )
{
    shared_ptr<GLongObject> val = make_shared<GLongObject>(valor);
    add(val);
}

/**
 * Agrega un long long
 **/
void GVector::add( long long valor )
{
    shared_ptr<GLongLongObject>val = make_shared<GLongLongObject>(valor);
    add(val);
}

/**
 * Agrega un long long
 **/
void GVector::add( double valor )
{
    shared_ptr<GDoubleObject> val = make_shared<GDoubleObject>(valor);
    add(val);
}

/**
 * Agrega un long long
 **/
void GVector::add( string valor )
{
    shared_ptr<GStringObject> val = make_shared<GStringObject>(valor);
    add(val);
}

/**
 * Retorna un entero
 */
int GVector::getInt( long index )
{    
    if ( index > posActual )
        return 0;

    shared_ptr<GObject> ptr = lstDatos->at(index);
    GIntObject *val = (GIntObject *)ptr.get();

    return val->valor;
}

/**
 * Retorna un long
 */
long GVector::getLong( long index )
{
    if ( index > posActual )
        return 0;

    shared_ptr<GObject> ptr = lstDatos->at(index);
    GLongObject *val = (GLongObject *)ptr.get();

    return val->valor;
}

/**
 * Retorna un long
 */
long long GVector::getLongLong( long index )
{
    if ( index > posActual )
        return 0;

    shared_ptr<GObject> ptr = lstDatos->at(index);
    GLongLongObject *val = (GLongLongObject *)ptr.get();

    return val->valor;
}

/**
 * Retorna un double
 */
double GVector::getDouble( long index )
{
    if ( index > posActual )
        return 0;

    shared_ptr<GObject> ptr = lstDatos->at(index);
    GDoubleObject *val = (GDoubleObject *)ptr.get();

    return val->valor;
}

/**
 * Retorna un string
 */
string GVector::getString( long index )
{
    if ( index > posActual )
        return 0;

    shared_ptr<GObject> ptr = lstDatos->at(index);
    GStringObject *val = (GStringObject *)ptr.get();

    return val->valor;;
}

