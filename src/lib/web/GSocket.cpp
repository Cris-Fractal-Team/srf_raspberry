#include "lib/web/GSocket.h"

//============================================================================
// Name        : SSLClient.cpp
// Compiling   : g++ -c -o SSLClient.o SSLClient.cpp
//               g++ -o SSLClient SSLClient.o -lssl -lcrypto
//============================================================================
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h> /* BasicInput/Output streams */
#include <string.h>
#include <iostream>
#include <time.h>
#include <thread>
#include <unistd.h>

#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>

#include "lib/general/GObject.h"

using namespace std;


/**
 * Indica que la librerira SSL aun no fue inicializada 
 */
int GSocket::libInicializada = 0;

/**
 * Constructor
 */
GSocket::GSocket()
{
    esServidor = 0;
    conectado = 0;
    usarOpenSSL = 0;
}


/**
 * Destructor
 */
GSocket::~GSocket()
{
    // cerrar();
}


/**
 * Inicia un servidor socke en un puerto dado
 * 
 * ip: direccion ip que monitorea el servidor
 * 
 * puerto : numero del puerto IP que monitorea el servidor
 * 
 * retorna 0 en caso de exito, otro valor en caso de error
 */     
void GSocket::iniciaServidor( string ip, int puerto )
{
    char puertoStr[10];
    char *cadenaCon;

    esServidor = 1;
    puertoServidor = puerto;
    ipServidor = ip.c_str();

    sprintf(puertoStr,"%d",puertoServidor);

    cadenaCon = (char *)malloc(strlen(ipServidor)+strlen(puertoStr)+3);

    if ( strlen(ipServidor) < 0 )
    {
        sprintf(cadenaCon,"%s:%s",ipServidor,puertoStr);
    }
    else
    {
        sprintf(cadenaCon,"%s",puertoStr);
    }
    cout << "Cadena conexion socket : " << cadenaCon << endl;
    

    if ( GSocket::libInicializada == 0 )
    {        
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
        GSocket::libInicializada = 1;

        // cout << "Libreria SSL iniciada " << endl;
    }

    if ( usarOpenSSL == 0 )
    {
        // crea el objeto BIO asociado al servidor
        bio = BIO_new_accept(cadenaCon);
        free(cadenaCon);
        if ( bio == NULL )
        {
            // cout << "No se pudo crear el BIO inicial " << endl;
            // return 1;
            throw std::runtime_error("No se pudo crear el servidor socket");
        }

        // se enlaza al puerto
        idBindPuerto = BIO_do_accept(bio);
        if ( idBindPuerto <= 0 )
        {
            cout << "No se pudo enlazar el puerto " << idBindPuerto << endl;
            // return 2;
            throw std::runtime_error("No se pudo enlazar el servidor socket al puerto");
        }

        // implementa una opcion para que no se bloquee el monitoreo
        // BIO_set_nbio_accept(bio,1);

        cout << "Servidor enlazado " << endl;
        conectado = 1;
    }
    else
    {
        // falta implementar
        free(cadenaCon);
    }

    // return 0;
}


/**
 * Solicita limpiar el buffer de lectura.
 * Con eso se puede garantizar que no hay nada pendiente que leer
 */
void GSocket::clearInputBuffer()
{
    if ( conectado == 0 ) return;

    const int bufferSize = 8192;  
    char buffer[bufferSize];
    int bytesRead = 0;

    // Leer datos mientras haya algo disponible en el buffer
    while ((bytesRead = BIO_read(bio, buffer, bufferSize)) > 0) {        
        std::cout << "Descartando " << bytesRead << " bytes del buffer." << std::endl;
        memset(buffer, 0, bufferSize);  // Limpiar el buffer temporal
    }

    // Verificar si hubo un error o si la lectura termin� correctamente
    if (bytesRead < 0) {
        if (!BIO_should_retry(bio)) {
            std::cerr << "Error: Fallo en la lectura del BIO." << std::endl;
        }
    } else if (bytesRead == 0) {
        std::cout << "El buffer del socket ha sido limpiado completamente." << std::endl;
    }
}


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
void GSocket::abrirConexion( string ip, int puerto, int usarSSL )
{
    char puertoStr[10];
    char *cadenaCon;

    puertoServidor = puerto;
    ipServidor = ip.c_str();

    sprintf(puertoStr,"%d",puertoServidor);

    cadenaCon = (char *)malloc(strlen(ipServidor)+strlen(puertoStr)+3);
    sprintf(cadenaCon,"%s:%s",ipServidor,puertoStr);

    if ( GSocket::libInicializada == 0 )
    {        
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
        GSocket::libInicializada = 1;
    }

    if ( usarOpenSSL == 0 )
    {
        // inicia conexion sin encriptado
        bio = BIO_new_connect(cadenaCon);
        free(cadenaCon);
        if(bio == NULL)
        {
            // cout << "Error al crear objeto de conexion " << endl;            
            // return 1;
            throw std::runtime_error("NO se pudo crear el socket");
        }

        if(BIO_do_connect(bio) <= 0)
        {
            // cout << "Error al abrir conexion " << endl;
            // return 2;
            throw std::runtime_error("NO se pudo conectar con el servidor");
        }
        conectado = 1;

        // return 0;
    }
    else
    {
        // inicia conexion con encriptado
        ctx = SSL_CTX_new(SSLv23_client_method());
        
        cout << "Configuracion de ubicacion de certificados" << endl;
        if(! SSL_CTX_load_verify_locations(ctx, NULL, "/usr/lib/ssl/certs"))
        {
            cout << "Error al verificar la ubicacion de los certificados" << endl;
            free(cadenaCon);
            // return 3;
            throw std::runtime_error("NO se pudo verificar los certificados SSL");
        }    
        
        cout << "Creando objeto BIO SSL" << endl;

        bio = BIO_new_ssl_connect(ctx);
        BIO_get_ssl(bio, & ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        BIO_set_conn_hostname(bio,cadenaCon);
        free(cadenaCon);

        if(BIO_do_connect(bio) <= 0)
        {
            cout << "Error al abrir conexion " << endl;
            // return 4;
            throw std::runtime_error("NO se pudo conectar con el servidor");
        }

        if(SSL_get_verify_result(ssl) != X509_V_OK)
        {
            cout << "Error al verificar el certificado" << endl;
            // return 5;
            throw std::runtime_error("NO se pudo verificar el resultado de la conexion SSL");
        }

        // return 0;
    }    
}

/**
 * Establece el timeout en segundos
*/
void GSocket::setTimeout( int segundos )
{
    int socket_fd = BIO_get_fd(bio, nullptr);
    if (socket_fd < 0) {
        std::cerr << "Error al obtener el descriptor de archivo del BIO." << std::endl;
        return;
    }

    struct timeval timeout;
    timeout.tv_sec = segundos;
    timeout.tv_usec = 0;

    // Configurar el timeout para operaciones de lectura
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // Configurar el timeout para operaciones de escritura
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}


/**
 * Escribe una cantidad de bytes en el socket 
 * 
 * buffer: posicion de memoria desde el que se escriben los bytes.
 * 
 * len : cantidad de bytes que se deben escribir o enviar
 * 
 * retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::write( char *buffer, long len )
{
    long enviados;

    if ( estaConectado() == 0 ) 
    {
        return;
    }

    int sockfd = BIO_get_fd(bio,BIO_NOCLOSE);

    while( 1 )
    {        
        // enviados = BIO_write(bio, buffer, len);
        enviados = send( sockfd, buffer, len, MSG_NOSIGNAL );
        if ( enviados <= 0 )
        {
            if(! BIO_should_retry(bio))
            {
                // cout << "Error al enviar datos al servidor " << endl;
                cerrar();

                // return -1;
                throw std::runtime_error("NO se pudo escribir los datos");
            }
        }
        else
        {
            len-= enviados;
            buffer+= enviados;
            if ( len <= 0 )
            {
                BIO_flush(bio);
                // cout << "Envio de datos finalizados" << endl;
                return;
            }
        }
    }  
}

/**
 * Envia una cadena de lenguaje C++, terminada en caracter nulo.
 * 
 * cadena: cadena que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeString( string cadena ) 
{
    writeChar((char *)cadena.c_str());
}


/**
 * Lee datos desde el socket.
 * 
 * buffer: posicion de memoria en la que se escriben los datos
 * 
 * maxLen : cantidad maxima de datos que se deben leer
 * 
 * retorna la cantidad de bytes leidos, un valor negativo en caso de error.
 */
long GSocket::read( char *buffer, long maxLen )
{
    if ( estaConectado() == 0 ) return -1;

    int sockfd = BIO_get_fd(bio,BIO_NOCLOSE);

    while(1 )
    {
        // long x = BIO_read(bio, buffer, maxLen);
        long x = recv(sockfd,buffer,maxLen,MSG_NOSIGNAL);
        // cout << "Respueeta de lectura socket :" << x << endl;
        if ( x == 0 )
        {
            return 0;
        }
        else 
        if(x < 0)
        {
            cout << "Respueeta de lectura socket :" << x << endl;
            if(! BIO_should_retry(bio))
            {
                // cout << "***** Error en retry " << endl;
                cerrar();
                return -1;
            }
        }
        else
        {
            return x;
        }
    }
}

/**
 * Lee N datos desde el socket.
 * 
 * buffer: posicion de memoria en la que se escriben los datos
 * 
 * len : cantidad de datos que se deben leer
 * 
 * retorna 0 si pudo leer los datos con exito, diferente a cero en caso de error.
 */
void GSocket::readLen( char *buffer, long len )
{
    long porLeer = len;
    long leido;

    while( 1 )
    {
        leido = read(buffer,porLeer);
        if ( leido == -1 )
        {
            // return -1;
            // cout << "No se pudo leer datos desde el servidor" << endl;
            
            throw std::runtime_error("NO se pudo leer datos desde el servidor");
        }
        if ( leido > 0 )
        {
            // cout << *buffer;            
            porLeer-= leido;
            buffer+= leido;

            // cout << "Datos leidos:" << leido << ", faltan leer " << porLeer << endl;

            if ( porLeer <= 0 )
            {
                // return 0;
                return;
            }
        }
        if ( leido == 0 )
        {            
            usleep(1000);
        }
    }   

    // return 0;
}


/**
 * Acepta un cliente que se ha conectado al servidor, en caso que no haya clientes se bloquea.
 * 
 * retorna una instancia de GSocket o NULL
 */
GSocket GSocket::aceptarCliente()
{
    GSocket rpta;    
    long rptaAcept;
    BIO *bioCliente;
    
    if ( conectado == 0 )
    {
        return rpta;
    }

    rptaAcept = BIO_do_accept(bio);
    if ( rptaAcept <= 0 )
    {
        // cout << "No se pudo aceptar la conexion del cliente" << endl;
        return rpta;
    }

    // cout << "Cliente conectado" << endl;

    bioCliente = BIO_pop(bio);

    BIO_set_flags(bioCliente,MSG_NOSIGNAL);

    // cout << "Bio del cliente obtenido" << endl;

    rpta.conectado = 1;
    rpta.esServidor = 0;
    rpta.ipServidor = ipServidor;
    rpta.puertoServidor = puertoServidor;
    rpta.bio = bioCliente;

    return rpta;
}


/**
 * Valida si el socket esta o no conectado
 * retorna 1 si esta conectado, 0 en caso que no esta conectado
 */ 
int GSocket::estaConectado()
{
    if ( conectado == 0 )
    {
        cout << "No esta Conectado el Socket server " << endl;
        return conectado;
    }

    if ( esServidor == 1 )
    {
        return conectado;
    }

    int socket_fd = BIO_get_fd(bio, nullptr);
    if (socket_fd < 0) return false;

    // Comprobar el estado del socket con una llamada a recv
    char buffer;
    int result = recv(socket_fd, &buffer, 1, MSG_PEEK | MSG_DONTWAIT);
    
    // cout << "Llamada a socket PEEK " << result << " ERR: " << errno << " >> " << EAGAIN << "," << EWOULDBLOCK << endl;

    if (result > 0) {
        // Datos disponibles, est� conectado        
        return 1;
    } else if (result == 0) {
        // La conexi�n fue cerrada limpiamente por el otro extremo
        conectado = 0;
        return 0;
    } else {
        // Verificar si el error es por una desconexi�n
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No hay datos ahora mismo, pero sigue conectado
            return 1;
        } else {
            // Ocurri� un error, posiblemente una desconexi�n
            conectado = 0;
            return 0;
        }
    }
}


/**
 * Cierra la conexion del socket o del servidor
 * 
 * retorna 0 en caso de exito u otro valor en caso de error
 */
void GSocket::cerrar()
{
    if ( conectado == 1 )
    {       
        conectado = 0;
        if ( esServidor == 0 )
        {
            cout << "Cerrando SOCKET cliente" << endl;
            if ( usarOpenSSL == 0 )
            {
                BIO_reset(bio);
                BIO_free_all(bio);
            }
            else
            {
                BIO_reset(bio);
                BIO_free_all(bio);
                SSL_CTX_free(ctx);
            }
        }
        else
        {
            // pendiente de validar
            cout << "Cerrando el servidor SOCKET" << endl;
            if ( usarOpenSSL == 0 )
            {
                BIO_reset(bio);
                BIO_free_all(bio);
            }
            else
            {
                BIO_reset(bio);
                BIO_free_all(bio);
                SSL_CTX_free(ctx);
            }
        }        
    }

    conectado = 0;

    // return 0;
}


/**
 * Envia un valor entero sin signo de 8 bits
 * 
 * valor: valor que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeUint8( uint8_t valor )
{
    write((char *)&valor,1);
}

/**
 * Envia un valor entero sin signo de 16 bits
 * 
 * valor: valor que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeUint16( uint16_t valor )
{
    unsigned char bytes[2];    

    bytes[1] = (valor >> 8) & 0xFF;
    bytes[0] = valor & 0xFF;

    write((char *)bytes,2);
}

/**
 * Envia un valor entero sin signo de 32 bits
 * 
 * valor: valor que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeUint32( uint16_t valor )
{
     unsigned char bytes[4];    

    bytes[3] = (valor >> 24) & 0xFF;
    bytes[2] = (valor >> 16) & 0xFF;
    bytes[1] = (valor >> 8) & 0xFF;
    bytes[0] = valor & 0xFF;

    write((char *)bytes,4);
}


/**
 * Envia un valor entero
 * 
 * valor: valor que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeInt( int valor )
{
    unsigned char bytes[4];    

    bytes[0] = (valor >> 24) & 0xFF;
    bytes[1] = (valor >> 16) & 0xFF;
    bytes[2] = (valor >> 8) & 0xFF;
    bytes[3] = valor & 0xFF;

    write((char *)bytes,4);
}

/**
 * Envia un valor long
 * 
 * valor: valor que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeLong( long long valor )
{
    unsigned char bytes[8];

    bytes[0] = (valor >> 56) & 0xFF;
    bytes[1] = (valor >> 48) & 0xFF;
    bytes[2] = (valor >> 40) & 0xFF;
    bytes[3] = (valor >> 32) & 0xFF;
    bytes[4] = (valor >> 24) & 0xFF;
    bytes[5] = (valor >> 16) & 0xFF;
    bytes[6] = (valor >> 8) & 0xFF;
    bytes[7] = valor & 0xFF;

    write((char *)bytes,8);
}

/**
 * Envia una cadena de lenguaje C, terminada en caracter nulo.
 * 
 * cadena: cadena que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeChar( char *cadena )
{
    int len = strlen(cadena);

    cout << "Enviando cadena: " << cadena << " ===> Tiene " << len << " caracteres " << endl;

    writeInt(len);
    write(cadena,len);
}


/**
 * Envia una cadena de lenguaje C, terminada en caracter nulo.
 * 
 * cadena: cadena que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeSmallChar( char *cadena )
{
    int len = strlen(cadena);

    cout << "Enviando cadena: " << cadena << " ===> Tiene " << len << " caracteres " << endl;

    writeUint8(len);
    write(cadena,len);
}


/**
 * Envia una cadena de lenguaje C, terminada en caracter nulo.
 * 
 * cadena: cadena que se envia
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
void GSocket::writeSmallString( string cadena )
{
    int len = cadena.length();

    cout << "Enviando cadena: " << cadena << " ===> Tiene " << len << " caracteres " << endl;

    writeUint16(len);
    write((char *)cadena.c_str(),len);
}


/**
 * Lee un valor entero sin signo de 32 bits desde el socket
 * 
 * valor : posicion de memoria en la que se lee el valor
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
uint32_t GSocket::readUint32()
{
    uint32_t rpta;
    unsigned char buffer[4];

    // cout << "Inicia lectura de entero, ojala se bloquee " << endl;

    readLen((char *)buffer,4);

    // cout << "Fin de lectura de bytes del entero :" << rpta << endl; 

    // if ( rpta != 0) return rpta;

    rpta = (uint32_t)buffer[3];
    rpta <<= 8;
    rpta |= (uint32_t)buffer[2];
    rpta <<= 8;
    rpta |= (uint32_t)buffer[1];
    rpta <<= 8;
    rpta |= (uint32_t)buffer[0];    

    return rpta;

    // *valor = rpta;

    // cout << "Entero leido: " << rpta << endl;

    // return 0;
}

/**
 * Lee un valor long desde el socket
 * 
 * valor : posicion de memoria en la que se lee el valor
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
long GSocket::readLong()
{
    long rpta;
    unsigned char buffer[4];

    readLen((char *)buffer,4);
    // if ( rpta != 0) return rpta;

    rpta = (long)buffer[0];
    rpta <<= 8;
    rpta |= (long)buffer[1];
    rpta <<= 8;
    rpta |= (long)buffer[2];
    rpta <<= 8;
    rpta |= (long)buffer[3];    
    rpta <<= 8;
    // rpta |= (long)buffer[4];    
    // rpta <<= 1;
    // rpta |= (long)buffer[5];    
    // rpta <<= 1;
    // rpta |= (long)buffer[6];    
    // rpta <<= 1;
    // rpta |= (long)buffer[7];    

    return rpta;

    //*valor = rpta;

    //return 0;
}

/**
 * Lee una cadena desde el socket.
 * Es responsabilidad del codigo que llamo a la funcion de liberar la memoria usada por el buffer
 * 
 * Retorna el buffer en el que se creo la cadena, NULL en caso de error.
 */
char *GSocket::readChar()
{
    uint32_t len;
    char *buffer;

    len = readUint32();

    buffer = (char *)malloc(len+1);
    if ( buffer == NULL ) return NULL;

    readLen(buffer,len);

    buffer[len] = 0;

    return buffer;
}

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
void GSocket::readChar( char *buffer, int maxLen )
{
    uint32_t len,lenLee;
    char valor;
    
    // lee la cantidad de bytes o caracteres de la cadena
    len = readUint32();
    
    // calcula la cantidad de bytes que se deben leer sin rebalsar el buffer
    if ( len >= maxLen )
    {
        lenLee = maxLen-1;
    }
    else
    {
        lenLee = len;
    }

    cout << "Leyendo cadena con " << lenLee << " caracteres" << endl;

    // lee la cadena 
    readLen(buffer,lenLee);
    // if ( rpta != 0 ) return rpta;

    buffer[lenLee] = 0;

    if ( lenLee < len ) return;

    // lee los bytes de la cadena enviada que ya no se pueden
    // guardar en el buffer pero se deben leer para liberar el socket
    while( lenLee < len )
    {
        readLen(&valor,1);    
        lenLee++;
    }
}

/**
 * Lee un valor entero sin signo de 16 bits desde el socket
 * 
 * valor : posicion de memoria en la que se lee el valor
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
uint16_t GSocket::readUint16()
{
    uint16_t rpta;
    unsigned char buffer[2];

    // cout << "Inicia lectura de entero, ojala se bloquee " << endl;

    readLen((char *)buffer,2);

    // cout << "Fin de lectura de bytes del entero :" << rpta << endl; 

    // if ( rpta != 0) return rpta;

    rpta = (uint16_t)buffer[1];
    rpta <<= 4;
    rpta |= (uint16_t)buffer[0];
    
    return rpta;
}

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
void GSocket::readSmallChar( char *buffer, int maxLen )
{
    
    uint16_t len,lenLee;
    char valor;
    
    // lee la cantidad de bytes o caracteres de la cadena
    len = readUint16();
    
    // calcula la cantidad de bytes que se deben leer sin rebalsar el buffer
    if ( len >= maxLen )
    {
        lenLee = maxLen-1;
    }
    else
    {
        lenLee = len;
    }
   
    read(buffer,lenLee);
    buffer[lenLee] = 0;

    while( lenLee < len )
    {
        readChar(&valor,1);
        lenLee++;
    }
}