

#ifndef _LECTOR_PARAMS_
#define _LECTOR_PARAMS_

#include "lib/general/GHashMap.h"

#include "memory"

/**
 * Lee un archivo de configuracion y todos los pametros los retorna como string.
 * 
 * El archivo de configuracion es un archivo de texto simple
 * donde los comentarios inician con # y se deben poner al inicio de una linea
 * 
 * Los parametros tienen el formato:
 * 
 *  nombre = valor
 * 
 * La libreria busca el caracter igual y todo lo que esta a su izquierda, es considerado 
 * el nombre de un parametro, al que se le hace un trim para eliminar los espacios 
 * a la izquierda y a la derecha.
 * 
 * Para los valores, no se eliminan los espacios, es decir poner el valor inmediatamente
 * despues del igual.
 * 
 * 
 * * convierteNombresLowerCase:  convierte todos los nombres de los parametros a lowercase para simplificar las busquedas
 * 
 */
shared_ptr<GHashMap> leeArchivoConfig(  string path, bool convierteNombresLowerCase = false );

/**
 * Guarda un hashmap como un archivo de texto de configuracion
 * 
 *  path: ruta del archivo que se crea o sobreescribe
 * 
 *  lstParams: hashmap con todos los parametros que se guardan
 * 
 */
void guardaArchivoConfig(string  path, shared_ptr<GHashMap>lstParams );

/**
 * Retorna todo el contenido de un archivo de texto
 * 
 * path:    ruta del archivo que se lee
 * 
 */
string leeArchivoTexto( string path );

/**
 * Retorna todo el contenido de un archivo binario.
 * Si el archivo no existe retorna un buffer vacio.
 */
std::vector<char>leeArchivoBinario( string path );


/**
 * Retonra la lista de nombres de todos los archivos de un subdirectorio
 */
std::vector<string>listArchivosDirectorio( string path );

#endif