#ifndef GOBJECT_H
#define GOBJECT_H

// #define GOBJECT_VERBOSE

#include <string>
#include <memory>

using namespace std;

/**
 * Clase base para las libreias Gimak
 */
class GObject
{
    public:

        /**
         * ID interno unico del objeto
         */
        static long long contador_ids;

        /**
         * Contador de la cantidad de veces en las que se ejecuto el destructor
         */
        static long long num_ids_destruidos;

        /**
         * ID unico del objeto
         */
        long long id_unico;        
        
        GObject();

        virtual ~GObject();

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

};


/**
 * Representa un puntero compartido
 */
class GPointer : public GObject
{
    public:

        /**
         * Constructor
         */
        GPointer();

        /**
         * Constructor
         */
        GPointer( GObject *ptr);

        /**
         * Constructor
         */
        GPointer( shared_ptr<GObject> ptr);

        /**
         * Destructor
         */ 
        ~GPointer();

        /**
         * Retorna una referencia al puntero contenido
         */
        void *get();        

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Asigna el valor almacenado
         */
        void set( GObject *valor );

        /**
         * Asigna el puntero compartido almacenado
         */
        void setSharedPtr( shared_ptr<GObject> ptr );

        /**
         * Retorna el puntero compartido
         */
        shared_ptr<GObject>getSharedPtr();

        /**
         * Sobrecarga para la direccion de un objeto
         */
        // void * operator &();


    private:
        shared_ptr<GObject> puntero = nullptr;
};


/**
 * Representa un puntero a un objeto que tiene un nombre
 */
class GNamedPointer : public GObject
{
    public :

        /**
         * Constructor
         */
        GNamedPointer();       

        /**
         * Constructor
         */
        GNamedPointer( string nombre, GObject *valor );       

        /**
         * Constructor
         */
        GNamedPointer( string nombre, shared_ptr<GObject> valor );       

        /**
         * Constructor
         */
        GNamedPointer( string nombre, GPointer valor );

        /**
         * Destructor
         */
        ~GNamedPointer();

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();


        /**
         * Nombre del objeto
         */
        string nombre;        

        /**
         * Puntero al que apunta
         */
        GPointer puntero;
};


/**
 * Objecto generico que encapsula a un entero
 */
class GIntObject : public GObject
{
    public:
        int valor;

        /**
         * Construtor
         */ 
        GIntObject();

        /**
         * Valor inicial del objeto
         * 
         * valIni : valor que se asigna a la variable
         */
        GIntObject( int valIni );

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el valor primitivo de la clase
         */
        operator int();
};

/**
 * Objecto generico que encapsula a un long
 */
class GLongObject : public GObject
{
    public:
        long valor;

        /**
         * Construtor
         */ 
        GLongObject();

        /**
         * Destructor
         */ 
        ~GLongObject();

        /**
         * Valor inicial del objeto
         * 
         * valIni : valor que se asigna a la variable
         */
        GLongObject( long valIni );

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el valor primitivo de la clase
         */
        operator long();
};

/**
 * Objecto generico que encapsula a un long
 */
class GLongLongObject : public GObject
{
    public:
        long long valor;

        /**
         * Construtor
         */ 
        GLongLongObject();

        /**
         * Destructor
         */ 
        ~GLongLongObject();

        /**
         * Valor inicial del objeto
         * 
         * valIni : valor que se asigna a la variable
         */
        GLongLongObject( long long valIni );

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el valor primitivo de la clase
         */
        operator long long();
};


/**
 * Objecto generico que encapsula a un double
 */
class GDoubleObject : public GObject
{
    public:
        double valor;

        /**
         * Construtor
         */ 
        GDoubleObject();

        /**
         * Valor inicial del objeto
         * 
         * valIni : valor que se asigna a la variable
         */
        GDoubleObject( double valIni );

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el valor primitivo de la clase
         */
        operator double();
};


/**
 * Objecto generico que encapsula a un string
 */
class GStringObject : public GObject
{
    public:
        string valor;

        /**
         * Construtor
         */ 
        GStringObject();

        /**
         * Valor inicial del objeto
         * 
         * valIni : valor que se asigna a la variable
         */
        GStringObject( string valIni );

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el valor primitivo de la clase
         */
        operator string();
};

/**
 * Objecto generico que encapsula a una instancia de GPointer
 */
class GPointerObject : public GPointer
{
    public:

        /**
         * Constructor simple
         */
        GPointerObject();

                
        /**
         * Valor inicial del objeto
         * 
         * valIni : valor que se asigna a la variable
         */
        GPointerObject( void *valIni );

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el valor primitivo de la clase
         */
        void * get();

        /**
         * Establece el valor almacenado
         */
        void set( void *ptr );


    private:
        void * valor;
};



#endif // GOBJECT_H
