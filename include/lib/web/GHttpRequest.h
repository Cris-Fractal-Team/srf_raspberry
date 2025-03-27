

#ifndef GHTTPREQUEST_H
#define GHTTPREQUEST_H

#include <string>

#include "GSocket.h"
#include "HttpHeaderReader.h"
#include "lib/general/GVector.h"
#include "lib/general/GHashMap.h"


using namespace std;

/**
 * Representa una peticion HTTP
 */
class GHttpRequest : public GObject
{
    public:
    
        /**
         * Constructor
         */
        GHttpRequest( shared_ptr<HttpHeaderReader> reader, GSocket socket );

        /**
         * Destructor
         */
        ~GHttpRequest();


        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna el URL invocado
         */
        string getUrl();

        /**
         * Retorna el URL invocado
         */
        string getQueryString();

        /**
         * Retorna el metodo HTTP
         */
        string getMetodo();

        /**
         * Retorna la data pura recibida por el post
         * Empleada en caso no se haya enviado como un formularios
         */
        string getPostData();

        /**
         * Retorna el valor de un parametro recibido
         * 
         * nombre : nombre del parametro cuyo valor se solicita
         */
        string getParam( string nombre );

        /**
         * Retorna el valor del parametro convertido a un numero entero
         * 
         * nombre : nombre del parametro cuyo valor se solicita
         * 
         * valErr : valor en caso no exista el parametros
         */
        int getParamInt( string nombre, int valErr );

        /**
         * Retorna el valor del parametro convertido a un numero entero
         * 
         * nombre : nombre del parametro cuyo valor se solicita
         * 
         * valErr : valor en caso no exista el parametros
         */
        long getParamLong( string nombre, long valErr );

        /**
         * Retorna el valor del parametro convertido a un numero double
         * 
         * nombre : nombre del parametro cuyo valor se solicita
         * 
         * valErr : valor en caso no exista el parametros
         */
        double getParamDouble( string nombre, double valErr );

        /**
         * Valida si un parametro es un arreglo
         */
        bool isParamArray( string nombre );

        /**
         * Retorna un parametro como un vector cuyos elementos son instancias de GStringObject
         */
        shared_ptr<GVector> getParamArray( string nombre );

        /**
         * Retorna la cantidad de parametros que tiene la peticion
         */
        int getParamCount();

        /**
         * Retorna el nombre del parametro en unap osicion dada
         */
        string getParamName( int index );

        /**
         * Retorna el valor de campo de la cabecera    
         */
        string getHeader( string param );

        /**
         * Retorna el valor de un cookie dado su nombre.
         * SI el cookie no existe retorna cadena vacia
         */
        string getCookie( string cookie );

        /**
         * Retorna el valor de campo de la cabecera convertido a numero entero
         * 
         */
        long long getHeaderLong( string param, long long valError );

    private:

        /**
         * Lee los datos de la cabecera
         */
        shared_ptr<HttpHeaderReader> headerReader;     

        /**
         * Socket desde el que se leen los datos
         */
        GSocket socketIn;   

        /**
         * Parametros enviados
         */
        GHashMap parametros;



        /**
         * Lee los datos enviados via POST y llena el HashMap de parametros
         */
        void leeDataPost();

        /**
         * Lee data post enviado como una cadena de texto simple
         * Por lo general es un XML o JSON
         */
        void leeDataPostTexto();

        /**
         * Lee data post enviado como un formulario
         */
        void leeDataPostForm();

        /**
         * Lee los datos del query string y llena el HashMap de parametros
         */
        void leeDataQueryString();

        /**
         * Agrega un parametro o campo.
         * Detecta si el parametro esta repetido, y en ese caso lo convierte en un arreglo.
         * 
         * nombre: nombre del parametro
         * valor : valor del parametro
         */
        void agregaParametro( string nombre, string valor );
};


#endif
