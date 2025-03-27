
#ifndef GSOCKET_H
#define GSOCKET_H

#include <string>

#include  "openssl/bio.h"
#include  "openssl/ssl.h"
#include  "openssl/err.h"

using namespace std;



/**
 * Representa un cliente HTTP
 */
class GSocket
{
    public:

        
        /**
         * Constructor
         */
        GSocket();

        /**
         * Destructor
         */
        ~GSocket();


        /**
         * Inicia un servidor socke en un puerto dado
         * 
         * ip: direccion ip que monitorea el servidor
         * 
         * puerto : numero del puerto IP que monitorea el servidor
         * 
         * retorna 0 en caso de exito, otro valor en caso de error
         */     
        void iniciaServidor( string ip, int puerto );


        /**
         * Abre una conexion con un servidor socket
         * 
         * ip : direccion IP o nombre del servidor
         * 
         * puerto : puerto al que se conecta
         * 
         * usar SSL : indica si se debe usar encriptacion con el servidor, 1 si se debe usaro, 0 si no se debe usar.
         * 
         * return 0 en caso de exito, otro valor en caso de error
         */
        void abrirConexion( string ip, int puerto, int usarSSL );

        /**
         * Escribe una cantidad de bytes en el socket 
         * 
         * buffer: posicion de memoria desde el que se escriben los bytes.
         * 
         * len : cantidad de bytes que se deben escribir o enviar
         * 
         * retorna 0 en caso de exito, otro valor en caso de error
         */
        void write( char *buffer, long len );

        /**
         * Lee datos desde el socket.
         * 
         * buffer: posicion de memoria en la que se escriben los datos
         * 
         * maxLen : cantidad maxima de datos que se deben leer
         * 
         * retorna la cantidad de bytes leidos, un valor negativo en caso de error.
         */
        long read( char *sbuffer, long maxLen );

        /**
         * Lee N datos desde el socket.
         * 
         * buffer: posicion de memoria en la que se escriben los datos
         * 
         * len : cantidad de datos que se deben leer
         * 
         * retorna 0 si pudo leer los datos con exito, diferente a cero en caso de error.
         */
        void readLen( char *sbuffer, long len );


        /**
         * Acepta un cliente que se ha conectado al servidor, en caso que no haya clientes se bloquea.
         * 
         * retorna una instancia de GSocket o NULL
         */
        GSocket aceptarCliente();

        /**
         * Valida si el socket esta o no conectado
         * retorna 1 si esta conectado, 0 en caso que no esta conectado
         */ 
        int estaConectado();

        /**
         * Cierra la conexion del socket o del servidor
         * 
         * retorna 0 en caso de exito u otro valor en caso de error
         */
        void cerrar();

        /**
         * Solicita limpiar el buffer de lectura.
         * Con eso se puede garantizar que no hay nada pendiente que leer
         */
        void clearInputBuffer();

        /**
         * Establece el timeout en segundos
         */
        void setTimeout( int segundos );


        /**
         * Envia un valor entero sin signo de 8 bits
         * 
         * valor: valor que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeUint8( uint8_t valor );

        /**
         * Envia un valor entero sin signo de 16 bits
         * 
         * valor: valor que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeUint16( uint16_t valor );

        /**
         * Envia un valor entero sin signo de 32 bits
         * 
         * valor: valor que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeUint32( uint16_t valor );

        /**
         * Envia un valor entero
         * 
         * valor: valor que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeInt( int valor );

        /**
         * Envia un valor long
         * 
         * valor: valor que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeLong( long long valor );

        /**
         * Envia una cadena de lenguaje C, terminada en caracter nulo.
         * 
         * cadena: cadena que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeChar( char *cadena );

        /**
         * Envia una cadena de lenguaje C.
         * 
         * cadena: cadena que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeSmallString( string cadena );

        /**
         * Envia una cadena de lenguaje C, terminada en caracter nulo.
         * 
         * cadena: cadena que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeSmallChar( char *cadena );

        /**
         * Envia una cadena de lenguaje C++, terminada en caracter nulo.
         * 
         * cadena: cadena que se envia
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void writeString( string cadena );

        /**
         * Lee un valor entero sin signo de 32 bits desde el socket
         * 
         * valor : posicion de memoria en la que se lee el valor
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        uint32_t readUint32();

        /**
         * Lee un valor entero sin signo de 16 bits desde el socket
         * 
         * valor : posicion de memoria en la que se lee el valor
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        uint16_t readUint16();

        /**
         * Lee un valor long desde el socket
         * 
         * valor : posicion de memoria en la que se lee el valor
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        long readLong();

        /**
         * Lee una cadena desde el socket.
         * Es responsabilidad del codigo que llamo a la funcion de liberar la memoria usada por el buffer
         * 
         * Retorna el buffer en el que se creo la cadena, NULL en caso de error.
         */
        char *readChar();

        /**
         * Lee una cadena desde el socket.
         * En caso que la cadena que se debe lee es mayor al valor maximo expresado, se trunca la cadena.
         * 
         * buffer: buffer en el que se escribe la cadena
         * 
         * maxLen: cantidad maxima de caracteres que se leen
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void readChar( char *buffer, int maxLen );

        /**
         * Lee una cadena desde el socket.
         * En caso que la cadena que se debe lee es mayor al valor maximo expresado, se trunca la cadena.
         * 
         * buffer: buffer en el que se escribe la cadena
         * 
         * maxLen: cantidad maxima de caracteres que se leen
         * 
         * Retorna 0 en caso de exito, otro valor en caso de error
         */
        void readSmallChar( char *buffer, int maxLen );



    private:

        /**
         * Indica si la libreria SSL ya fue inicializada
         */
        static int libInicializada;

        /**
         * Indica si es un servidor 1 o un cliente 0
         */
        int esServidor;

        /**
         * Indica si esta conectado o activo el socket
         * 1: si esta activo
         * 0: no esta activo
         */
        int conectado;
        
        /**
         * IP o nombre del servidor
         */
        char const *ipServidor;

        /**
         * Puerto del servidor
         */
        int puertoServidor;

        /**
         * Inidica si se debe usar OpenSSL
         */
        int usarOpenSSL;

        /**
         * Socket con el que se trabaja
         */
        BIO * bio;

        /**
         * ID de la conexion con el puerto, realizado por un servidor
         */
        long idBindPuerto;

        /**
         * Conexto SSL con el que se trabaja
         */
        SSL_CTX  *ctx;
    
        /**
         * Configuracion SSL
         */
        SSL  *ssl;
        
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
        
};

#endif  // GSOCKET_H
