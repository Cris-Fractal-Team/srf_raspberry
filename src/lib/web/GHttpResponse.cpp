
#include <iostream>
#include <sstream>
#include <string>

#include "lib/web/GSocket.h"
#include "lib/general/GObject.h"
#include "lib/web/GHttpServer.h"
#include "lib/general/GVector.h"
#include "lib/utils/GStringUtils.h"



using namespace std;



/**
 * Constructor
 */
GHttpResponse::GHttpResponse( GSocket socket )
{
    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << endl;
    #endif

    socketOut = socket;
    accion = HTTP_RESPONSE_ACCION_NONE;
    httpResponseCode = 0;

    cabecera.reserve(512);
}

/**
 * Destructor
 */
GHttpResponse::~GHttpResponse()
{
    #ifdef GOBJECT_VERBOSE
    cout << "Destructor:  " << getType() << " ID:" << id_unico << endl;
    #endif 
}


/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GHttpResponse::getType()
{
    return string("GHttpResponse");
}

/**
 * Agrega un valor de header
 * 
 * nombre: nombre de la cabecera
 * 
 * valor : valor que se le asigna a la cabecesra
 */
void GHttpResponse::addHeader( string nombre, string valor )
{
    cabecera.append(nombre);
    cabecera.append(": ");
    cabecera.append(valor);
    cabecera.append("\r\n");
}

/**
 * Establece la cantidad de datos que se envian
 */
void GHttpResponse::setContentLength( long len )
{
    addHeader("Content-Length",to_string(len));
}

/**
 * Establece el tipo de datos que se realiza
 */
void GHttpResponse::setContentType( string tipo )
{
    addHeader("Content-Type",tipo);
}

/**
 * Establece el codigo HTTP de respuesta
 */
void GHttpResponse::setHttpCode( int codigo )
{
    httpResponseCode = codigo;
}

/**
 * Establece una respuesta del tipo texto.
 * Para un caso exitoso de HTTP.
 * 
 * rpta: texto de la respuesta
 * 
 * contentType: formato del texto de la respuesta
 */
void GHttpResponse::setTextResponse( string rpta, string contentType )
{
    if ( httpResponseCode  == 0) httpResponseCode = 200;

    string codigoRpta("HTTP/1.1 ");

    codigoRpta.append(to_string(httpResponseCode));
    codigoRpta.append(" OK\r\n");

    setContentLength(rpta.length());
    setContentType(contentType);        
    cabecera.append("\r\n");    

    // cout << "GHttpResponse : ENVIANDO Respuests TEXTO : " << endl << codigoRpta << endl;

    socketOut.write((char *)codigoRpta.c_str(),codigoRpta.length());
    socketOut.write((char *)cabecera.c_str(),cabecera.length());
    socketOut.write((char *)rpta.c_str(),rpta.length());

    socketOut.cerrar();

    accion = HTTP_RESPONSE_ACCION_PROCESSED;

}

/**
    * Establece una respuesta exitosa del tipo binario.
    * Es responsabilidad del metodo que invoca de liberar la RAM
    * 
    * buffer: puntero a la estructura que tienel os bytes que se deben enviar
    * bufferLen : cantidad de bytes que se deben enviar
    * contentType : tipo de datos binarios enviados
    */
void GHttpResponse::setBinaryResponse( void *buffer, long bufferLen, string contentType )
{
    if ( httpResponseCode  == 0) httpResponseCode = 200;

    string codigoRpta("HTTP/1.1 ");

    codigoRpta.append(to_string(httpResponseCode));
    codigoRpta.append(" OK\r\n");

    setContentLength(bufferLen);   
    setContentType(contentType); 
    cabecera.append("\r\n");

    socketOut.write((char *)codigoRpta.c_str(),codigoRpta.length());
    socketOut.write((char *)cabecera.c_str(),cabecera.length());

    if ( bufferLen > 0 )
    {
        socketOut.write((char *)buffer,bufferLen);
    }
    
    socketOut.cerrar();

    accion = HTTP_RESPONSE_ACCION_PROCESSED;
}

/**
    * Establece una respuesta exitosa del tipo HTML
    */
void GHttpResponse::setHtmlResponse( string rpta )
{
    setTextResponse(rpta,"text/html");   
}

/**
    * Establece una respuesta exitosa del tipo Json
    */
void GHttpResponse::setJsonResponse( string rpta )
{
    setTextResponse(rpta,"application/json");
}

/**
* Establece una respuesta exitosa del tipo HTML
*/
void GHttpResponse::setXmlResponse( string rpta )
{
    setTextResponse(rpta,"text/xml");
}


/**
 * Envia una respuesta personalizada
 * 
 * responseCode: primera linea de la respuesta HTTP
 * 
 * buffer: datos que se envian luego 
 */
void GHttpResponse::setCustomResponse( string resposeCode, void *buffer, int bufferLen )
{
    if ( bufferLen > 0 )
    {
        setContentLength(bufferLen);   
    }    
    cabecera.append("\r\n");

    socketOut.write((char *)resposeCode.c_str(),resposeCode.length());
    socketOut.write((char *)cabecera.c_str(),cabecera.length());

    if ( bufferLen > 0 )
    {
        socketOut.write((char *)buffer,bufferLen);
    }
        
    // socketOut.cerrar();

    accion = HTTP_RESPONSE_ACCION_PROCESSED;
}

/**
    * Agrega un cookie
    * 
    * nombre : nombre del cookie
    * valor : valor del cookie
    * path : ruta base para la cual el cookie es valido
    * duracion : cantidad de segundos que dura el cookie. -1 indica no enviar
    */
void GHttpResponse::addCookie( string nombre, string valor, string path = "/", long duracion = -1 )
{
    string valorHeader(nombre);

    valorHeader.append("=");
    valorHeader.append(valor);
    valorHeader.append(";");

    valorHeader.append(" Path=");
    valorHeader.append(path);
    valorHeader.append(";");

    if ( duracion >= 0 )
    {
        valorHeader.append(" Max-Age=");
        valorHeader.append(to_string(duracion));
        valorHeader.append(";");
    }

    addHeader("Set-Cookie",valorHeader);
}

/**
 * Retorna la accion
 */
int GHttpResponse::getAccion()
{
    return accion;
}

/**
 * Retorna el socket asociado
 */
GSocket GHttpResponse::getSocket()
{
    return socketOut;
}