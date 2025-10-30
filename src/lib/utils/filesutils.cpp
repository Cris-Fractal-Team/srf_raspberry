
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>

#include "lib/utils/fileutils.h"
#include "lib/utils/GStringUtils.h"

namespace fs = std::filesystem;

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
 * despues del igual
 * 
 * path:    ruta del archivo que se lee
 * 
 * convierteNombresLowerCase:  convierte todos los nombres de los parametros a lowercase para simplificar las busquedas
 *  
 * 
 */
shared_ptr<GHashMap> leeArchivoConfig(  string path, bool convierteNombresLowerCase )
{
    shared_ptr<GHashMap> lstParams;
    std::ifstream file(path);

    lstParams = make_shared<GHashMap>();

    if ( !file.is_open() )
    {
        cout << "No se pudo leer el archivo con rostros conocidos" << endl;
        return lstParams;
    }

    string line,nombre,valor;
    int pos;

    while( getline(file, line))
    {      
        pos = line.find("=");
        if ( pos < 0 ) continue;

        nombre = line.substr(0,pos);
        nombre = GStringUtils::trim(nombre);

        if ( nombre.at(0) == '#' ) continue;

        valor = line.substr(pos+1);
        if ( convierteNombresLowerCase == true )
        {
            nombre = GStringUtils::toLowerCase(nombre);
        }

        lstParams->putString(nombre,valor);
    }
    file.close();

    return lstParams;
}


/**
 * Retorna todo el contenido de un archivo de texto
 */
string leeArchivoTexto( string path )
{
    // Crear un flujo de archivo de entrada
    std::ifstream archivo(path);

    // Verificar si el archivo se pudo abrir
    if (!archivo.is_open()) {
        std::cerr << "Error al abrir el archivo: " << path << std::endl;
        return "";
    }

    // Usar un stringstream para almacenar el contenido del archivo
    std::stringstream buffer;
    buffer << archivo.rdbuf();  // Leer todo el archivo en el buffer

    // Cerrar el archivo
    archivo.close();

    // Retornar el contenido del buffer como string
    return buffer.str();

}

/**
 * Retorna todo el contenido de un archivo binario.
 * Si el archivo no existe retorna un buffer vacio.
 */
std::vector<char>leeArchivoBinario( string path )
{
    std::vector<char>buffer;

    // Crear un flujo de archivo de entrada
    std::ifstream archivo(path, std::ios::binary|std::ios::ate);

    // Verificar si el archivo se pudo abrir
    if (!archivo.is_open()) 
    {
        return buffer;
    }

    // obtiene el tamano del archivo
    std::streamsize fileSize = archivo.tellg();
    buffer.resize(fileSize);

    archivo.seekg(0, std::ios::beg);
    if ( !archivo.read(buffer.data(), fileSize) )
    {
        buffer.clear();
    }
    archivo.close();

    return buffer;
}


/**
 * Establece todo el contenido de un archivo de texto
 * 
 *  path:       ruta del archivo
 * 
 *  contenido:  contenido del archivo de texto
 */
void guardaArchivoTexto( string path, string contenido )
{
    std::ofstream archivo(path, std::ios::out);
    if ( archivo.is_open() )
    {
        archivo << contenido;
        archivo.close();
    }
}


/**
 * Guarda un hashmap como un archivo de texto de configuracion
 * 
 *  path: ruta del archivo que se crea o sobreescribe
 * 
 *  lstParams: hashmap con todos los parametros que se guardan
 * 
 */
void guardaArchivoConfig(string  path, shared_ptr<GHashMap>lstParams )
{
    string clave,valor,data;
    std::vector<string> lstClaves = lstParams->getLstClaves();
    int i,n;

    n = lstClaves.size();
    for(i=0;i<n;i++)
    {
        clave = lstClaves.at(i);
        valor = lstParams->getString(clave);
        data+= clave + "=" + valor + "\n";
    }

    guardaArchivoTexto(path, data);
}

/**
 * Retonra la lista de nombres de todos los archivos de un subdirectorio
 */
std::vector<string>listArchivosDirectorio( string path )
{
    fs::path dir(path);
    std::error_code ec;
    std::vector<string> lstArchivos;
    fs::directory_entry entry; 
    string rutaArchivo;

    if ( !fs::exists(dir) || !fs::is_directory(dir) )
        return lstArchivos;

    fs::directory_iterator dir_it(dir, fs::directory_options::skip_permission_denied), finLista;
    
    while( true )
    {        
        entry = *dir_it;
        if ( entry.is_directory() )
        {
            dir_it.increment(ec);
            if ( dir_it == finLista ) 
                break;
            continue;        
        }
            
        rutaArchivo = entry.path();
        lstArchivos.push_back(rutaArchivo);

        dir_it.increment(ec);
        if ( dir_it == finLista ) 
            break;
    }

    std::sort(lstArchivos.begin(), lstArchivos.end());


    return lstArchivos;
}
