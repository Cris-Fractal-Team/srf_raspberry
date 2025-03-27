
#ifndef GHTTPSERVERCOMMAND_H
#define GHTTPSERVERCOMMAND_H

#include <string>

#include "GSocket.h"
#include "HttpHeaderReader.h"
#include "lib/general/GVector.h"
#include "lib/general/GHashMap.h"
#include "GHttpRequest.h"
#include "GHttpResponse.h"
#include "GHttpServer.h"


using namespace std;


class GHttpServer;

/**
 * Configuracion de un procesador de request del servidor
 */
class GHttpServerCommand : public GObject
{
    public:
        
        /**
         * Referencia al servidor al que pertence
         */
        GHttpServer *servidor;

        /**
         * Nombre del cookie usado para contener el ID de las sesiones
         */
        static string idCookieSession;

        /**
         * Indica que la sesion HTTP es obligatoria para procesar eventos en el comando
         */
        bool sesionOblogatoria;
          
        /**
         * Constructor
         */
        GHttpServerCommand();

        /**
         * Destructor
         */
        ~GHttpServerCommand();


        /**
         * Establece la ruta que maneja
         * 
         * ruta: ruta que procesa del servidor se puede agregar al final un asterisco para indicar que
         *          procesa todos los URLs que inician con el texto anterior al asterisco
         * 
         * metodoHttp: puede ser GET, POST, GET_POST refiriendoce a los dos metodos
         */
        void setRuta( string ruta, string metodoHttp );

        /**
         * Valida si el comando procesa una ruta que se ha enviado al servidor.
         * 
         * ruta : ruta que se ha invocado
         * 
         * metodo : metodo HTTP que se empleo
         */
        bool esProcesador( string ruta, string metodo );

        /**
         * Dado un codigo HTML, lo analiza buscando secuencias del tipo <% valor %>
         * Por cada elemento de ese tipo, busca el valor en el hashmap lstValores y lo reemplaza,
         * si no existe una clave en el hashmap se retorna cadena vacia.
         * Esto ayuda a tener codigo HTML dinamico simple
         */
        string reemplazaTagHtml( string html, shared_ptr<GHashMap> lstValores );

        /**
         * Retorna la ruta del command
         */
        string getRuta();

        /**
         * Retorna la ruta raiz sin el comodin del command
         */
        string getRutaRaiz();

        /**
         * Procesa una peticion
         * 
         * redireccion : indica si es una invocacion directa (false) o se ha redireccionado desde otro Command (true)
         */
        virtual void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion );

    private:

        /**
         * Ruta base
         */
        string rutaBase;

        /**
         * Metodo que procesa
         */
        string metodoProc;

        /**
         * Ruta raiz, empleada cuando el comendo procesa varios urls
         */
        string rutaRaiz;      

        /**
         * Indica que se trata de un procesador de URLs generico
         */
        bool esUrlGenerico;

};


/**
 * Representa un comando que envia el contenido de archivos binarios o de texto desde 
 * una ruta base del sistema de archivos, donde la ruta relativa del URL del elemento 
 * se concatena a un PATH del sistema y de esa manera se obtiene la ruta fisica del archivo.
 * 
 * El sistema obtendra la lista de los CONTENT/TYPES desde un archivo de texto, que podra
 * ser completado segun sea el caso.
 * 
 * Por ejemplo si se configuran:
 *  a) El URL:  /resources/* 
 *  b) El pathBaseArchivos : /home/usuario/www/resources/
 * 
 * Si se solicita la imagen con SCR = "/resources/images/fotos/logo.jpeg";
 * 
 * El comando retornara el contenido binario del archivo: /home/usuario/www/resources/images/fotos/logo.jpeg
 *  
 */
class GHttpStaticContentCommand : public GHttpServerCommand
{
    public:
        
        /**
         * Ruta base de los archivos 
         */
        string pathBaseArchivos;

        /**
         * Mapa que usa las extensiones de los archivos como clave para retornar un string
         * que contiene el content/type.
         */
        shared_ptr<GHashMap> lstContenTypes;

        /**
         * Constructor
         */
        GHttpStaticContentCommand();

        /**
         * Destructor
         */
        ~GHttpStaticContentCommand();

        /**
         * Lee los contenttypes desde un archivo
         */
        void cargaContentTypes( string path );

        /**
         * Procesa una peticion
         */
        void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion ) override;
};


#endif