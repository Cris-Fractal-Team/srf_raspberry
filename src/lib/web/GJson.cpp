
#include <string.h>
#include <string>
#include <iostream>

#include "lib/general/GHashMap.h"
#include "lib/general/GVector.h"
#include "lib/general/GObject.h"

#include "lib/web/GJson.h"

using namespace std;

int analizaObjeto( shared_ptr<GHashMap> mapa, char *txtBuffer, int *posActual );

/**
 * Dado el caracter del codigo escape de JSON retorna la letra que representa.
 * En caso no se reconozca se retorna una cadena en blanco
 */
string getCarEscape( char letra )
{
    string valorAtt;

    if ( letra == 'n' )
    {
        valorAtt.append("\n");                
    }
    else
    if ( letra == 'r' )
    {
        valorAtt.append("\r");                
    }
    else
    if ( letra == 't' )
    {
        valorAtt.append("\t");                
    }
    else
    if ( letra == '\\' )
    {
        valorAtt.append("\\");                
    }
    else
    if ( letra == '"' )
    {
        valorAtt.append("\"");                
    }

    return valorAtt;
}


/**
 * Inicia el analisis de un objeto JSON y guarda lo detectado
 * dentro del mapa.
 * 
 * 
 * mapa : objeto en el que se ponen los atributos
 * txtBuffer : cadena que se analiza
 * posActual : posicion desde la que se trabaja.
 * 
 * retorna 0 en caso de exito. Otro valor en caso de error
 * 1 Caracter no esperado 
 * 2 Fin no esperado
 */
int analizaArreglo( shared_ptr<GVector> arreglo, char *txtBuffer, int *posActual )
{
    shared_ptr<GHashMap> nuevoObjeto;
    shared_ptr<GVector> lstValores;
    // Posibles estados:
    // 1: Espera un acarter no nulo, que indica inicio de atributo de texto (comilla doble), valor numerico o signo, inicio de arreglo, inicio de objeto
    // 2: Espera comilla doble para final del valor de una cadena de texto.
    // 3: Espera un caracter no nulo, que indica inicio de atributo de texto (comilla doble), valor numerico o signo, inicio de arreglo, inicio de objeto, fin de arreglo
    // 4: Espera un caracter parte de un numero
    // 5: Espera un caracter escape dentro de un string
    char letra;
    int estado;    
    int len,rpta,posIniNumero;    
    string valorStr,valorEsc;
    bool puntoEncontrado = false;
    double valorNumerico;
    char letraSimple[2];

    letraSimple[1]=0;
    estado = 1;
    len = strlen(txtBuffer);
    while( *posActual < len )
    {
        // cout << "Antes de obetener sigueinte letra: " << *posActual << endl;
        letra = txtBuffer[*posActual];
        letraSimple[0] = letra;

        // cout << "analizaArreglo, estado: " << estado << " Procesa letra: " << letraSimple << endl;
        if (( estado == 1 ) || ( estado == 3 ))
        {
            if ((( letra != ' ' ) && ( letra != '\t' )) && (( letra != '\r' ) && ( letra != '\n' )))
            {
                if ( letra == '"' )
                {
                    // inicia un valor string
                    // cout << "Inicia un valor string " << endl;
                    estado = 2;
                    (*posActual)++;
                    valorStr.clear();
                }
                else
                if (( ( letra >= '0' ) && ( letra <= '9') ) || ( ( letra == '+' ) || ( letra == '-') ))
                {
                    // inicia un valor numerico
                    // cout << "Inicia un valor numerico " << endl;
                    estado = 4;
                    posIniNumero = *posActual;
                    (*posActual)++;
                    puntoEncontrado = false;                    
                }
                else
                if ( letra == '{' )
                {
                    // inicia un objeto
                    nuevoObjeto = make_shared<GHashMap>();
                    // cout << "Inicia un nuevo objeto " << endl;

                    // Inicia el valor de un objeto
                    (*posActual)++;
                    rpta = analizaObjeto(nuevoObjeto,txtBuffer,posActual);
                    if ( rpta != 0 )
                    {
                        return rpta;
                    }
                    else
                    {
                        // cout << "Se agrega un objeto al arreglo" << endl;
                        arreglo->add(nuevoObjeto);
                        estado = 3;
                    }
                }
                else
                if ( letra == '[' )
                {
                    // inicia un arreglo
                    // cout << "Inicia un nuevo arreglo " << endl;
                    (*posActual)++;
                    lstValores = make_shared<GVector>();
                    rpta = analizaArreglo(lstValores,txtBuffer,posActual);
                    if ( rpta != 0 )
                    {
                        return rpta;
                    }
                    else
                    {
                        // cout << "Se agrega un arreglo al arreglo" << endl;
                        arreglo->add(lstValores);
                        estado = 3;
                    }
                }
                else
                if (( letra == ']' ) && ( estado == 3 ))
                {
                    // fin del arreflo
                    (*posActual)++;
                    return 0;
                }
                else
                if (( letra == ',' ) && ( estado == 3 ))
                {
                    // inicio de un nuevo atributo
                    estado = 1;
                    (*posActual)++;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                (*posActual)++;
            }
        }
        else
        if ( estado == 2 )
        {
            // espera comilla para valor string
            if ( letra == '"' )
            {
                // fin del valor string
                // cout << "Fin del valor string :" <<  valorStr << endl;

                arreglo->add(make_shared<GStringObject>(valorStr));
                (*posActual)++;
                estado = 3;
            }
            else
            if ( letra == '\\')
            {
                (*posActual)++;
                estado = 5;
            }
            else
            {
                valorStr.append(letraSimple);
                (*posActual)++;
            }
        }
        else
        if ( estado == 4 )
        {
            // espera valor numerico
            if ( ( letra >= '0' ) && ( letra <= '9') ) 
            {
                (*posActual)++;
            }
            else
            if ( ( letra == '.' ) && ( puntoEncontrado == false ) )
            {
                (*posActual)++;
                puntoEncontrado = true;
            }
            else
            if (( ( letra == ' ') || ( letra == '\t') ) || ( ( letra == ',') || ( letra == ']') ))
            {
                // fin del numero
                valorStr = string(txtBuffer+posIniNumero,*posActual-posIniNumero);
                try
                {
                    valorNumerico = stod(valorStr);
                }
                catch(...)
                {
                    return 1;
                }
                arreglo->add(make_shared<GDoubleObject>(valorNumerico));
                (*posActual)++;

                if ( letra == ',') 
                {
                    estado = 1;
                }
                else
                if ( letra == ']')
                {
                    return 0;
                }
                else
                {
                    estado = 3;
                }                
            }
            else
            {
                return 1;
            }
        }
        else
        if ( estado == 5 )
        {
            valorEsc = getCarEscape(letra);
            if ( valorEsc.length() == 0 )
            {
                return 1;
            }
            valorStr.append(valorEsc);
            estado = 2;
            (*posActual)++;            
        }
    }

    return 2;
}

/**
 * Inicia el analisis de un objeto JSON y guarda lo detectado
 * dentro del mapa.
 * 
 * 
 * mapa : objeto en el que se ponen los atributos
 * txtBuffer : cadena que se analiza
 * posActual : posicion desde la que se trabaja.
 * 
 * retorna 0 en caso de exito. Otro valor en caso de error
 * 1 Caracter no esperado 
 * 2 Fin no esperado
 */
int analizaObjeto( shared_ptr<GHashMap> mapa, char *txtBuffer, int *posActual )
{
    // Posibles estados:    
    // 2: Espera comillas dobles para inicio del nombre de un atriburo
    // 3: Espera dos puntos (:)
    // 4: Espera caracter no blanco, para iniciar el valor de un atributo
    // 5: Espera una comilla doble para finalizar valor de atributo string
    // 6: Espera partes de un valor numerico
    // 7: Espera una letra para leer caracter especial
    // 8: Espera un caracter no blanco, para un nuevo atributo (caracter coma) o fin de objeto (caracter cerrar llave)
    int estado = 2;
    int len = strlen(txtBuffer);
    int numComillas = 0;
    int posIniNombre,rptaAnaliza;
    char letra;
    char cadenaSimple[2];
    double valorNum;
    bool puntoEncontrado;
    shared_ptr<GHashMap> nuevoObjeto;
    shared_ptr<GVector> arreglo;
    string nombreAtt,valorAtt,valorEsc;

    // cout << "Procesa Objeto, Longitud de la cadena " << len << endl;
    
    cadenaSimple[1] = 0;
    posIniNombre = -1;
    while( *posActual < len )
    {
        // cout << "Antes de obetener sigueinte letra: " << *posActual << endl;
        letra = txtBuffer[*posActual];

        cadenaSimple[0] = letra;
        // cout << "analizaObjecto -> Estado: " << estado << " Procesa letra: " << cadenaSimple << endl;
 
        if ( estado == 2 )
        {
            // Se espera una comilla, se trata del nombre de un atributo
            if ( letra == '"' )
            {              
                if ( numComillas == 0 )
                {
                    // cout << "Inicio de nombre de atributo" << endl;
                    posIniNombre = *posActual;
                    numComillas++;
                }
                else
                {
                    nombreAtt = string(txtBuffer+posIniNombre+1,*posActual-posIniNombre-1);
                    // cout << "Atributo encontrado:" << nombreAtt << endl;
                    estado = 3;               
                    numComillas=0;     
                }
                // cout << "Antes de incrementar puntero" << endl;
                (*posActual)++;
            }
            else
            if ((( letra == ' ' ) || ( letra == '\t' )) || (( letra == '\r' ) || ( letra != '\n' )))
            {
                (*posActual)++;
            }
            else
            {
                return 1;
            }
        }      
        else
        if ( estado == 3 )
        {
            // se esperan dos puntos (:), antes de los dos puntos se esperan caracteres blancos
            if ( letra == ':' )
            {
                // cout << "Inicio del valor de un atributo" << endl;
                estado = 4;
                (*posActual)++;
            }
            else
            if ((( letra != ' ' ) && ( letra != '\t' )) && (( letra != '\r' ) && ( letra != '\n' )))
            {
                // caracter invalidao
                return 1;
            }
            else
            {
                (*posActual)++;
            }
        }
        else
        if ( estado == 4 )
        {
            // se espera encontrar un caracer no blanco
            if ((( letra != ' ' ) && ( letra != '\t' )) && (( letra != '\r' ) && ( letra != '\n' )))
            {
                if ( letra == '"' )
                {
                    // es un atributo de texto
                    // // cout << "Inicia valor de texto" << endl;
                    estado = 5;
                    valorAtt.clear();
                    (*posActual)++;
                }
                else
                if (( ( letra >= '0' ) && ( letra <= '9') ) || ( ( letra == '+' ) || ( letra == '-') ))
                {
                    // es un atributo numerico
                    // cout << "Inicia valor numerico" << endl;
                    estado = 6;
                    posIniNombre = *posActual;
                    puntoEncontrado = false;
                    (*posActual)++;
                }
                else
                if ( letra == '[' )
                {
                    // es un atributo arreglo
                    // cout << "Inicia valoir arreglo" << endl;
                    (*posActual)++;
                    arreglo = make_shared<GVector>();
                    rptaAnaliza = analizaArreglo(arreglo,txtBuffer,posActual);
                    if ( rptaAnaliza != 0 )
                    {
                        return rptaAnaliza;
                    }   
                    else
                    {
                        // cout << "Guarda atributo arreglo: " << nombreAtt << " , con  " << arreglo->size() << " elementos" << endl;
                        mapa->put(nombreAtt,arreglo);
                    }
                    estado = 8;
                }
                else
                if ( letra == '{' )
                {
                    // es un atributo objeto
                    // cout << "Inicia valor objeto" << endl;
                    (*posActual)++;
                    nuevoObjeto = make_shared<GHashMap>();
                    rptaAnaliza = analizaObjeto(nuevoObjeto,txtBuffer,posActual);
                    if ( rptaAnaliza != 0 )
                    {
                        return rptaAnaliza;
                    }                    
                    else
                    {
                        // cout << "Guarda atributo objeto: " << nombreAtt << endl;
                        mapa->put(nombreAtt,nuevoObjeto);
                    }
                    estado = 8;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                (*posActual)++;
            }
        }
        else
        if ( estado == 5 )
        {
            // se espera comilla doble para finalizar el valor del atributo texto
            if ( letra == '\\' )
            {
                // es un codigo especial
                estado = 7;
                (*posActual)++;
            }
            else
            if ( letra == '"' )
            {
                // fin del valor de texto
                (*posActual)++;
                estado = 8;
                // cout << "Fin de atributo de texto: " << valorAtt << endl;
                mapa->putString(nombreAtt,valorAtt);
            }
            else
            {
                valorAtt.append(cadenaSimple);
                (*posActual)++;
            }
        }
        else
        if ( estado == 6 )
        {
            // se esta procesando un valor numerico
            if ( ( letra >= '0' ) && ( letra <= '9') )
            {
                (*posActual)++;
            }
            else
            if (( letra == '.' ) && ( puntoEncontrado == false ))
            {
                puntoEncontrado = true;
                (*posActual)++;
            }
            else
            if (( letra == ' ') || ( letra == '\t')) 
            {
                // fin del numero con un caracter blanco
                valorAtt = string(txtBuffer+posIniNombre,*posActual-posIniNombre);
                try
                {
                    valorNum = stod(valorAtt);
                }
                catch(...)
                {
                    return 1;
                }                
                // cout << "Fin de atributo numerico con fin objeto: " << valorAtt << " " << valorNum << endl;
                mapa->putDouble(nombreAtt,valorNum);
                estado = 8;
                (*posActual)++;
            }
            else
            if ( letra == ',') 
            {
                // fin del numero e indicador que toca un nuevo atributo
                valorAtt = string(txtBuffer+posIniNombre,*posActual-posIniNombre);
                try
                {
                    valorNum = stod(valorAtt);
                }
                catch(...)
                {
                    return 1;
                }
                // cout << "Fin de atributo numerico con coma: " << valorAtt << " " << valorNum << endl;                
                mapa->putDouble(nombreAtt,valorNum);
                estado = 2;
                (*posActual)++;
            }
            else
            if ( letra == '}' )
            {
                // fin del numero y fin de objeto
                valorAtt = string(txtBuffer+posIniNombre,*posActual-posIniNombre);
                try
                {
                    valorNum = stod(valorAtt);
                }
                catch(...)
                {
                    return 1;
                }
                // cout << "Fin de atributo numerico con fin objeto: " << valorAtt << " " << valorNum << endl;
                mapa->putDouble(nombreAtt,valorNum);
                (*posActual)++;
                return 0;
            }
            else
            {
                // caracter invalido para un numero
                return 1;
            }
        }
        else
        if ( estado == 7 )
        {
            valorEsc = getCarEscape(letra);
            if ( valorEsc.length() == 0 )
            {
                // caracter de escape invalido
                return 1;
            }
            valorAtt.append(valorEsc);
            (*posActual)++;
            estado = 5;
        }
        else
        if ( estado == 8 )
        {
            // espera caracter no blanco para inico para inicio de nuevo atributo con una coma o fin de objeto con llave
            if ((( letra != ' ' ) && ( letra != '\t' )) && (( letra != '\r' ) && ( letra != '\n' )))
            {
                if ( letra == ',' )
                {
                    // se espera el nombre de un nuevo atributo
                    // cout << "Se espera nuevo nombre de atributo " << endl;
                    estado = 2;
                    (*posActual)++;
                }
                else
                if ( letra == '}' )
                {
                    // fin del objeto
                    // cout << "Fin del objeto " << endl;
                    (*posActual)++;
                    return 0;
                }
                else
                {
                    // caracter invalido
                    return 1;
                }
            }
            else
            {
                (*posActual)++;
            }
        }
    }

    if ( estado == 4 )
    {
        // se esperaba un espacio en blanco
        return 0;
    }

    return 2;
}


/**
 * Dada una cadena en formato JSON la convierte en un hashmap.
 * Para obtener los valores de texto llamara a getString.
 * Para obtener los valores numeros llamara a getDouble.
 * Para obtener los valores que son arreglos llamara a geObject su valor es una instancia de GVector.
 * Para obtener los valores que son objetos llamar a getObject su valor es una instancia de GHashMap
 * 
 * txtJson: cadena que se analiza
 * errorCod: 0 en caso de exito, otro valor en caso de error ( 1: Caracter invalidao, 2: Final no esperado ) 
 * 
 */
shared_ptr<GHashMap> GJson::parse( string txtJson, int *errorCod )
{
    shared_ptr<GHashMap> rpta;
    
    int len = txtJson.length();
    int posActual,res;
    char *txtDatos = (char *)txtJson.c_str();
    char letra;

    posActual = 0;
    while( posActual < len )
    {
        letra = txtDatos[posActual];
        if ( letra == '{' )
        {            
            posActual++;
            res = analizaObjeto(rpta,txtDatos,&posActual);
            if ( res != 0 )
            {
                *errorCod = res;
                return rpta;
            }
        }
        else
        if ((( letra != ' ' ) && ( letra != '\t' )) && (( letra != '\r' ) && ( letra != '\n' )))
        {
            *errorCod = 1;
            return rpta;
        }        
        else
        {
            posActual++;
        }
    
    }

    *errorCod = 0;

    return rpta;
}