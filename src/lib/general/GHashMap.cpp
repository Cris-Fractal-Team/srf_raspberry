
#include <string>
#include <malloc.h>
#include <memory>
#include <iostream>

#include "lib/general/GHashMap.h"
#include "lib/general/GVector.h"
#include "lib/general/GObject.h"
#include "lib/utils/GStringUtils.h"


/**
 * Constructor
 */
GHashMap::GHashMap()
{

}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GHashMap::getType()
{
    return string("GHashMap");
}


/**
 * Retorna el valor de una variable entera almacenada
 * 
 * varName : nombre de la variable
 * 
 * errVal : valor que se debe retornar en caso no exita la variable o no sea entera
 */
int GHashMap::getInt( string varName, int errVal )
{
    shared_ptr<GIntObject> valor = dynamic_pointer_cast<GIntObject>(get(varName));

    if ( valor == nullptr ) return errVal;
    
    if ( valor->getType().compare("int") != 0 ) return errVal;

    return valor->valor;
}

/**
 * Retorna el valor de una variable long almacenada
 * 
 * varName : nombre de la variable
 * 
 * errVal : valor que se debe retornar en caso no exita la variable o no sea long
 */
long GHashMap::getLong( string varName, long errVal )
{
     shared_ptr<GLongObject> valor = dynamic_pointer_cast<GLongObject>(get(varName));

    if ( valor == nullptr ) return errVal;
    
    if ( valor->getType().compare("long") != 0 ) return errVal;

    return valor->valor;
}


/**
 * Retorna el valor de una variable long almacenada
 * 
 * varName : nombre de la variable
 * 
 * errVal : valor que se debe retornar en caso no exita la variable o no sea long
 */
long long GHashMap::getLongLong( string varName, long long errVal )
{
     shared_ptr<GLongLongObject> valor = dynamic_pointer_cast<GLongLongObject>(get(varName));;

    if ( valor == nullptr ) return errVal;
    
    if ( valor->getType().compare("longlong") != 0 ) return errVal;

    return valor->valor;
}


/**
 * Retorna el valor de una variable double almacenada
 * 
 * varName : nombre de la variable
 * 
 * errVal : valor que se debe retornar en caso no exita la variable o no sea double
 */
double GHashMap::getDouble( string varName, double errVal )
{
     shared_ptr<GDoubleObject> valor = dynamic_pointer_cast<GDoubleObject>(get(varName));;

    if ( valor == nullptr ) return errVal;
    
    if ( valor->getType().compare("double") != 0 ) return errVal;

    return valor->valor;
}

/**
 * Retorna el valor de una variable string almacenada
 * 
 * varName : nombre de la variable
 */
string GHashMap::getString( string varName )
{
     shared_ptr<GStringObject> valor = dynamic_pointer_cast<GStringObject>(get(varName));;

    if ( valor == nullptr ) return "";
    
    if ( valor->getType().compare("string") != 0 ) return "";

    return valor->valor;
}
 

/**
 * Retorna el valor de una variable puntero almacenada
 * 
 * varName : nombre de la variable
 */
shared_ptr<GObject> GHashMap::get( string varName )
{
    int indice = getIndiceParam(varName);

    if ( indice == -1 ) 
    {        
        return nullptr;
    }
    
    shared_ptr<GObject> puntero = lstValores.get(indice);
        
    return puntero;
}


/**
 * Retorna un HashMap almacenado como objeto
 * 
 * varName : nombre de la variable
 */
shared_ptr<GHashMap> GHashMap::getHashMap( string varName )
{
    shared_ptr<GHashMap> ptr = dynamic_pointer_cast<GHashMap>(get(varName));;

    if ( ptr == nullptr ) return make_shared<GHashMap>();

    return ptr;
}

/**
 * Retorna un Vector almacenado como objeto
 * 
 * varName : nombre de la variable
 */
shared_ptr<GVector> GHashMap::getVector( string varName )
{
    shared_ptr<GVector> ptr = dynamic_pointer_cast<GVector>(get(varName));;

    if ( ptr == nullptr ) return make_shared<GVector>();

    return ptr;
}



/**
 * Guarda una variable entera
 * 
 * varName : nombre de la variable
 * 
 * valor : valor que se almacena
 */
void GHashMap::putInt( string varName, int valor )
{
    int indice = getIndiceParam(varName);

    if ( indice >= 0 ) remove(varName);

    put(varName,make_shared<GIntObject>(valor));
}

/**
 * Guarda una variable long
 * 
 * varName : nombre de la variable
 * 
 * valor : valor que se almacena
 */
void GHashMap::putLong( string varName, long valor )
{
    int indice = getIndiceParam(varName);

    if ( indice >= 0 ) remove(varName);
    put(varName,make_shared<GLongObject>(valor));
}

/**
 * Guarda una variable long long
 * 
 * varName : nombre de la variable
 * 
 * valor : valor que se almacena
 */
void GHashMap::putLongLong( string varName, long long valor )
{
    int indice = getIndiceParam(varName);

    if ( indice >= 0 ) remove(varName);
    put(varName,make_shared<GLongLongObject>(valor));
}


/**
 * Guarda una variable double
 * 
 * varName : nombre de la variable
 * 
 * valor : valor que se almacena
 */
void GHashMap::putDouble( string varName, double valor )
{
    int indice = getIndiceParam(varName);

    if ( indice >= 0 ) remove(varName);
    put(varName,make_shared <GDoubleObject>(valor));
}

/**
 * Guarda una variable entera
 * 
 * varName : nombre de la variable
 * 
 * valor : valor que se almacena
 */
void GHashMap::putString( string varName, string valor )
{
    int indice = getIndiceParam(varName);

    if ( indice >= 0 ) remove(varName);

    put(varName,make_shared<GStringObject>(valor));
}


/**
 * Remueve una variable
 * 
 * varName : nombre de la variable que se borra
 * 
 * borraPuntero : indice si se desea que se borre o no el puntero
 */
void GHashMap::remove( string varName )
{
    int indice = getIndiceParam(varName);

    if ( indice == -1 ) return;

    remove(indice);    
}


/**
 * Retorna el indice de un parametro
 * 
 * varName : nombre del parametro
 * 
 * retorna el indice o -1 en caso no exita el parametro
 */
int GHashMap::getIndiceParam( string varName )
{
    int num;
    string nombre;

    num = lstClaves.size();
    for(int i = 0 ; i < num; i++ )
    {
        nombre = lstClaves[i];
        if ( nombre.compare(varName) == 0 )
        {
            return i;
        }
    }
    return -1;
}



/**
 * Guarda un elemento en el hash
 */
void GHashMap::put( string clave, shared_ptr<GObject> valor )
{
    int index = getIndiceParam(clave);

    if ( index < 0 )
    {
        lstClaves.push_back(clave);
        lstValores.add(valor);
        return;
    }

    lstValores.set(index, valor);
}


/**
 * Elimina un elemento del hash
 */
void GHashMap::remove( int indice )
{
    lstClaves.erase(lstClaves.begin()+indice);
    lstValores.remove(indice);
}

/**
 * Retorna la lista de claves del mapa
 */
vector<string> GHashMap::getLstClaves()
{
    return lstClaves;
}

/**
 * Valida si una clave o codigo existe en el mapa
 */
bool GHashMap::hasKey( string key )
{
    int ndx = getIndiceParam(key);
    if ( ndx < 0 ) return false;

    return true;
}

/**
 * Retorna el valor de una variable string almacenada que luego se convierte a long
 * 
 * varName : nombre de la variable
 * 
 * errVal : valor que se debe retornar en caso no exista la variable o no sea string o no se pueda
 *          convertir a long
 */
long GHashMap::getStringLong( string varName, long errVal )
{
    int index;
    string data;

    index = getIndiceParam(varName);
    if ( index < 0 )
    {
        return errVal;
    }

    data = lstValores.getString(index);

    try
    {
        return stol(data);
    }
    catch( exception e )
    {
        return errVal;
    }
}

/**
 * Retorna el valor de una variable string almacenada que luego se convierte a double
 * 
 * varName : nombre de la variable
 * 
 * errVal : valor que se debe retornar en caso no exista la variable o no sea string
 *          o no se pueda convetir a double
 */
double GHashMap::getStringDouble( string varName, double errVal )
{
    int index;
    string data;

    index = getIndiceParam(varName);
    if ( index < 0 )
    {
        return errVal;
    }

    data = lstValores.getString(index);

    try
    {
        return stod(data);
    }
    catch( exception e )
    {
        return errVal;
    }
}

/**
 * Retorna el valor booleano de una variable.
 * El valor interno sera un string con los siguiente posibles valores:
 * 
 *      true = true, t, s, si
 *      false = false, f, n, no
 * 
 * El parametro errVal se retorna en caso no exista la variable
 */
bool GHashMap::getStringBool( string varName, bool errVal )
{
    int index;
    string data;

    index = getIndiceParam(varName);
    if ( index < 0 )
    {
        return errVal;
    }

    data = lstValores.getString(index);
    data = GStringUtils::toLowerCase(data);

    if ( data.compare("t") == 0 ) return true;
    if ( data.compare("true") == 0 ) return true;
    if ( data.compare("si") == 0 ) return true;
    if ( data.compare("s") == 0 ) return true;

    if ( data.compare("f") == 0 ) return false;
    if ( data.compare("false") == 0 ) return false;
    if ( data.compare("no") == 0 ) return false;
    if ( data.compare("n") == 0 ) return false;

    cout << "getStringBool Error : " << varName << " = " << data << endl;

    return errVal;
}