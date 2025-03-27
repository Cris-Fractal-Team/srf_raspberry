
#include "lib/web/HttpHeaderReader.h"


#include <malloc.h>       
#include <string.h>

#include <iostream>       
#include <typeinfo>       
#include <exception>      
#include <string>

using namespace std;

/**
 * Cantidad maxima de bytes de la cabecera
 */
#define TAM_MAX_CABECERA 8192


/**
 * Constructor
 */
HttpHeaderReader::HttpHeaderReader()
{
    dataCabecera = (char *)malloc(TAM_MAX_CABECERA+2);
}


/**
 * Destructor
 */
HttpHeaderReader::~HttpHeaderReader()
{
    if ( dataCabecera != NULL )
    {
        free(dataCabecera);    
        dataCabecera = NULL;
    }    
}


/**
 * Lee la cabecera HTTP
 * 
 * socket : socket desde el que se lee la respuesta
 * 
 * Genera una excepcion del tipo runtimeexception en caso de error
 */
void HttpHeaderReader::leeCabeceraHttp( GSocket socket )
{
    char *buffer;
    string msgError;
    int leidos,estado,dataTotalLeida;

    buffer = dataCabecera;
    estado = 0;
    dataTotalLeida = 0;

    // lee la cabecera
    while( 1 )
    {
        leidos = socket.read(buffer,1);
        if ( leidos == -1 ) 
        {   cout << "NO se pudo leer la cabecera" << endl;
            break;
        }
        
        // cout << (char)*buffer ;

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
        buffer++;
    }
    
    if ( estado == 4 ) 
    {
        buffer++;
        *buffer = 0;

        lenData = getHeaderLong("Contet-Length",0);

        return;
    }
    
    msgError.append("Cabecera HTTP incompleta");
    throw std::runtime_error(msgError.c_str());
}

/**
 * Retorna el valor de una cabecera
 * 
 * nombre : nombre de la cabecera que se busca
 * 
 * retorna el valor de la cabecera o cadena blanca en caso no exista
 */
string HttpHeaderReader::getHeader( string nombre )
{
    char *contLen;
    char *posFin;
    string nombreCab(nombre);
    string rpta;


    nombreCab.append(": ");
    contLen = strstr(dataCabecera,nombreCab.c_str());

    if ( contLen != NULL )
    {
        contLen+= nombreCab.size();
        posFin = strstr(contLen,"\r\n");
        if ( posFin == NULL) return string();

        *posFin = 0;
        rpta.append(contLen);
        *posFin = '\r';

        return rpta;
    }
    else
    {
        return rpta;
    }
}

/**
 * Retorna el valor de una cabecera convertido a long
 * 
 * nombre : nombre de la cabecera que se busca
 * 
 * errVal : valor que se debe retornar en caso no se pueda convertir a long
 * 
 * retorna el valor convertido a string, en caso no exista o no se pueda convertir a string retorna errVal
 */
long HttpHeaderReader::getHeaderLong( string nombre, long errVal )
{
    char *contLen;
    long valorLong;
    string nombreCab(nombre);


    nombreCab.append(": ");
    contLen = strstr(dataCabecera,nombreCab.c_str());

    // contLen = strstr(dataCabecera,"Content-Length: ");
    if ( contLen != NULL )
    {
        contLen+= nombreCab.size();
        sscanf(contLen,"%li",&valorLong);

        return valorLong;
    }
    else
    {
        return errVal;
    }
}


/**
 * Lee datos basicos desde la cabecera de una respuesta
 */
void HttpHeaderReader::leeDatosBasicosRpta()
{
    char *contLen;
    char *contEncoding;

    contLen = strstr(dataCabecera," ");
    if ( contLen == NULL )
    {
        return;
    }
    contLen++;
    contEncoding = strstr(contLen," ");
    if ( contEncoding == NULL )
    {
        return;
    }

    httpError = atoi(contLen);
    cout << "Codigo http " << httpError << endl;

    
}

/**
 * Lee datos basicos desde la cabecera de una peticion
 */
void HttpHeaderReader::leeDatosBasicosPeticion()
{
    char *finMetodo;
    char *finUrl;
    char *iniQuery;

    finMetodo = strstr(dataCabecera," ");
    if ( finMetodo == NULL )
    {
        return;
    }
    *finMetodo = 0;
    metodoReq.assign(dataCabecera);
    *finMetodo = ' ';

    finMetodo++;
    finUrl = strstr(finMetodo," HTTP/");
    if ( finUrl == NULL )
    {
        urlReq.assign("/");
        return;
    }

    *finUrl = 0;
    iniQuery = strstr(finMetodo,"?");
    if ( iniQuery != NULL )
    {
        // tiene query string
        *iniQuery = 0;
        urlReq.assign(finMetodo);
        *iniQuery = '?';

        iniQuery++;
        queryString.assign(iniQuery);
    }
    else
    {
        // no tiene query string        
        urlReq.assign(finMetodo);
    }
    *finUrl = ' ';
}