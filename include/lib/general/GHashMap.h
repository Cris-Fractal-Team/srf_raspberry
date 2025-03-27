#ifndef GHASHMAP_H
#define GHASHMAP_H

#include <string>

#include "GVector.h"
#include "GObject.h"

using namespace std;


/**
 * Representa un haspmap
 */
class GHashMap : public GObject
{
    public:

        /**
         * Constructor
         */
        GHashMap();

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el valor de una variable entera almacenada
         * 
         * varName : nombre de la variable
         * 
         * errVal : valor que se debe retornar en caso no exita la variable o no sea entera
         */
        int getInt( string varName, int errVal );

        /**
         * Retorna el valor de una variable long almacenada
         * 
         * varName : nombre de la variable
         * 
         * errVal : valor que se debe retornar en caso no exita la variable o no sea long
         */
        long getLong( string varName, long errVal );

        /**
         * Retorna el valor de una variable long almacenada
         * 
         * varName : nombre de la variable
         * 
         * errVal : valor que se debe retornar en caso no exita la variable o no sea long
         */
        long long getLongLong( string varName, long long errVal );

        /**
         * Retorna el valor booleano de una variable.
         * El valor interno sera un string con los siguiente posibles valores:
         * 
         *      true = true, t, s, si
         *      false = false, f, n, no
         * 
         * El parametro errVal se retorna en caso no exista la variable
         */
        bool getStringBool( string varName, bool errVal );


        /**
         * Retorna el valor de una variable double almacenada
         * 
         * varName : nombre de la variable
         * 
         * errVal : valor que se debe retornar en caso no exita la variable o no sea double
         */
        double getDouble( string varName, double errVal );

        /**
         * Retorna el valor de una variable string almacenada
         * 
         * varName : nombre de la variable
         */
        string getString( string varName );

        /**
         * Retorna el valor de una variable string almacenada que luego se convierte a long
         * 
         * varName : nombre de la variable
         * 
         * errVal : valor que se debe retornar en caso no exista la variable o no sea string o no se pueda
         *          convertir a long
         */
        long getStringLong( string varName, long errVal );

        /**
         * Retorna el valor de una variable string almacenada que luego se convierte a double
         * 
         * varName : nombre de la variable
         * 
         * errVal : valor que se debe retornar en caso no exista la variable o no sea string
         *          o no se pueda convetir a double
         */
        double getStringDouble( string varName, double errVal );

        /**
         * Retorna el valor de una variable puntero almacenada
         * 
         * varName : nombre de la variable
         */
        shared_ptr<GObject> get( string varName );

        /**
         * Retorna un HashMap almacenado como objeto
         * 
         * varName : nombre de la variable
         */
        shared_ptr<GHashMap> getHashMap( string varName );

        /**
         * Retorna un Vector almacenado como objeto
         * 
         * varName : nombre de la variable
         */
        shared_ptr<GVector> getVector( string varName );        
                        
        /**
         * Guarda una variable entera
         * 
         * varName : nombre de la variable
         * 
         * valor : valor que se almacena
         */
        void putInt( string varName, int valor );

        /**
         * Guarda una variable long
         * 
         * varName : nombre de la variable
         * 
         * valor : valor que se almacena
         */
        void putLong( string varName, long valor );

        /**
         * Guarda una variable long long
         * 
         * varName : nombre de la variable
         * 
         * valor : valor que se almacena
         */
        void putLongLong( string varName, long long valor );

        /**
         * Guarda una variable double
         * 
         * varName : nombre de la variable
         * 
         * valor : valor que se almacena
         */
        void putDouble( string varName, double valor );


        /**
         * Guarda una variable double
         * 
         * varName : nombre de la variable
         * 
         * valor : valor que se almacena
         */
        void putString( string varName, string valor );

        /**
         * Remueve una variable
         * 
         * varName : nombre de la variable se borra
         */
        void remove( string varName );

        /**
         * Retorna la lista de claves del mapa
         */
        std::vector<string> getLstClaves();

        /**
         * Valida si una clave o codigo existe en el mapa
         */
        bool hasKey( string key );

        /**
         * Guarda un elemento en el hash
         */
        void put( string clave, shared_ptr<GObject> valor );  

    private:
        
        /**
         * Lista de claves
         */
        std::vector<string> lstClaves;

        /**
         * Lista de valores
         */
        GVector lstValores;

        /**
         * Elimina un elemento del hash
         */
        void remove( int indice );

        /**
         * Retorna el indice de un parametro
         * 
         * varName : nombre del parametro
         * 
         * retorna el indice o -1 en caso no exita el parametro
         */
        int getIndiceParam( string varName );

};

#endif