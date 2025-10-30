
#ifndef _PREPROCEADOR_DATASET_
#define _PREPROCEADOR_DATASET_

#include <string>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <memory>
#include "lib/general/GHashMap.h"
#include "hailo/hailort.hpp"

using std::string;
using std::vector;
using std::shared_ptr;

/**
 * Brinda servicios para pre procesar un dataset
 * y crear un directorio 
 */
class PreProcesadorDataSet
{
    public:

        /**
         * Pre-procesa un directorio con imagenes capturadas
         * eliminando las imagenes que no tienen una cara luego 
         * de que se transformo.
         */
        void eliminaNoCaras( hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr, string path );

        /**
         * Analiza los archivos de un directorio donde cada archivo
         * contiene una cara, se supone que los debe ordener en orden
         * alfabetico de nombre y luego calcular el descriptor facial
         * y mientras pertenezcan a la misma persona agruparlos en un directorio
         * de un pathDestino
         */
        void agrupaCaras( hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr, string pathOrig, string pathDestino, float similaridadCoseno, shared_ptr<GHashMap> params );

    private:

        /**
         * Dada una lista de nombres de archivos que terminan en _indice.extesion
         * Calcula el indice o secuencia maxia
         */
        int buscaSecuenciaMaxima( vector<string>lstNombres );

        /**
         * Ordena un vector con una lista de nombres de archivos en base al sufijo
         * de secuancia que tiene, el formata del nombre es : _secuencia.extension
         * 
         *  param lstNombres: referencia a la lista que se desea ordenar
         * 
         *  param secMaxima: valor maximo de la secuencia
         * 
         * Retorna un vector con los nombres ordenados
         */
        vector<string>  ordenaPorSecuencia( vector<string> *lstNombres, int secMaxima );
};

#endif