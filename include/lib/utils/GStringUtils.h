
#include <string>

#include "lib/general/GVector.h"
#include "lib/general/GObject.h"

#ifndef GSTRINGUTILS_H
#define GSTRINGUTILS_H

using namespace std;

class GStringUtils
{
    public:

        /**
         * Cuenta cuantas veces aparece una sub-cadena dentro de una cadena
         */
        static int subStrCount( string cadena, string subStr );

        /**
         * Parte una cadena en base a un separador
         * 
         * cadena : cadena que se analiza
         * separador : separador de las partes
         * 
         * Retorna un vector con los elemenos encontrados
         */
        static GVector split( string cadena, string separador );   

        /**
         * Retorna el ltrim de una cadena
         */
        static string ltrim( string cadena );

        /**
         * Retorna el rtrim de una cadena
         */
        static string rtrim( string cadena );

        /**
         * Retorna el trim de una cadena
         */
        static string trim( string cadena );

        /**
         * Calcula el indice de una cadena dentro de un arreglo de cadenas.
         *  cadena : cadena que se busca.
         *  lst: lista de cadenas.
         *  trim: indica si se debe hacer trim o no a los elementos del ventor.
         */
        static int find( string cadena, GVector lst, bool trim);

        /**
         * Decodifica una cadena codificada con formato URL
         */
        static string decode_URLEncodedString( string cadena );


        /**
         * Retorna la conversion de una cadena de texto a pura letra minuscula
         */
        static string toLowerCase( string cadena );

        /**
         * Retorna la conversion de una cadena de texto a pura letra mayuscula
         */
        static string toUpperCase( string cadena );

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
        static void addJsonAtt( string *cadena, string atributo, string valor, bool esNumerico, bool agregaFinLinea = true );

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
        static void addJsonAtt( string *cadena, string atributo, string *valor, bool esNumerico,bool agregaFinLinea = true );


        /**
         * Convierte un valor decimal a un string con una cantidad de decimales de presicion
         */
        static string to_string_fixed( float valor, int decimales );

         /**
         * Convierte un valor decimal a un string con una cantidad de decimales de presicion
         */
        static string to_string_fixed( double valor, int decimales );

        /**
         * Convierte un valor entero en un string con N digitos y completa si es necesario
         * con ceros a la izquierd
         */
        static string to_fixed_digits( int valor, int digits );    
};

#endif