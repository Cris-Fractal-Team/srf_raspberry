
#ifndef GHTTPRESPONSE_H
#define GHTTPRESPONSE_H

#include <string>

#include "GSocket.h"
#include "lib/general/GObject.h"


using namespace std;

/**
 * Codigo de la accion que indica no hacer nada
 */ 
#define HTTP_RESPONSE_ACCION_NONE 0

/**
 * Codigo de la accion que indica que se envio una respuesta
 */
#define HTTP_RESPONSE_ACCION_PROCESSED 1

/**
 * Representa una respuesta enviada via HTTP
 */
class GHttpResponse  : public GObject
{
    public:

        /**
         * Constructor
         */
        GHttpResponse( GSocket socket );

        /**
         * Destructor
         */
        ~GHttpResponse();

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Agrega un valor de header
         * 
         * nombre: nombre de la cabecera
         * 
         * valor : valor que se le asigna a la cabecesra
         */
        void addHeader( string nombre, string valor );

        /**
         * Establece la cantidad de datos que se envian
         */
        void setContentLength( long len );

        /**
         * Establece el tipo de datos que se realiza
         */
        void setContentType( string tipo );

        /**
         * Establece el codigo HTTP de respuesta
         */
        void setHttpCode( int codigo );

        /**
         * Establece una respuesta del tipo texto
         * 
         * rpta: texto de la respuesta
         * 
         * contentType: formato del texto de la respuesta
         */
        void setTextResponse( string rpta, string contentType );

        /**
        * Establece una respuesta exitosa del tipo binario.
        * Es responsabilidad del metodo que invoca de liberar la RAM
        * 
        * buffer: puntero a la estructura que tienel os bytes que se deben enviar
        * bufferLen : cantidad de bytes que se deben enviar
        * contentType : tipo de datos binarios enviados
        */
        void setBinaryResponse( void *buffer, long bufferLen, string contentType );

        /**
         * Establece una respuesta exitosa del tipo HTML
         */
        void setHtmlResponse( string html );

        /**
         * Establece una respuesta exitosa del tipo HTML
         */
        void setJsonResponse( string html );

        /**
         * Establece una respuesta exitosa del tipo HTML
         */
        void setXmlResponse( string html );


        /**
         * Envia una respuesta personalizada
         * 
         * responseCode: primera linea de la respuesta HTTP
         * 
         * buffer: datos que se envian luego 
         */
        void setCustomResponse( string resposeCode, void *buffer, int bufferLen );

        /**
         * Agrega un cookie
         * 
         * nombre : nombre del cookie
         * valor : valor del cookie
         * path : ruta base para la cual el cookie es valido
         * duracion : cantidad de segundos que dura el cookie
         */
        void addCookie( string nombre, string valor, string path, long duracion );

        /**
         * Retorna la accion
         */
        int getAccion();

        /**
         * Retorna el socket asociado
         */
        GSocket getSocket();

    private:

        /**
         * Datos de la cabecera que se debe enviar
         */
        string cabecera;     

        /**
         * Socket con el que se debe trabajar
         */
        GSocket socketOut;

        /**
         * Codigo de la accion que debe seguir, definida en constantes internas
         * HTTP_RESPONSE_ACCION_XXXX
         */
        int accion;

        /**
         * Codigo HTTP de respuesta
         */
        int httpResponseCode;
        
};

#endif
