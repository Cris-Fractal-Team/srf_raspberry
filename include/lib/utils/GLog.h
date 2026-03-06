
#include "lib/general/GVector.h"
#include <string>
#include <string>
#include <fstream>

#ifndef GLOGGIN_H
#define GLOGGIN_H

using namespace std;


/**
 * Codigos de los modos de loggin
 */
#define GLOG_MODE_DEBUG 1
#define GLOG_MODE_INFO 2
#define GLOG_MODE_ERROR 3


/**
 * Clase que brinda servicios de loggs
 */
class GLog
{
    public:

        /**
         * Modo de depuracion
         */
        static int mode;

        /**
         * Imprime un mensaje de log en modalidad debug
         *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
         *  message: mensaje que se envia al log
         */
        static void debug( string sessionId, string message);

        /**
         * Imprime un mensaje de log en modalidad debug
         *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
         *  message: mensaje que se envia al log
         */
        static void info( string sessionId, string message);

        /**
         * Imprime un mensaje de log en modalidad debug
         *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
         *  message: mensaje que se envia al log
         */
        static void error( string sessionId, string message);

        /**
         * Cierra el archivo
         */
        static void close();

        /**
         * Abre el archivo de log
         *  path : ruta del archivo
         */
        static void open( string path );

        /**
         * Escribe una cadena en el log, en la primera linea pone la hora
         * de la escritura.
         * En la segunda linea se muestra el mensaje como tal
         *  msg: mensaje que se escribe
         *  monstrarEnConsola: indica si se debe o no mostrar el mensaje en la consola de la aplicacion
         */
        static void writeSimple( string msg, bool mostrarEnConsola );

    private:

        /**
         * Escribe una cadena en el log.
         * Es usada por todas las otras llamadas
         *  msg: mensaje que se escribe
         */
        static void writeLog( string msg );

        /**
         * Retorna la cadena de texto formateada.
         * Es llamada por todas las otras funciones para dar un formato uniforme a todos los mensajes.
         * Adicionalmente garantiza que todas las filas tengan un mensaje.
         *  type: texto identificador del tipo de mensaje
         *  sessionID: es un prefijo que se pone a cada linea del mensaje para poder filtrar 
         *  message: mensaje que se envia al log
         */
        static string getFormattedString( string type, string sessionId, string message );

        
        /**
         * Archivo en el que se escribe
         */
        static std::ofstream logFile;
};

#endif