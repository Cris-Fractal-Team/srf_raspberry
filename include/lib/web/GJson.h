#ifndef GJSON_H
#define GJSON_H

#include <string>

#include "lib/general/GHashMap.h"
#include "lib/general/GVector.h"
#include "lib/general/GObject.h"

using namespace std;

class GJson
{
    public:

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
        static shared_ptr<GHashMap> parse( string txtJson, int *errorCod );
};

#endif 
