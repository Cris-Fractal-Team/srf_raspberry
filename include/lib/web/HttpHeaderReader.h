

#ifndef HTTPHEADERREADER_H
#define HTTPHEADERREADER_H

#include "GSocket.h"

using namespace std;

/**
 * Clase que lee y procesa el header de una transaccion HTTP
 */
class HttpHeaderReader
{
    public:

        /**
         * Buffer en el que se almacena la cabecera
         */
        char *dataCabecera;

        /**
         * Codigo HTTP de la respuesta
         */
        int httpError;

        /**
         * Cantidad de datos leidos
         */
        long lenData;

        /**
         * Metodo de la peticion
         */
        string metodoReq;

        /**
         * URL de la peticion
         */
        string urlReq;

        /**
         * Query string de la peticion
         */
        string queryString;

        /**
         * Constructor
         */
        HttpHeaderReader();
        

        /**
         * Destructor
         */
        ~HttpHeaderReader();

        
        /**
         * Lee la respuesta HTTP
         * 
         * socket : socket desde el que se lee la respuesta
         * 
         * Genera una excepcion del tipo runtimeexception en caso de error
         */
        void leeRespuestaHttp( GSocket socket );

        /**
         * Lee la cabecera HTTP
         * 
         * socket : socket desde el que se lee la respuesta
         * 
         * Genera una excepcion del tipo runtimeexception en caso de error
         */
        void leeCabeceraHttp( GSocket socket );    


        /**
         * Retorna el valor de una cabecera
         * 
         * nombre : nombre de la cabecera que se busca
         * 
         * retorna el valor de la cabecera o cadena blanca en caso no exista
         */
        string getHeader( string nombre );

        /**
         * Retorna el valor de una cabecera convertido a long
         * 
         * nombre : nombre de la cabecera que se busca
         * 
         * errVal : valor que se debe retornar en caso no se pueda convertir a long
         * 
         * retorna el valor convertido a string, en caso no exista o no se pueda convertir a string retorna errVal
         */
        long getHeaderLong( string nombre, long errVal );

        /**
         * Lee datos basicos desde la cabecera por una respuesta
         */
        void leeDatosBasicosRpta();

        /**
         * Lee datos basicos desde la cabecera para una peticion
         */
        void leeDatosBasicosPeticion();
        
        
};

#endif
