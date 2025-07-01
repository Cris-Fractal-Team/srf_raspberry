

#include "lib/utils/GLog.h"
#include "lib/general/GDate.h"
#include "lib/utils/GStringUtils.h"

#include <iostream>

using namespace std;

/**
 * Modo de depuracion
 */
int GLog::mode = GLOG_MODE_DEBUG ;

/**
 * Archivo con el que trabaja el log
 */
std::ofstream GLog::logFile;

/**
 * Imprime un mensaje de log en modalidad debug
 *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
 *  message: mensaje que se envia al log
 */
void GLog::debug( string sessionId, string message)
{
    if ( GLog::mode <= GLOG_MODE_DEBUG )
    {
        string msg = GLog::getFormattedString("DEBUG",sessionId,message);
        GLog::writeLog(msg);
    }
}

/**
 * Imprime un mensaje de log en modalidad debug
 *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
 *  message: mensaje que se envia al log
 */
void GLog::info( string sessionId, string message)
{
    if ( GLog::mode <= GLOG_MODE_INFO )
    {
        string msg = GLog::getFormattedString("INFO",sessionId,message);
        GLog::writeLog(msg);
    }
}

/**
 * Imprime un mensaje de log en modalidad debug
 *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
 *  message: mensaje que se envia al log
 */
void GLog::error( string sessionId, string message)
{
    if ( GLog::mode <= GLOG_MODE_ERROR )
    {
        string msg = GLog::getFormattedString("ERROR",sessionId,message);
        GLog::writeLog(msg);
    }
}

/**
 * Retorna la cadena de texto formateada
 *  type: texto identificador del tipo de mensaje
 *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
 *  message: mensaje que se envia al log
 */
string GLog::getFormattedString( string type, string sessionId, string message )
{
    GVector lst = GStringUtils::split(message,"\n");
    GDate ahora;
    string rpta,cadena;
    int n = lst.size();

    for(int i=0; i<n; i++)
    {
        cadena = lst.getString(i);
        rpta.append("[");
        rpta.append(ahora.toString());
        rpta.append("]");
        rpta.append(" {");
        rpta.append(type);
        rpta.append("}");
        if ( sessionId.length() > 0 )
        {
            rpta.append(" (ID:");
            rpta.append(sessionId);
            rpta.append(")");
        }
        rpta.append(" ");
        rpta.append(cadena);
        rpta.append("\n");
    }

    return rpta;
}


/**
 * Escribe una cadena en el log, en la primera linea pone la hora
 * de la escritura.
 * En la segunda linea se muestra el mensaje como tal
 *  msg: mensaje que se escribe
 *  monstrarEnConsola: indica si se debe o no mostrar el mensaje en la consola de la aplicacion
 */
void GLog::writeSimple( string msg, bool monstrarEnConsola )
{
    GDate ahora;

    if ( monstrarEnConsola )
    {

        GLog::writeLog(ahora.toString());
        GLog::writeLog("\n");
        GLog::writeLog(msg);
        GLog::writeLog("\n");
    }
    else
    {
        if (GLog::logFile.is_open()) 
        {
            GLog::logFile << "[" << ahora.toString() << "]\n";        
            GLog::logFile.flush();
            GLog::logFile << msg << "\n";        
            GLog::logFile.flush();
        }
    }
}


/**
 * Escribe una cadena en el log
 *  msg: mensaje que se escribe
 */
void GLog::writeLog( string msg )
{
    cout << msg;

    if (GLog::logFile.is_open()) 
    {        
        GLog::logFile << msg;
        GLog::logFile.flush(); // Forzar escritura inmediata
    }
}

/**
 * Cierra el archivo
 */
void GLog::close()
{
    if (GLog::logFile.is_open()) 
    {
        GLog::logFile.close();
    }
}

/**
 * Abre el archivo de log
 *  path : ruta del archivo
 */
void GLog::open( string path )
{
    GLog::logFile.open(path.c_str(), std::ios::app);
    if (!GLog::logFile.is_open()) {
        std::cerr << "Error al abrir el archivo de log: " << path << std::endl;        
    }
}