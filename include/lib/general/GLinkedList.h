

#ifndef _GLINKED_LIST_
#define _GLINKED_LIST_

#include <memory.h>
#include <stdexcept>


/**
 * Nodo de la lista doblemente enlazada
 */
template <typename T>
class GLinkedListNode
{
    public :

        /**
         * Valor almacenado en el nodo
         */
        T data;

        /**
         * Referencia al siguiente elemento
         */
        GLinkedListNode<T>*next;

        /**
         * Referencia al elemento anterior
         */
        GLinkedListNode<T>*prev;        
};


/**
 * Lista doblemente enlazada
 */
template <typename T>
class GLinkedList
{
    public:

        /**
         * Constructor
         */
        GLinkedList();

        /**
         * Destructor
         */
        ~GLinkedList();

        /**
         * Crear una lista clone 
         */
        GLinkedList<T> getClone();

        /**
         * Agrega todos los elementos de una lista a la lista actual
         */
        void addAll( GLinkedList<T> list );

        /**
         * borra todos los elementos de la lista
         */
        void reset();        
    
        /**
         * Agrega un valor a la lista
         */
        void add( T value );

        /**
         * Retorna un valor de la lista
         */
        T get( int index ) const;

        /**
         * Retorna un valor de la lista
         */
        T getLast();

        /**
         * Retorna la direccion de memoria de un elemento de la lista
         */
        T *getAddr( int index );

        /**
         * Retorna la direccion de memoria del ultimo elemento de la lista
         */
        T *getAddrUltimo();

        /**
         * Establece el valor de un elemento de la lista
         */
        void set( int index, T value );

        /**
         * Retorna un nodo de la lista dado su indice o ubicacion
         */
        GLinkedListNode<T> *getNode( int index ) const;        

        /**
         * Elimina un nodo de la lista
         */
        void remove( int index );

        /**
         * Remueve el ultimo elemento
         */
        void removeLast();

        /**
         * Retorna la cantidad de elementos de la lista
         */
        int size() const;

        /**
         * Copia los valores de una lista a otra
         */
        GLinkedList<T>& operator=(const GLinkedList<T>& other);

    private:

            /**
         * Cabeza de la lista
         */
        GLinkedListNode<T> *head;

        /**
         * Cola de la lista
         */
        GLinkedListNode<T> *tail;

        /**
         * Numero de elementos de la lista
         */
        int numElem;

        /**
         * Ultima posicion accedida
         */
        int ultimaPosLeida;

        /**
         * Puntero a la ultima posicion leida
         */
        GLinkedListNode<T>ptrUltimaPosLeida;

};



/**
 * Constructor
 */
template<typename T>
GLinkedList<T>::GLinkedList()
{
    numElem = 0;
    head = NULL;
    tail = NULL;
    ultimaPosLeida = -1;
    ptrUltimaPosLeida = NULL;
}

/**
 * Destructor
 */
template<typename T>
GLinkedList<T>::~GLinkedList()
{
}

/**
 * borra todos los elementos de la lista
 */
template<typename T>
void GLinkedList<T>::reset()
{
    while( numElem > 0 )
        removeLast();

    ptrUltimaPosLeida = NULL;
    ultimaPosLeida = -1;
}

/**
 * Agrega un valor a la lista
 */
template<typename T>
void GLinkedList<T>::add( T value )
{
    if ( head == NULL )
    {
        head = new GLinkedListNode<T>();
        head->data = value;
        head->prev = NULL;
        head->next = NULL;
        
        tail = head;        
    }
    else
    {
        GLinkedListNode<T> *newNode = new GLinkedListNode<T>();
        newNode->prev = tail;
        newNode->next = NULL;
        newNode->data = value;
        tail->next = newNode;

        tail = newNode;
    }

    numElem++;
}

/**
 * Retorna un valor de la lista
 */
template<typename T>
T GLinkedList<T>::get( int index ) const
{
    if ( index == ultimaPosLeida )
        return ptrUltimaPosLeida->data;

    GLinkedListNode<T> *nodo = getNode(index);
    
    return nodo->data;
}

/**
 * Retorna un valor de la lista
 */
template<typename T>
T GLinkedList<T>::getLast()
{
    if ( tail != NULL )
        return tail->data;

    GLinkedListNode<T> *nodo = getNode(numElem-1);
    
    return nodo->data;
}


/**
 * Establece el valor de un elemento de la lista
 */
template<typename T>
void GLinkedList<T>::set( int index, T value )
{
    GLinkedListNode<T> *nodo = getNode(index);
    nodo->data = value;
}

/**
 * Retorna el puntero a un nodo
 */
template<typename T>
GLinkedListNode<T> *GLinkedList<T>::getNode( int index ) const
{
    if (( index < 0 ) || ( index >= numElem ))
        throw std::out_of_range("Inidex out of range");

    GLinkedListNode<T> *nodo;
    int indexOrig = index;

    if (( ultimaPosLeida >= 0 ) &&( index > ultimaPosLeida ))
    {
        index-= ultimaPosLeida;
        nodo = ptrUltimaPosLeida;
    }
    else
    {
        nodo = head;
    }

    while( index > 0 )
    {
        nodo = nodo->next;
        index--;
    }
    ultimaPosLeida = indexOrig;
    ptrUltimaPosLeida = nodo;

    return nodo;
}

/**
 * Elimina un nodo de la lista
 */
template<typename T>
void GLinkedList<T>::remove( int index )
{
    GLinkedListNode<T> *nodo = getNode(index);

    if ( nodo == head )
    {
        head = head->next;
        if ( head != NULL )
        {
            head->prev = NULL;
        }        
        if ( tail == nodo )
        {
            tail = NULL;
        }
    }
    else
    if ( nodo == tail )
    {
        tail = nodo->prev;
        if ( tail != NULL)
            tail->next = NULL;
    }
    else
    {
        nodo->prev->next = nodo->next;
        nodo->next->prev = nodo->prev; 
    }

    delete nodo;

    if ( index <= ultimaPosLeida )
    {
        ultimaPosLeida = -1;
        ptrUltimaPosLeida = NULL;
    }
    
    numElem--;
}

/**
 * Remueve el ultimo elemento
 */
template<typename T>
void GLinkedList<T>::removeLast()
{
    if ( numElem == 0 )
        return;

    GLinkedListNode<T> *nodo = tail;
   
    if ( tail == head )
    {
        head = NULL;
        tail = NULL;
    }
    else
    {
        tail = tail->prev;
    }

    ultimaPosLeida = -;
    ptrUltimaPosLeida = NULL;

    numElem--;

    delete nodo;
}

/**
 * Retorna la cantidad de elementos de la lista
 */
template<typename T>
int GLinkedList<T>::size() const
{
    return numElem;
}   

/**
 * Crear una lista clone 
 */
template<typename T>
GLinkedList<T> GLinkedList<T>::getClone()
{
    GLinkedList<T> rpta;

    rpta.addAll(*this);

    return rpta;
}

/**
 * Agrega todos los elementos de una lista a la lista actual
 */
template<typename T>
void GLinkedList<T>::addAll( GLinkedList<T> list )
{
    int n = list.size();
    for(int i=0;i<n;i++)
    {
        add(list.get(i));
    }          
}

/**
 * Retorna la direccion de memoria de un elemento de la lista
 */
template<typename T>
T *GLinkedList<T>::getAddr( int index )
{
    GLinkedListNode<T> *nodo = getNode(index);

    return &nodo->data;
}   

/**
 * Retorna la direccion de memoria del ultimo elemento de la lista
 */
template<typename T>
T *GLinkedList<T>::getAddrUltimo() 
{
    if ( tail == NULL ) return NULL;

    return &tail->data;
}


/**
 * Copia los valores de una lista a otra
 */
template<typename T>
GLinkedList<T>& GLinkedList<T>::operator=(const GLinkedList<T>& other)
{
    int i,n;
    reset();
    
    n = other.size();
    for(i=0;i<n;i++)
    {
        add(other.get(i));
    }

    return *this;
}


#endif