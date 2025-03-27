#include "lib/general/GObject.h"

#include <string>
#include <stdio.h>
#include <iostream>


using namespace std;

long long GObject::contador_ids = 0;

long long GObject::num_ids_destruidos = 0;

GObject::GObject()
{    
    contador_ids++;
    id_unico = contador_ids;

    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << endl;
    #endif

}

GObject::~GObject()
{
    #ifdef GOBJECT_VERBOSE
    cout << "Destructor:  " << getType() << " ID:" << id_unico << endl;
    #endif
    num_ids_destruidos++;
}


/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GObject::getType()
{
    return string("GObject");
}


/**
 * Constructor
 */
GPointer::GPointer()
{
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << endl;
    #endif

    puntero.reset();
}

 /**
 * Constructor
 */
GPointer::GPointer( GObject *ptr)
{
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor Pointer : " << getType() << " ID: " << id_unico  << endl;
    #endif

    set(ptr);  
}

/**
 * Constructor
 */
GPointer::GPointer( shared_ptr<GObject> ptr)
{
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor Pointer : " << getType() << " ID: " << id_unico << endl;
    #endif

    puntero = ptr;
}

/**
 * Destructor
 */ 
GPointer::~GPointer()
{
    cout << "Destructor :" << getType() << " ID:" << id_unico << " Ref: " << puntero.use_count() << " ID objeto: " <<  puntero->id_unico << endl;
   
    #ifdef GOBJECT_VERBOSE
    cout << "Destructor :" << getType() << " ID:" << id_unico << " Ref: " << puntero.use_count() << " ID objeto: " <<  puntero->id_unico << endl;
    #endif    
}

/**
 * Retorna una referencia al puntero contenido
 */
void *GPointer::get()
{
    return puntero.get();
}


/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GPointer::getType()
{
    return string("GPointer");
}

/**
 * Asigna el valor almacenado
 */
void GPointer::set( GObject *valor )
{    
    // shared_ptr<void> p = make_shared<void>(valor);
    puntero.reset(valor);
}

/**
 * Asigna el puntero compartido almacenado
 */
void GPointer::setSharedPtr( shared_ptr<GObject> ptr )
{
    puntero = ptr;
}

/**
 * Retorna el puntero compartido
 */
shared_ptr<GObject> GPointer::getSharedPtr()
{
    return puntero;
}

/**
 * Sobrecarga para la direccion de un objeto
 */
//void * GPointer::operator &()
//{
//    return puntero.get();
// }


/**
 * Constructor
 */
GNamedPointer::GNamedPointer()
{
    nombre = string();
}

/**
 * Constructor
 */
GNamedPointer::GNamedPointer( string nombrePtr, GObject *valorAlm )
{
    nombre = nombrePtr;
    puntero.set(valorAlm);
}   

/**
 * Constructor
 */
GNamedPointer::GNamedPointer( string nombrePtr, shared_ptr<GObject> valor )
{
    nombre = nombrePtr;
    puntero.setSharedPtr(valor);
}

/**
 * Constructor
 */
GNamedPointer::GNamedPointer( string nombrePtr, GPointer valor )
{
    nombre = nombrePtr;
    puntero = valor;
}

/**
 * Destructor
 */
GNamedPointer::~GNamedPointer()
{
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GNamedPointer::getType()
{
    return string("GNamedPointer");
}


/**
 * Construtor
 */ 
GIntObject::GIntObject()
{
}

/**
 * Valor inicial del objeto
 * 
 * valIni : valor que se asigna a la variable
 */
GIntObject::GIntObject( int valIni )
{
    valor = valIni;
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GIntObject::getType()
{
    return string("int");
}

/**
 * Retorna el valor entero del objeto
 */
GIntObject::operator int() 
{ 
    return valor; 
}


/**
 * Construtor
 */ 
GLongObject::GLongObject()
{
    valor = -1;
}

/**
 * Valor inicial del objeto
 * 
 * valIni : valor que se asigna a la variable
 */
GLongObject::GLongObject( long valIni )
{
    valor = valIni; 
}

/**
 * Destructor
 */ 
GLongObject::~GLongObject()
{
    // cout << "Destructor :" << getType() << " = " << valor << endl;
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GLongObject::getType()
{
    return string("long");
}

/**
 * Retorna el valor primitivo de la clase
 */
GLongObject::operator long() 
{ 
    return valor; 
}



/**
 * Construtor
 */ 
GLongLongObject::GLongLongObject()
{
    valor = -1;
}

/**
 * Valor inicial del objeto
 * 
 * valIni : valor que se asigna a la variable
 */
GLongLongObject::GLongLongObject( long long valIni )
{
    valor = valIni; 
}

/**
 * Destructor
 */ 
GLongLongObject::~GLongLongObject()
{
    // cout << "Destructor :" << getType() << " = " << valor << endl;
}


/**
 * Retorna el valor primitivo de la clase
 */
GLongLongObject::operator long long() 
{ 
    return valor; 
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GLongLongObject::getType()
{
    return string("longlong");
}






/**
 * Construtor
 */ 
GDoubleObject::GDoubleObject()
{
}

/**
 * Valor inicial del objeto
 * 
 * valIni : valor que se asigna a la variable
 */
GDoubleObject::GDoubleObject( double valIni )
{
    valor = valIni;
}


/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GDoubleObject::getType()
{
    return string("double");
}

/**
 * Retorna el valor primitivo de la clase
 */
GDoubleObject::operator double() 
{ 
    return valor; 
}

 /**
 * Construtor
 */ 
GStringObject::GStringObject()
{
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << endl;
    #endif
}

/**
 * Valor inicial del objeto
 * 
 * valIni : valor que se asigna a la variable
 */
GStringObject::GStringObject( string valIni )
{
    valor = valIni;
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << " Valor: " << valIni << endl;
    #endif
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GStringObject::getType()
{
    return string("string");
}


/**
 * Retorna el valor primitivo de la clase
 */
GStringObject::operator string() 
{ 
    return valor; 
}


/**
 * Construtor
 */ 
GPointerObject::GPointerObject()
{
    valor = NULL;
}

/**
 * Valor inicial del objeto
 * 
 * valIni : valor que se asigna a la variable
 */
GPointerObject::GPointerObject( void * valIni )
{
    valor = valIni;
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GPointerObject::getType()
{
    return string("pointer");
}
 

/**
 * Retorna el valor primitivo de la clase
 */
void * GPointerObject::get()
{
    return valor;
}

/**
 * Establece el valor almacenado
 */
void GPointerObject::set( void *ptr )
{
    valor = ptr;
}

