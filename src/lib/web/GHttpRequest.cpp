
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
GHttpRequest::GHttpRequest( shared_ptr<HttpHeaderReader> reader, GSocket socket )
{
    headerReader = reader;
    socketIn = socket;

    #ifdef GOBJECT_VERBOSE
    cout << "Constructor : " << getType() << " ID: " << id_unico << endl;
    #endif

    // cout << "HEADER HTTP Leido: " << endl << headerReader->dataCabecera << endl;

    if ( headerReader->metodoReq.compare("POST") == 0 )
    {
        // cout << "Antes de leer data de post" << endl;
        leeDataPost();
    }    

    // cout << "Antes de leer data de querystring" << endl;
    leeDataQueryString();
}


/**
 * Destructor
 */
GHttpRequest::~GHttpRequest()
{
    #ifdef GOBJECT_VERBOSE
    cout << "Destructor:  " << getType() << " ID:" << id_unico << endl;
    #endif
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GHttpRequest::getType()
{
    return string("GHttpRequest");
}

/**
 * Retorna el URL invocado
 */
string GHttpRequest::getUrl()
{
    return headerReader->urlReq;
}


/**
 * Retorna el URL invocado
 */
string GHttpRequest::getQueryString()
{
    return headerReader->queryString;
}

/**
 * Retorna el metodo HTTP
 */
string GHttpRequest::getMetodo()
{
    return headerReader->metodoReq;
}

/**
 * Retorna la data pura recibida por el post
 * Empleada en caso no se haya enviado como un formularios
 */
string GHttpRequest::getPostData()
{    
    return parametros.getString("data");
}

/**
 * Retorna el valor de un parametro recibido
 * 
 * nombre : nombre del parametro cuyo valor se solicita
 */
string GHttpRequest::getParam( string nombre )
{
    if ( isParamArray(nombre) == false )
    {
        return parametros.getString(nombre);
    }
    else
    {
        shared_ptr<GVector>lstDatos = getParamArray(nombre);
        return lstDatos->getString(0);
    }
}

/**
 * Retorna el valor del parametro convertido a un numero entero
 * 
 * nombre : nombre del parametro cuyo valor se solicita
 * 
 * valErr : valor en caso no exista el parametros
 */
int GHttpRequest::getParamInt( string nombre, int valErr )
{
    string valor = getParam(nombre);

    if ( valor.length() == 0 ) return valErr;
    
    int rpta;
    try
    {
        rpta = stoi(valor);
    }
    catch(...)
    {
        rpta = valErr;
    }
    
    return rpta;
}

/**
 * Retorna el valor del parametro convertido a un numero entero
 * 
 * nombre : nombre del parametro cuyo valor se solicita
 * 
 * valErr : valor en caso no exista el parametros
 */
long GHttpRequest::getParamLong( string nombre, long valErr )
{
    string valor = getParam(nombre);
 
    if ( valor.length() == 0 ) return valErr;
    
    long long rpta;
    try
    {
        rpta = stoll(valor);
    }
    catch(...)
    {
        rpta = valErr;
    }
     
    return rpta;
    
    return rpta;
}


/**
 * Retorna el valor del parametro convertido a un numero double
 * 
 * nombre : nombre del parametro cuyo valor se solicita
 * 
 * valErr : valor en caso no exista el parametros
 */
double GHttpRequest::getParamDouble( string nombre, double valErr )
{
    string valor = getParam(nombre);

    if ( valor.length() == 0 ) return valErr;
    
    double rpta;
    try
    {
        rpta = stod(valor);
    }
    catch(...)
    {
        rpta = valErr;
    }
     
    return rpta;
}


/**
 * Valida si un parametro es un arreglo
 */
bool GHttpRequest::isParamArray( string nombre )
{
    if ( parametros.hasKey(nombre) == false ) return false;

    shared_ptr<GObject> valorParam = parametros.get(nombre);
    if ( valorParam->getType().compare("GVector") == 0 )
    {
        return true;
    }

    return false;
}

/**
 * Retorna un parametro como un vector cuyos elementos son instancias de GStringObject
 */
shared_ptr<GVector> GHttpRequest::getParamArray( string nombre )
{
    shared_ptr<GObject>ptr;
    ptr = parametros.get(nombre);

    if ( ptr == NULL ) return NULL;
    
    return dynamic_pointer_cast<GVector>(ptr);
}


/**
 * Lee los datos del query string y llena el HashMap de parametros
 */
void GHttpRequest::leeDataQueryString()
{
    GVector partes = GStringUtils::split(headerReader->queryString,"&");
    int i,n,posIgual;
    GStringObject *obj;
    string param,valor;

    n = partes.size();

    // cout << "EL query string tiene " << n << " Tokens" << endl;
    for(i=0;i<n;i++)
    {
        obj = (GStringObject *)partes.get(i).get();
        // cout << "Procesando el token : " << obj->valor << endl;
        posIgual = obj->valor.find("=");
        if ( posIgual >= 0 )
        {
            param = obj->valor.substr(0,posIgual);
            valor =  GStringUtils::decode_URLEncodedString(obj->valor.substr(posIgual+1));

            // cout << "Nombre: " << param << " Valor: " << valor << endl;
            agregaParametro(param,valor);
        } 
        else
        {
            // cout << "El token no tiene valor" << endl;            
            agregaParametro(obj->valor,"");
        }
    }
}


/**
 * Lee los datos enviados via POST y llena el HashMap de parametros
 */
void GHttpRequest::leeDataPost()
{
    string contentType = headerReader->getHeader("Content-Type");

    // cout << "Header Content-Type: " << contentType << endl;

    int pos = contentType.find(";");
    string tipoDato;

    if ( pos > 0 ) tipoDato.append(contentType.substr(0,pos));
    else tipoDato = contentType;

    // cout << "ContentType : " << tipoDato << endl;

    if ( tipoDato.compare("application/x-www-form-urlencoded") == 0 )
    {
        // cout << "Lee POST con formulario " << endl;
        leeDataPostForm();
    }
    else
    if (((( tipoDato.compare("application/json") == 0 ) || ( tipoDato.compare("text/json") == 0 )) ||
       (( tipoDato.compare("application/xml") == 0 ) || ( tipoDato.compare("text/xml") == 0 ))) ||
       ( tipoDato.compare("text/html") == 0 ) )
    {
        // cout << "Lee POST de texto simple " << endl;
        leeDataPostTexto();
    }
    else
    {
        // cout << "ContentType no soportado" << endl;
    }
    
}

/**
 * Lee data post enviado como una cadena de texto simple
 * Por lo general es un XML o JSON
 */
void GHttpRequest::leeDataPostTexto()
{
    long cantidadData = headerReader->getHeaderLong("Content-Length",0);

    if ( cantidadData == 0 )
    {
        return;
    }

    char *texto = (char *)malloc(cantidadData+1);
    socketIn.readLen(texto,cantidadData);
    texto[cantidadData] = 0;

    string data(texto);
    free(texto);

    parametros.putString("data",data);
}

/**
 * Lee data post enviado como un formulario
 */
void GHttpRequest::leeDataPostForm()
{  
    long cantidadData = headerReader->getHeaderLong("Content-Length",0);

    // cout << "Cantidad de data POST " << cantidadData << endl;
    if ( cantidadData == 0 )
    {
        return;
    }

    char *texto = (char *)malloc(cantidadData+1);
    socketIn.readLen(texto,cantidadData);
    texto[cantidadData] = 0;

    string data(texto);
    free(texto);

    GVector partes = GStringUtils::split(data,"&");
    int i,n,posIgual;
    GStringObject *obj;
    string param,valor;

    n = partes.size();

    // cout << "POST tiene " << n << " valores" << endl;

    for(i=0;i<n;i++)
    {
        obj = (GStringObject *)partes.get(i).get();
        posIgual = obj->valor.find("=");
        if ( posIgual >= 0 )
        {
            param = obj->valor.substr(0,posIgual);
            valor = obj->valor.substr(posIgual+1);

            param = GStringUtils::decode_URLEncodedString(param);
            valor = GStringUtils::decode_URLEncodedString(valor);
            agregaParametro(param,valor);            
        } 
        else
        {
            param = GStringUtils::decode_URLEncodedString(obj->valor);
            agregaParametro(param,"");
        }
    }

    
}


/**
 * Agrega un parametro o campo.
 * Detecta si el parametro esta repetido, y en ese caso lo convierte en un arreglo.
 * 
 * nombre: nombre del parametro
 * valor : valor del parametro
 */
void GHttpRequest::agregaParametro( string nombre, string valor )
{
    if ( parametros.hasKey(nombre) == false )
    {
        parametros.putString(nombre,valor);
    }
    else
    {
        shared_ptr<GVector> vector;
        shared_ptr<GObject> valorParam = parametros.get(nombre);

        if ( valorParam->getType().compare("GVector") == 0 )
        {
            // ya es un vector            
            vector = dynamic_pointer_cast<GVector>(valorParam);
        }
        else
        {
            // se tiene que convertir en vector
            vector = make_shared<GVector>();
            vector->add(valorParam);
        }
        vector->add(valor);
    }
}

/**
 * Retorna la cantidad de parametros que tiene la peticion
 */
int GHttpRequest::getParamCount()
{
    return parametros.getLstClaves().size();
}

/**
 * Retorna el nombre del parametro en unap osicion dada
 */
string GHttpRequest::getParamName( int index )
{
    return parametros.getLstClaves()[index];
}

/**
 * Retorna el valor de campo de la cabecera    
 */
string GHttpRequest::getHeader( string param )
{
    return headerReader->getHeader(param);
}

/**
 * Retorna el valor de campo de la cabecera convertido a numero entero
 * 
 */
long long GHttpRequest::getHeaderLong( string param, long long valError )
{
    return headerReader->getHeaderLong(param,valError);
}

/**
 * Retorna una sesion en base al nombre de un cookie que deberia tener el ID
 * de la sesion
 */
string GHttpRequest::getCookie( string cookie )
{
    string valores = headerReader->getHeader("Cookie");
    int posIni,posFin;

    posIni = valores.find(cookie);
    if ( posIni < 0 )
    {
        return "";
    }
    posFin = valores.find(";", posIni);
    if ( posFin < 0 )
    {
        posFin = valores.length();
    }
    
    string valor = valores.substr(posIni+cookie.length()+1, (posFin-posIni-cookie.length()));
    return valor;
}