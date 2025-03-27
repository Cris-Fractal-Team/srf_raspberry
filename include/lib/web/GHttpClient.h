
#ifndef GHTTPCLIENT_H
#define GHTTPCLIENT_H

#include <string>

#include  "openssl/bio.h"
#include  "openssl/ssl.h"
#include  "openssl/err.h"

using namespace std;

/**
 * Representa un cliente HTTP
 */
class GHttpClient
{
    public:

        /**
         * Buffer en el que se almacena la cabecera
         */
        char *dataCabecera;

        /**
         * Cantidad de datos leidos
         */
        long lenData;

        /**
         * BUffer en el que se guardo toda la respuesta enviada
         */
        char *bufferResponse;

        /**
         * Codigo HTTP de la respuesta
         */
        int httpError;

        /**
         * Constructor
         */
        GHttpClient();

        /**
         * Destructor
         */
        ~GHttpClient();

        /**
         * Hace un envio HTTP a un URL
         * 
         * url : url en el que se envia la data
         * method : metodo que se usa : GET o POST
         * data : datos
         * 
         * Retorna 0 ne caso de exito.
         * -100 En caso no existe protocolo
         */
        int doHttp( string url, string method, char *data = NULL, long dataLen = 0 );

        /**
         * Establece el valor de una cabecera para hacer una peticion
         * 
         * cabecera: nombre de la cabecera
         * valor : valor que se le asigna a la cabecera
         */
        void setHeader( string cabecera, string valor );


        /**
         * Retorna la respuesta convertida a string
         */
        string responseToStr();


    private:

        /**
         * Indica si la libreria SSL ya fue inicializada
         */
        static int libInicializada;
        
        /**
         * IP o nombre del servidor
         */
        char *ipServidor;

        /**
         * Puerto del servidor
         */
        int puertoServidor;

        /**
         * Inidica si se debe usar OpenSSL
         */
        int usarOpenSSL;

        /**
         * Cabecera que se enviara con la peticion
         */
        string cabeceraPeticion;

        /**
         * Socket con el que se trabaja
         */
        BIO * bio;

        /**
         * Conexto SSL con el que se trabaja
         */
        SSL_CTX  *ctx;
    
        /**
         * Configuracion SSL
         */
        SSL  *ssl;

        /**
         * Hace un GET hacia un servidor
         * servidor: nombre o ip del servidor
         * puerto: puerto ip del servidor
         * usarSSL: indica si se debe (1) o no (0) usar SSL para encriptar los datos
         * url: ruta que se desea obtener
         * data : datos que se encian con POST
         * dataLen : cantidad de bytes que se envian con POST
         */
        int _doHttp( const char *servidor, int puerto, int usarSSL, const char *metodo, const char *url, char *data = NULL, long dataLen = 0  );
        
        /**
         * Inicia la conexion con el servidor         
         */
        int iniciaConexion();

        /**
         * Cierra la conexion
         */
        void cierraConexion();

        /**
         * Escribe datos en el socket
         * 
         * buffer: datos que se escriben
         * len : cantidad de datos que se escriben
         * 
         * Retorna la cantidad de datos que escribio.
         * -1 Indica error en escritura
         */
        int writeData( char *buffer, long len );

        /**
         * Envia los datos de un cadena.
         * Internamente llama a writeData con el puntero de datos del string
         * 
         * Retorna la cantidad de datos que se envio
         */ 
        int writeData( string cadena );

        /**
         * Lee datos desde el socket
         * 
         * buffer: buffer en el que se escriben los datos
         * maxLen: cantidad maxima de datos que se escriben
         * 
         * Retorna la cantidad de bytes que se leyeron, -1 indica fin de conexion
         */
        long readData( char *buffer, long maxLen );

        /**
         * Lee los datos desde el socket hasta completar la lectura
         * de una cantidad de "len" bytes.
         * 
         * buffer: buffer en el que se escriben los datos
         * len : cantida de datos que se deben de leer.
         * 
         * Retorna 0 en caso de exito, -1 en caso de error
         */
        int readDataLen( char *buffer, long len );

        /**
         * Lee la respuesta HTTP
         * Retorna 0 en caso de exito.
         * -1 Indica error al leer cabecera.
         * -3 Indica que no se encontro el content-length del request y/o Transfer-Encoding
         * -2 Indica error de RAM.
         */
        int leeRespuestaHttp();

        /**
         * Lee la cabecera HTTP
         * Retorna 0 en caso de exito otros valores negativos en caso de error
         */
        int leeCabeceraHttp();

        /**
         * Lee la data en base al content-length
         * Retorn 0 en caso de exito,-1 en caso de error de data , -2 en caso de falta de ram
         */
        int leeDataContentLength();

        /**
         * Lee la data en base a bloques
         * Retorn 0 en caso de exito,-1 en caso de error de data , -2 en caso de falta de ram
         */
        int leeDataBloques();

        /**
         * Lee la cantidad de bytes que el servidor ha enviado para el sigueinte bloque de datos (chunck)
         * 
         * Retorna la cantidad de datos que se deben leer. -1 en caso de error
         */
        long leeTamDatosChunk();

};

#endif  // GHTTPCLIENT_H
