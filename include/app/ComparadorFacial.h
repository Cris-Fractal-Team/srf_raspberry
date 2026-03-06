
#ifndef _COMPARADOR_FACIAL_
#define _COMPARADOR_FACIAL_

#include <iostream>
#include <string>

#include "app/EmisorImagenesHTTP.h"
#include "lib/graphics/ImageSource.h"
#include "lib/general/GThread.h"
#include "lib/hailolib/hailo8l.h"
#include "lib/hailolib/DetectorCaraHaloScrfd.h"
#include "lib/hailolib/FaceRecHailo.h"
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/hailolib/IdentificadorPersonasHailo.h"

/**
 * Clase que compara rostros usando la configuracion 
 * del sistema
 */
class ComparadorFacial
{
	public:

        /**
         * Compara dos archivos que estan alineados frontalmente y tiene la resolucion
         * esperada por la red neuronal.
         * 
         *  pathCara1: ruta de la primera imagen
         * 
         *  pathCara2: ruta de la segunda imagen
         * 
         *  params: parametros leidos desde el archivo de configuracion de la aplicacion
         *
         * Retorna la similaridad entre las caras
         */
        float comparaSimple( string pathCara1, string pathCara2, shared_ptr<GHashMap> params );

        /**
         * Compara dos archivos que tienen una foto en la que hay una persona
         * se debe detectar el rostro en cada imagen y luego hacer la comparacion
         * 
         *  pathCara1: ruta de la primera imagen
         * 
         *  pathCara2: ruta de la segunda imagen
         * 
         *  params: parametros leidos desde el archivo de configuracion de la aplicacion
         *
         * Retorna la similaridad entre las caras
         */
        float comparaBusca( string pathCara1, string pathCara2, shared_ptr<GHashMap> params );

};


#endif