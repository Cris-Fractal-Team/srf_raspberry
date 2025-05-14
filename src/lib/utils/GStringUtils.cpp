
#include <string>
#include <iostream>

#include "lib/utils/GStringUtils.h"
#include "lib/general/GObject.h"
#include "lib/general/GVector.h"


using namespace std;

/**
 * Cuenta cuantas veces aparece una sub-cadena dentro de una cadena
 */
int GStringUtils::subStrCount( string cadena, string subStr )
{
    int posIni,posFin,n,num;

    n = cadena.length();
    posIni = 0;
    num = 0;

    while( posIni < n )
    {
        posFin = cadena.find(subStr,posIni);
        if ( posFin >= 0 )
        {
            num++;
            posFin = posFin+1;
        }
        else
        {
            break;
        }
    }

    return num;
}

/**
 * Parte una cadena en base a un separador
 * 
 * cadena : cadena que se analiza
 * separador : separador de las partes
 * 
 * Retorna un vector con los elemenos encontrados
 */
GVector GStringUtils::split( string cadena, string separador )
{
    GVector rpta;
    string parte;

    int posIni,posFin,n;

    n = cadena.length();
    posIni = 0;

    // cout << "SPLIT de " << cadena << " CON SEPARADOR " << separador << " ID del Vector : "  << rpta.id_unico << endl;
    while( posIni < n )
    {
        posFin = cadena.find(separador,posIni);

        if ( posFin >= 0 )
        {
            parte = cadena.substr(posIni,posFin-posIni);

            rpta.add(make_shared<GStringObject>(parte));
            posIni = posFin+1;
        }
        else
        {            
            parte = cadena.substr(posIni);
            // cout << "No se encotro token, se agrega: "  << parte << endl;

            rpta.add(make_shared<GStringObject>(parte));
            posIni = n;
        }
    }

    // cout << "SPLIT Finaliza" << endl;
    return rpta;
}

/**
 * Decodifica una cadena codificada con formato URL
 */
string GStringUtils::decode_URLEncodedString( string cadena )
{
    string cadenaFinal,codHex;
    shared_ptr<GStringObject> txt;
    int i,n,codigo;

    // cout << "Decodificando : " << cadena << endl;

    if ( cadena.length() == 0 )
    {
        return cadena;
    }

    GVector lstTokens = GStringUtils::split(cadena,"%");
    n = lstTokens.size();

    if ( n== 0 )
    {
        return cadena;
    }

    // cout << "La cadena codificada tiene " << n << " tokens" << endl;

    // ptr = lstTokens.get(0);
    // cout << "Objecto inicial obtenido " << endl;

    // txt = (GStringObject *)&ptr;
    txt = dynamic_pointer_cast<GStringObject>(lstTokens.get(0));
    // cout << "Texto inicial obtenido " << endl;

    cadenaFinal.append(txt->valor);

    // cout << "La cadena inicia con : " << cadenaFinal << endl;
    
    for(i=1;i<n;i++)
    {        
        txt = dynamic_pointer_cast<GStringObject>(lstTokens.get(i));        
        codHex = txt->valor.substr(0,2);        
        // cout << "Procesando Token: " << txt->valor << endl;
        sscanf(codHex.c_str(), "%x", &codigo);
        cadenaFinal.append(string(1,char(codigo)));
        cadenaFinal.append(txt->valor.substr(2));
    }

    // cout << "Cadena decodificada: " << cadenaFinal << endl;

    return cadenaFinal;
}

/**
 * Retorna el ltrim de una cadena
 */
string GStringUtils::ltrim( string cadena )
{
    int pos = 0;
    int max = cadena.size();
    string rpta;
    rpta.reserve(max);

    while( pos < max )
    {
        if ( cadena.at(pos) > 32 ) break;
        pos++;
    }
    if ( pos == max ) return "";

    return cadena.substr(pos,max-pos);
}

/**
 * Retorna el rtrim de una cadena
 */
string GStringUtils::rtrim( string cadena )
{
    string rpta;
    int pos = cadena.size()-1;
    rpta.reserve(pos);

    while( pos >= 0)
    {
        if ( cadena.at(pos) > 32 ) break;
        pos--;
    }
    if ( pos < 0 ) return "";

    return cadena.substr(0,pos+1);
}

/**
 * Retorna el trim de una cadena
 */
string GStringUtils::trim( string cadena )
{
    return ltrim(rtrim(cadena));
}

/**
 * Calcula el indice de una cadena dentro de un arreglo de cadenas.
 *  cadena : cadena que se busca.
 *  lst: lista de cadenas.
 *  trim: indica si se debe hacer trim o no a los elementos del ventor.
 * 
 * Retorna -1 en caso no encuentre la cadena
 */
int GStringUtils::find( string cadena, GVector lst, bool trim)
{
    int n = lst.size();
    string txt;

    for( int i = 0; i < n; i++) 
    {
        txt = lst.getString(i);
        if ( trim == true ) txt = GStringUtils::trim(txt);
        if ( txt.compare(cadena) == 0 )
            return i;
    }

    return -1;
}

/**
 * Retorna la conversion de una cadena de texto a pura letra minuscula
 */
string GStringUtils::toLowerCase( string cadena )
{
    string rpta;
    int i,n;

    n = cadena.length();
    for(i=0;i<n;i++)
    {
        rpta+= ((char)tolower(cadena.at(i)));
    }

    return rpta;
}

/**
 * Retorna la conversion de una cadena de texto a pura letra mayuscula
 */
string GStringUtils::toUpperCase( string cadena )
{
    string rpta;
    int i,n;

    n = cadena.length();
    for(i=0;i<n;i++)
    {
        rpta+= ((char)toupper(cadena.at(i)));
    }

    return rpta;
}

/**
 * Agrega un atributo JSON a una cadena que almacena un JSON
 *      cadena:
 *          Cadena que tiene el JSON a la que se le agrega el atributo
 * 
 *      atributo:
 *          Nombre del atributo JSON
 * 
 *      valor:
 *          Valor del atributo
 * 
 *      esNumerico:
 *          Indica si el valor es numerico (true) y no se deben agregar comillas al rededor
 *          del valor, o es string (false) para el que se debe agregar comillas al rededor
 *          del valor
 * 
 *      agregaFinLinea:
 *          Indica si se debe o no agregar un fin de linea o caracter \n
 *      
 */
void GStringUtils::addJsonAtt( string *cadena, string atributo, string valor, bool esNumerico, bool agregaFinLinea )
{
    cadena->append("\"");
    cadena->append(atributo);
    cadena->append("\":");

    if (esNumerico == false ) cadena->append("\"");
    cadena->append(valor);
    if (esNumerico == false ) cadena->append("\"");

    if ( agregaFinLinea )
        cadena->append(",\n");
}

/**
 * Agrega un atributo JSON a una cadena que almacena un JSON
 *      cadena:
 *          Cadena que tiene el JSON a la que se le agrega el atributo
 * 
 *      atributo:
 *          Nombre del atributo JSON
 * 
 *      valor:
 *          Valor del atributo
 * 
 *      esNumerico:
 *          Indica si el valor es numerico (true) y no se deben agregar comillas al rededor
 *          del valor, o es string (false) para el que se debe agregar comillas al rededor
 *          del valor
 *  
 *      agregaFinLinea:
 *          Indica si se debe o no agregar un fin de linea o caracter \n
 *      
 */
void GStringUtils::addJsonAtt( string *cadena, string atributo, string *valor, bool esNumerico, bool agregaFinLinea )
{
    cadena->append("\"");
    cadena->append(atributo);
    cadena->append("\":");
    
    if (esNumerico == false ) cadena->append("\"");
    cadena->append(*valor);
    if (esNumerico == false ) cadena->append("\"");

    if ( agregaFinLinea )
        cadena->append(",\n");
}