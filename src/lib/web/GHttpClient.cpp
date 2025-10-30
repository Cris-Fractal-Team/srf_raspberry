#include "lib/web/GHttpClient.h"

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

#include <chrono>

#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>

using namespace std;

/**
 * Cantidad maxima de bytes de la cabecera
 */
#define TAM_MAX_CABECERA 8192

/**
 * Indica que la librerira SSL aun no fue inicializada 
 */
int GHttpClient::libInicializada = 0;

/**
 * Constructor
 */
GHttpClient::GHttpClient()
{
    dataCabecera = (char *)malloc(TAM_MAX_CABECERA+2);
    bufferResponse = NULL;
    bio = NULL;
    ctx = NULL;
}


/**
 * Destructor
 */
GHttpClient::~GHttpClient()
{
    free(dataCabecera);
    if ( bufferResponse != NULL )
    {
        free(bufferResponse);
    }
    cierraConexion();
}


/**
 * Hace un GET hacia un servidor
 * servidor: nombre o ip del servidor
 * puerto: puerto ip del servidor
 * usarSSL: indica si se debe (1) o no (0) usar SSL para encriptar los datos
 * metodo : GET o POST
 * url: ruta que se desea obtener
 * data : puntero para la data de POST
 * dataLen : longitud de datos en bytes
 * 
 * Retorn 0 en caso de exito, -1 en caso de error, -2 error de ram
 */
int GHttpClient::_doHttp( const char *servidor, int puerto, int usarSSL, const char* metodo, const char *url, char *data, long dataLen )
{
    int rptaCon;
    int post = 0;
    char *comando;

    ipServidor = (char *)servidor;
    puertoServidor = puerto;
    usarOpenSSL = usarSSL;

    httpError = -1;

    rptaCon = iniciaConexion();
    if ( rptaCon != 0 ) 
    {
        return rptaCon;
    }        

    comando = (char *)malloc(strlen(url)+strlen(servidor)+34);

    if ( usarSSL == 0 )
    {
        if ( puerto == 80 )
        {
            sprintf(comando,"%s %s HTTP/1.1\r\nHost: %s\r\n",metodo,url,servidor);
        }
        else
        {
            sprintf(comando,"%s %s HTTP/1.1\r\nHost: %s:%d\r\n",metodo,url,servidor,puerto);
        }
    }
    else
    {
        if ( puerto == 443 )
        {
            sprintf(comando,"%s %s HTTP/1.1\r\nHost: %s\r\n",metodo,url,servidor);
        }
        else
        {
            sprintf(comando,"%s %s HTTP/1.1\r\nHost: %s:%d\r\n",metodo,url,servidor,puerto);
        }
    }    
    
    rptaCon = writeData(comando,strlen(comando));    
    free(comando);
    if ( rptaCon != 0 )
    {
        cierraConexion();
        return rptaCon;
    }

    if ( strcmp(metodo,"POST") == 0 )
    {
        post = 1;
        cabeceraPeticion.append("Content-Length: ");
        cabeceraPeticion.append(to_string(dataLen));
        cabeceraPeticion.append("\r\n");

        if ( cabeceraPeticion.find("Content-Type") == string::npos )
        {
            cabeceraPeticion.append("Content-Type: application/x-www-form-urlencoded");
        }
    }

    auto enviaCabecera = std::chrono::high_resolution_clock::now();
    if ( cabeceraPeticion.length() > 0 )
    {
        rptaCon = writeData((char *)cabeceraPeticion.c_str(),cabeceraPeticion.length());
        if ( rptaCon != 0 )
        {
            cierraConexion();
            return rptaCon;
        }
    }

    writeData("\r\n");

    auto enviaCuerpo = std::chrono::high_resolution_clock::now();
    if ( post == 1 )
    {
        writeData(data,dataLen);
    }
    auto leeRespuesta = std::chrono::high_resolution_clock::now();

    if ( rptaCon == 0 )
    {
        leeRespuestaHttp();
    }
    auto finLeeRespuesta = std::chrono::high_resolution_clock::now();

    cierraConexion();    

    auto durCabecera = std::chrono::duration_cast<std::chrono::milliseconds>(enviaCuerpo - enviaCabecera);
    auto durCuerpo = std::chrono::duration_cast<std::chrono::milliseconds>(leeRespuesta - enviaCuerpo);
    auto durRespuesta = std::chrono::duration_cast<std::chrono::milliseconds>(finLeeRespuesta - leeRespuesta);

    // cout << "HTTP.T.Cabecera " << durCabecera.count() << " ms T.Cuerpo " << durCuerpo.count() << " ms T.Respuesta " << durRespuesta.count() << endl;

    return 0;
}

/**
 * Inicia la conexion con el servidor         
 */
int GHttpClient::iniciaConexion()
{
    char puertoStr[10];
    char *cadenaCon;

    sprintf(puertoStr,"%d",puertoServidor);

    cadenaCon = (char *)malloc(strlen(ipServidor)+strlen(puertoStr)+3);
    sprintf(cadenaCon,"%s:%s",ipServidor,puertoStr);

    if ( GHttpClient::libInicializada == 0 )
    {        
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
        GHttpClient::libInicializada = 1;
    }

    if ( usarOpenSSL == 0 )
    {
        // inicia conexion sin encriptado
        bio = BIO_new_connect(cadenaCon);
        free(cadenaCon);
        if(bio == NULL)
        {
            // cout << "Error al crear objeto de conexion " << endl;            
            return 1;
        }

        if(BIO_do_connect(bio) <= 0)
        {
            // cout << "Error al abrir conexion " << endl;
            cierraConexion();
            return 2;
        }

        return 0;
    }
    else
    {
        // inicia conexion con encriptado
        ctx = SSL_CTX_new(SSLv23_client_method());
        
        // cout << "Configuracion de ubicacion de certificados" << endl;
        if(! SSL_CTX_load_verify_locations(ctx, NULL, "/usr/lib/ssl/certs"))
        {
            cout << "Error al verificar la ubicacion de los certificados" << endl;
            free(cadenaCon);
            cierraConexion();
            return 3;
        }    
        
        // cout << "Creando objeto BIO SSL" << endl;

        bio = BIO_new_ssl_connect(ctx);
        BIO_get_ssl(bio, & ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        BIO_set_conn_hostname(bio,cadenaCon);
        free(cadenaCon);

        if(BIO_do_connect(bio) <= 0)
        {
            cout << "Error al abrir conexion " << endl;
            cierraConexion();
            return 4;
        }

        if(SSL_get_verify_result(ssl) != X509_V_OK)
        {
            cout << "Error al verificar el certificado" << endl;
            cierraConexion();
            return 5;
        }

        return 0;
    }
}

/**
 * Cierra la conexion
 */
void GHttpClient::cierraConexion()
{
    

    if ( usarOpenSSL == 0 )
    {
        if ( bio != NULL )
        {
            BIO_reset(bio);
            BIO_free_all(bio);
        }
    }
    else
    {
        if ( bio != NULL )
        {
            BIO_reset(bio);
            BIO_free_all(bio);
        }
        
        if ( ctx != NULL )
        {
            SSL_CTX_free(ctx);
        }            
    }

    bio = NULL;
    ctx = NULL;
}


/**
 * Escribe datos en el socket
 * 
 * buffer: datos que se escriben
 * len : cantidad de datos que se escriben
 * 
 * Retorna -1 en caso de error, 0 en caso de exito
 */
int GHttpClient::writeData( char *buffer, long len )
{
    long enviados;
    
    while( 1 )
    {
        enviados = BIO_write(bio, buffer, len);
        if ( enviados <= 0 )
        {
            if(! BIO_should_retry(bio))
            {
                // cout << "Error al enviar datos al servidor " << endl;
                // BIO_reset(bio);
                // BIO_free_all(bio);
                cierraConexion();
                return -1;
            }
        }
        else
        {
            len-= enviados;
            buffer+= enviados;
            if ( len <= 0 )
            {
                return 0;
            }
        }
    }        
}


/**
 * Envia los datos de un cadena.
 * Internamente llama a writeData con el puntero de datos del string
 * 
 * Retorna la cantidad de datos que se envio
 */ 
int GHttpClient::writeData( string cadena )
{
    return writeData((char *)cadena.c_str(),cadena.length());
}

/**
 * Lee datos desde el socket
 * 
 * buffer: buffer en el que se escriben los datos
 * maxLen: cantidad maxima de datos que se escriben
 * 
 * Retorna la cantidad de bytes que se leyeron, -1 indica fin de conexion
 */
long GHttpClient::readData( char *buffer, long maxLen )
{    
    while(1 )
    {
        long x = BIO_read(bio, buffer, maxLen);
        if ( x == 0 )
        {
            return -1;
        }
        else 
        if(x < 0)
        {
            if(! BIO_should_retry(bio))
            {
                cout << "***** Error en retry " << endl;
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
 * Lee la cabecera HTTP 
 *  Retorna 0 en caso de exito.
 * -1 Indica error al leer cabecera.
 * -3 Indica que no se encontro el content-length del request y/o Transfer-Encoding
 * -2 Indica error de RAM.
 */
int GHttpClient::leeRespuestaHttp()
{
    int rptaCab;    
    char *contLen;
    char *contEncoding;
    
    rptaCab = leeCabeceraHttp();
    if ( rptaCab != 0 )
    {
        return -1;
    }
    // cout << "Cabecera LEIDA" << endl;
    contLen = strstr(dataCabecera," ");
    if ( contLen == NULL )
    {
        return -1;
    }
    contLen++;
    contEncoding = strstr(contLen," ");
    if ( contEncoding == NULL )
    {
        return -1;
    }

    httpError = atoi(contLen);
    // cout << "Codigo http " << httpError << endl;

    if ( bufferResponse != NULL )
    {
        free(bufferResponse);
        bufferResponse = NULL;
    }

    contLen = strstr(dataCabecera,"Content-Length: ");
    if ( contLen != NULL )
    {
        contLen+= 16;
        sscanf(contLen,"%li",&lenData);
        // cout << "Content Length:" << lenData << endl;
        return leeDataContentLength();
    }
    else
    {
        contEncoding = strstr(dataCabecera,"Transfer-Encoding: ");
        if ( contEncoding == NULL )
        {
            return -3;
        }

        contEncoding+= 19;
        // cout << "Transferencia codificada" << endl ;
        return leeDataBloques();
    }
}


/**
 * Lee la cabecera HTTP
 * 
 * Retorna 0 en caso de exito, otro valor en caso de error
 */
int GHttpClient::leeCabeceraHttp()
{
    char *buffer;
    int leidos,estado,dataTotalLeida;

    buffer = dataCabecera;
    estado = 0;
    dataTotalLeida = 0;

    // lee la cabecera
    while( 1 )
    {
        leidos = readData(buffer,1);
        if ( leidos == -1 ) break;

        dataTotalLeida++;
        if ( dataTotalLeida == TAM_MAX_CABECERA ) 
        {
            estado = -1;
            break;
        }
        
        if ( *buffer == '\r' )
        {
            if ( estado == 0 )
            {
                estado = 1;
            }
            else
            if ( estado == 2 )
            {
                estado = 3;
            }
            else
            {
                estado = 0;
            }
        }
        else
        if ( *buffer == '\n' )
        {
            if ( estado == 1 )
            {
                estado = 2;
            }
            else
            if ( estado == 3 )
            {
                estado = 4;
                break;
            }
        }
        else
        {
            estado = 0;
        }
        // cout << *buffer;
        buffer++;
    }
    
    if ( estado == 4 ) 
    {
        buffer++;
        *buffer = 0;
        return 0;
    }
    

    return estado;
}


/**
 * Lee la data en base al content-length
 * Retorn 0 en caso de exito,-1 en caso de error de data , -2 en caso de falta de ram
 */
int GHttpClient::leeDataContentLength()
{
    int rpta;

    bufferResponse = (char *)malloc(lenData);
    if ( bufferResponse == NULL )
    {
        return -2;
    }

    rpta = readDataLen(bufferResponse,lenData);
    if ( rpta == -1 )
    {
        free(bufferResponse);
        bufferResponse = NULL;
    }

    return rpta;
}

/**
 * Lee la data en base a bloques
 * Retorn 0 en caso de exito,-1 en caso de error de data , -2 en caso de falta de ram
 */
int GHttpClient::leeDataBloques()
{
    long tamBloque;
    char *buffer;
    char finBloque[2];
    int rpta;

    lenData = 0;

    if ( bufferResponse != NULL )
        free(bufferResponse);

    bufferResponse = NULL;
    buffer = NULL;
    while(1)
    {
        tamBloque = leeTamDatosChunk();
        if ( tamBloque == -1 )
        {
            if ( bufferResponse != NULL )
            {
                free(bufferResponse);
                bufferResponse = NULL;
            }
            return -1;
        }           

        if ( tamBloque == 0 )
        {
            return 0;
        }

        bufferResponse = (char *)realloc(bufferResponse,lenData+tamBloque);

        if ( bufferResponse == NULL )
        {
            return -2;
        }

        if ( buffer == NULL )
        {
            buffer = bufferResponse;
        }
        else
        {
            buffer = bufferResponse+lenData;
        }

        // lee la data del bloque
        rpta = readDataLen(buffer,tamBloque);
        if ( rpta == -1 )
        {
            free(bufferResponse);
            bufferResponse = NULL;
            return -1;
        }

        // lee el salto de pagina como fin de bloque
        rpta = readDataLen(finBloque,2);
        if ( rpta == -1 )
        {
            free(bufferResponse);
            bufferResponse = NULL;
            return -1;
        }

        lenData+= tamBloque;
    }

    return 0;
}

/**
 * Lee los datos desde el socket hasta completar la lectura
 * de una cantidad de "len" bytes.
 * 
 * buffer: buffer en el que se escriben los datos
 * len : cantida de datos que se deben de leer.
 * 
 * Retorna 0 en caso de exito, -1 en caso de error
 */
int GHttpClient::readDataLen( char *buffer, long len )
{    
    long porLeer = len;
    long leido;

    while( 1 )
    {
        leido = readData(buffer,porLeer);
        if ( leido == -1 )
        {
            return -1;
        }
        if ( leido > 0 )
        {
            // cout << *buffer;

            porLeer-= leido;
            buffer+= leido;
            if ( porLeer <= 0 )
            {
                return 0;
            }
        }
    }   

    return 0;
}

/**
 * Lee la cantidad de bytes que el servidor ha enviado para el sigueinte bloque de datos (chunck)
 * 
 * Retorna la cantidad de datos que se deben leer.
 * En caso de error retorna -1
 */
long GHttpClient::leeTamDatosChunk()
{
    char tamDatos[20];
    int leidos;
    char *buffer;
    long tamdatos;

    buffer = &tamDatos[0];
    while(1)   
    {
        leidos = readDataLen(buffer,1);
        if ( leidos == -1 ) return -1;

        if ( *buffer == '\r' )
        {
            leidos = readDataLen(buffer,1);
            *buffer = 0;
            tamdatos = strtol(tamDatos,NULL,16);

            return tamdatos;
        }
        else
        {
            buffer++;
        }
    }
}

/**
 * Establece el valor de una cabecera para hacer una peticion
 * 
 * cabecera: nombre de la cabecera
 * valor : valor que se le asigna a la cabecera
 */
void GHttpClient::setHeader( string cabecera, string valor )
{
    cabeceraPeticion.append(cabecera);
    cabeceraPeticion.append(": ");
    cabeceraPeticion.append(valor);
    cabeceraPeticion.append("\r\n");
}

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
int GHttpClient::doHttp( string url, string method, char *data , long dataLen )
{
    size_t posFinProtocolo;
    size_t posIniPath;
    size_t posIniPuerto;

    int usarSSL;
    int puerto = 0;

    posFinProtocolo = url.find("://");
    if ( posFinProtocolo == string::npos )
    {
        return -100;
    }

    posIniPath = url.find("/",posFinProtocolo+3);
    posIniPuerto = url.find(":",posFinProtocolo+3);

    string protocolo = url.substr(0,posFinProtocolo);
    string strpuerto;
    string ip;
    string path;

    if ( posIniPath != string::npos ) path = url.substr(posIniPath);
    else path = "/";

    if ( protocolo.compare("https") == 0 )
    {
        usarSSL = 1;
        if ( posIniPuerto == string::npos ) 
        {
            puerto = 443;
        }      
    }
    else
    {
        usarSSL = 0;
        if ( posIniPuerto == string::npos ) 
        {
            puerto = 80;
        }
    }

    if ( puerto == 0 )
    {
        strpuerto = url.substr(posIniPuerto+1,posIniPath-posIniPuerto-1);
        puerto = stoi(strpuerto);
    }   

    if ( posIniPuerto != string::npos )
    {
        ip = url.substr(posFinProtocolo+3,posIniPuerto-posFinProtocolo-3);
    }
    else
    {
        if ( posIniPath != string::npos )
        {
            ip = url.substr(posFinProtocolo+3,posIniPath-posFinProtocolo-3);
        }
        else
        {
            ip = url.substr(posFinProtocolo+3);
        }
    }

    // cout << "URL: " << url << endl;
    // cout << "Servidor: " << ip << " Usarl SSL : " << usarSSL << " Puerto : " << puerto << " PATH : " << path << endl;

    if ( method.compare("POST") == 0 )
    {
        return _doHttp(ip.c_str(),puerto,usarSSL,method.c_str(),path.c_str(), data, dataLen);
    }
    else return _doHttp(ip.c_str(),puerto,usarSSL,method.c_str(),path.c_str());
}

/**
 * Retorna la respuesta convertida a string
 */
string GHttpClient::responseToStr()
{
    string rpta;

    rpta.reserve(lenData);
    rpta.append(bufferResponse,lenData);

    return rpta;
}