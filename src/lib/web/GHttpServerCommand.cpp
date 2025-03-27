#include <iostream>


#include "lib/web/GSocket.h"
#include "lib/general/GObject.h"
#include "lib/web/GHttpServer.h"
#include "lib/web/GHttpServerCommand.h"
#include "lib/general/GVector.h"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/fileutils.h"

using namespace std;


/**
 * Nombre del cookie usado para contener el ID de las sesiones
 */
string GHttpServerCommand::idCookieSession = "_ID_SESSION_";

/**
 * Constructor
 */
GHttpServerCommand::GHttpServerCommand()
{
    sesionOblogatoria = false;

    #ifdef GOBJECT_VERBOSE
    cout << "Constructor Commandd : " << getType() << " ID: " << id_unico << endl;
    #endif 
}

/**
 * Destructor
 */
GHttpServerCommand::~GHttpServerCommand()
{
    #ifdef GOBJECT_VERBOSE
    cout << "Destructor:  " << getType() << " ID:" << id_unico << endl;
    #endif
}


/**
 * Retorna la ruta del command
 */
string GHttpServerCommand::getRuta()
{
    return rutaBase;
}

/**
 * Retorna la ruta raiz sin el comodin del command
 */
string GHttpServerCommand::getRutaRaiz()
{
    return rutaRaiz;
}


/**
 * Constructor
 * 
 * ruta: ruta que procesa del servidor se puede agregar al final un asterisco para indicar que
 *          procesa todos los URLs que inician con el texto anterior al asterisco
 * 
 * metodoHttp: puede ser GET, POST, GET_POST refiriendoce a los dos metodos
 */
void GHttpServerCommand::setRuta( string ruta, string metodoHttp )
{
    int pos;

    rutaBase = ruta;
    metodoProc = metodoHttp;

    pos = rutaBase.find_last_of("*");
    if (( pos > 0 ) && ( pos == (int)(rutaBase.length()-1)))
    {
        esUrlGenerico = true;
        rutaRaiz = rutaBase.substr(0,pos);
    }
    else
    {
        esUrlGenerico = false;
    }
}

/**
 * Valida si el comando procesa una ruta que se ha enviado al servidor.
 * 
 * ruta : ruta que se ha invocado
 * 
 * metodo : metodo HTTP que se empleo
 */
bool GHttpServerCommand::esProcesador( string ruta, string metodo )
{
    // cout << "GHttpServerCommando.esProcesador : " << ruta << " " << metodo << endl;

    // cout << "Validando con el comando " << rutaBase << " " << metodoProc << endl;
    
    if ( metodoProc.find(metodo) < 0 )
    {
        // cout << "El metodo no es soportado" << endl;
        return false;
    }

    if ( esUrlGenerico == true )
    {
        // cout << "EL URL es generico " << endl;

        if ( ruta.find(rutaRaiz) == 0 )
        {
            return true;
        }
    }
    else
    if ( ruta.compare(rutaBase) == 0 )
    {
        // cout << "EL URL es simple " << endl;
        return true;
    }

    // cout << "El URL no es soportado " << endl;

    return false;
}

/**
 * Procesa una peticion
 */
void GHttpServerCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion )
{    
}


/**
 * Dado un codigo HTML, lo analiza buscando secuencias del tipo <% valor %>
 * Por cada elemento de ese tipo, busca el valor en el hashmap lstValores y lo reemplaza,
 * si no existe una clave en el hashmap se retorna cadena vacia.
 * Esto ayuda a tener codigo HTML dinamico simple
 */
string GHttpServerCommand::reemplazaTagHtml( string html, shared_ptr<GHashMap> lstValores )
{
    int posIni,posFin,pos,n;
    string rpta,tag,valor;

    pos = 0;
    n = html.length();
    while( true )
    {
        if ( pos >= n )
        {
            break;
        }

        posIni = html.find("<%", pos);
        if ( posIni < 0 )
        {
            rpta.append(html.substr(pos));
            break;
        }
        posFin = html.find("%>", posIni);
        if ( posFin < 0 )
        {
            pos = posIni+2;
            rpta.append("<%");
            continue;
        }

        // se encontro un TAG especial
        tag = html.substr(posIni+2, (posFin-posIni)-2);
        tag = GStringUtils::trim(tag);

        // cout << "Buscando TAG para reemplazar :" << tag << endl;

        if ( lstValores->hasKey(tag) == false )
        {
            // cout << "No existe el TAG :" << tag << endl;
            valor = "";
        }
        else
        {            
            valor = lstValores->getString(tag);
            // cout << "Si existe el TAG para reemplazar :" << tag << " : " << valor << endl;
        }
        rpta.append(html.substr(pos,(posIni-pos)));
        rpta.append(valor);
        pos = posFin+2;
    }

    return rpta;
}


/**
 * Constructor
 */
GHttpStaticContentCommand::GHttpStaticContentCommand()
{

}

/**
 * Destructor
 */
GHttpStaticContentCommand::~GHttpStaticContentCommand()
{

}

/**
 * Lee los contenttypes desde un archivo
 */
void GHttpStaticContentCommand::cargaContentTypes( string path )
{
    lstContenTypes = leeArchivoConfig(path, true);
}



/**
 * Procesa una peticion
 */
void GHttpStaticContentCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion )
{
    string contentType;
    string path = request->getUrl();
    string rutaRelativa = path.substr(getRutaRaiz().length());    
    string rutaArchivo = pathBaseArchivos + rutaRelativa; 

    std::vector<char>buffer = leeArchivoBinario(rutaArchivo);

    cout << "Archivo leido:" << buffer.size() << endl;

    int pos = rutaArchivo.find_last_of(".");
    if ( pos < 0 )
    {
        contentType = "application/octet-stream";
    }
    else
    {
        string ext = rutaArchivo.substr(pos+1);
        ext = GStringUtils::toLowerCase(ext);
        contentType = lstContenTypes->getString(ext);
        if ( contentType.length() == 0 )
        {
            contentType = "application/octet-stream";
        }
    }

    cout << "Num Bytes del archivo:" << buffer.size() << endl;
    
    response->setBinaryResponse((void*)buffer.data(), buffer.size(), contentType);
}