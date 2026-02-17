#ifndef _GLINKED_LIST_
#define _GLINKED_LIST_

#include <memory.h>
#include <stdexcept>
#include <iostream>

using namespace std;

/**
 * Nodo de la lista doblemente enlazada
 */
template <typename T>
class GLinkedListNode
{
    public:
        T data;
        GLinkedListNode<T>* next;
        GLinkedListNode<T>* prev;
};

/**
 * Lista doblemente enlazada
 */
template <typename T>
class GLinkedList
{
    public:
        GLinkedList();
        GLinkedList(const GLinkedList<T> &base);   // ✅ const copy ctor
        ~GLinkedList();

        GLinkedList<T> getClone();

        void addAll(const GLinkedList<T> &list);   // ✅ const
        void reset();

        void add(T value);

        T get(int index) const;                    // ✅ const
        T getLast() const;                         // ✅ const

        T *getAddr(int index);
        const T *getAddr(int index) const;         // ✅ const overload

        T *getAddrUltimo();
        const T *getAddrUltimo() const;            // ✅ const overload

        void set(int index, T value);

        int indexOf(T value);

        GLinkedListNode<T> *getNode(int index);            // non-const
        GLinkedListNode<T> *getNode(int index) const;      // ✅ const overload

        void remove(int index);
        void removeLast();

        int size() const;

        GLinkedList<T>& operator=(const GLinkedList<T>& other);

        bool contains(T value);
        bool contains(T value) const;

    private:
        GLinkedListNode<T> *head;
        GLinkedListNode<T> *tail;
        int numElem;

        // ✅ cache mutable para permitir get() const con caching
        mutable int ultimaPosLeida;
        mutable GLinkedListNode<T> *ptrUltimaPosLeida;
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
 * Constructor copia (const)
 */
template<typename T>
GLinkedList<T>::GLinkedList(const GLinkedList<T> &base)
{
    numElem = 0;
    head = NULL;
    tail = NULL;
    ultimaPosLeida = -1;
    ptrUltimaPosLeida = NULL;

    addAll(base);
}

/**
 * Destructor
 */
template<typename T>
GLinkedList<T>::~GLinkedList()
{
    reset();
}

/**
 * borra todos los elementos de la lista
 */
template<typename T>
void GLinkedList<T>::reset()
{
    while (numElem > 0)
        removeLast();

    ptrUltimaPosLeida = NULL;
    ultimaPosLeida = -1;
    head = NULL;
    tail = NULL;
    numElem = 0;
}

/**
 * Agrega un valor a la lista
 */
template<typename T>
void GLinkedList<T>::add(T value)
{
    if (head == NULL)
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
 * Retorna un valor de la lista (const)
 */
template<typename T>
T GLinkedList<T>::get(int index) const
{
    if (index == ultimaPosLeida && ptrUltimaPosLeida != NULL)
        return ptrUltimaPosLeida->data;

    GLinkedListNode<T> *nodo = getNode(index);
    return nodo->data;
}

/**
 * Retorna un valor de la lista (ultimo) (const)
 */
template<typename T>
T GLinkedList<T>::getLast() const
{
    if (tail != NULL)
        return tail->data;

    GLinkedListNode<T> *nodo = getNode(numElem - 1);
    return nodo->data;
}

/**
 * Establece el valor de un elemento de la lista
 */
template<typename T>
void GLinkedList<T>::set(int index, T value)
{
    GLinkedListNode<T> *nodo = getNode(index);
    nodo->data = value;
}

/**
 * Retorna la posicion de un elemento en la lista
 */
template<typename T>
int GLinkedList<T>::indexOf(T value)
{
    int n = size();
    for (int i = 0; i < n; i++)
    {
        T valor = get(i);
        if (valor == value)
            return i;
    }
    return -1;
}

/**
 * Retorna un puntero a un nodo (non-const)
 */
template<typename T>
GLinkedListNode<T> *GLinkedList<T>::getNode(int index)
{
    // reutiliza el const overload (sin duplicar lógica)
    return const_cast<GLinkedListNode<T>*>(
        static_cast<const GLinkedList<T>*>(this)->getNode(index)
    );
}

/**
 * Retorna un puntero a un nodo (const)
 */
template<typename T>
GLinkedListNode<T> *GLinkedList<T>::getNode(int index) const
{
    if ((index < 0) || (index >= numElem))
        throw std::out_of_range("Inidex out of range");

    GLinkedListNode<T> *nodo;
    int pos;

    if ((ultimaPosLeida >= 0) && (index > ultimaPosLeida) && (ptrUltimaPosLeida != NULL))
    {
        pos = index - ultimaPosLeida;
        nodo = ptrUltimaPosLeida;
    }
    else
    {
        pos = index;
        nodo = head;
    }

    while (pos > 0)
    {
        nodo = nodo->next;
        pos--;
    }

    // cache
    ultimaPosLeida = index;
    ptrUltimaPosLeida = nodo;

    return nodo;
}

/**
 * Elimina un nodo de la lista
 */
template<typename T>
void GLinkedList<T>::remove(int index)
{
    GLinkedListNode<T> *nodo = getNode(index);

    if (nodo == NULL)
    {
        cout << "Error nodo NULL" << endl;
        nodo = getNode(index);
    }

    if (nodo == head)
    {
        head = head->next;
        if (head != NULL)
            head->prev = NULL;

        if (tail == nodo)
            tail = NULL;
    }
    else if (nodo == tail)
    {
        tail = nodo->prev;
        if (tail != NULL)
            tail->next = NULL;
    }
    else
    {
        nodo->prev->next = nodo->next;
        nodo->next->prev = nodo->prev;
    }

    delete nodo;

    if (index <= ultimaPosLeida)
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
    if (numElem == 0)
        return;

    remove(numElem - 1);
}

/**
 * Retorna la cantidad de elementos
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
 * Agrega todos los elementos de una lista a la lista actual (const)
 */
template<typename T>
void GLinkedList<T>::addAll(const GLinkedList<T> &list)
{
    int n = list.size();
    for (int i = 0; i < n; i++)
    {
        add(list.get(i));
    }
}

/**
 * Retorna la direccion de memoria de un elemento (non-const)
 */
template<typename T>
T *GLinkedList<T>::getAddr(int index)
{
    GLinkedListNode<T> *nodo = getNode(index);
    return &nodo->data;
}

/**
 * Retorna la direccion de memoria de un elemento (const)
 */
template<typename T>
const T *GLinkedList<T>::getAddr(int index) const
{
    GLinkedListNode<T> *nodo = getNode(index);
    return &nodo->data;
}

/**
 * Retorna la direccion de memoria del ultimo elemento (non-const)
 */
template<typename T>
T *GLinkedList<T>::getAddrUltimo()
{
    if (tail == NULL) return NULL;
    return &tail->data;
}

/**
 * Retorna la direccion de memoria del ultimo elemento (const)
 */
template<typename T>
const T *GLinkedList<T>::getAddrUltimo() const
{
    if (tail == NULL) return NULL;
    return &tail->data;
}

/**
 * Copia los valores de una lista a otra
 */
template<typename T>
GLinkedList<T>& GLinkedList<T>::operator=(const GLinkedList<T>& other)
{
    reset();

    int n = other.size();
    GLinkedListNode<T> *nodoBase = other.head;

    for (int i = 0; i < n; i++)
    {
        add(nodoBase->data);
        nodoBase = nodoBase->next;
    }

    return *this;
}

/**
 * Indica si un valor existe en la lista (non-const)
 */
template<typename T>
bool GLinkedList<T>::contains(T value)
{
    return const_cast<const GLinkedList<T>*>(this)->contains(value);
}

/**
 * Indica si un valor existe en la lista (const)
 */
template<typename T>
bool GLinkedList<T>::contains(T value) const
{
    int n = size();
    for (int i = 0; i < n; i++)
    {
        T v = get(i);
        if (v == value)
            return true;
    }
    return false;
}

#endif
