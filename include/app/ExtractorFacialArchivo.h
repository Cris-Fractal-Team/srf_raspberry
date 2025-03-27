

#ifndef _EXTRACTOR_FACIAL_ARCHIVO_
#define _EXTRACTOR_FAICL_ARCHIVO_

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
 * Clase que procesa un archivo de datos y un directorio con fotos para 
 * generar un archivo final que representa una BD de reconocimiento.ADJ_OFFSET_SINGLESHOT
 * 
 * El archivo de datos es un CSV:
 * 
 *      idPersona, nombre, pathFoto
 * 
 * idPersona : ID en un servicio externo con el que se identifica a una persona
 * nombre : Nombre de la persona con el idPersona
 * pathFoto : ruta del archivo JPEG con la foto, que debera estar dentro del directorio de fotos
 * 
 * Al final se genera un archivo con al BD de reconocimiento que tiene el siguiente formato: 
 * 
 * nombrePersona, idPersona, lista valores descriptor facial
 * 
 */
class ExtractorFacialArchivo
{
    public:

        /**
         * Ruta del archivo de da
         */
        std::string pathArchivoDatos;

        /**
         * Nombre del directorio en el que estan las fotos
         */
        std::string pathFotos;

        /**
         * Ruta del archivo con la BD de personas conocidas
         */
        std::string pathArchivoBD;

        /**
         * Lista de parametros que se configuran a nivel del
         * app web de la aplicacion, es decir son variables
         */
        shared_ptr<GHashMap>lstParamsApp;

        /**
         * Indica si se debe o no invertira verticalmente las imagenes
         */
        bool invertirVertical;
        
        /**
         * Ancho minimo de un rostro para ser procesado
         */
        int anchoRostroMinimo;

        /**
         * Altura minima de un rostro para ser procesada
         */
        int alturaRostroMinima;       

        /**
         * Ancho con el que se procesa la deteccion de un rostro
         */
        int anchoRostroDet;

        /**
         * Altura con la que se procesa la deteccion de un rostro
         */
        int alturaRostroDet;

        /**
         * Resolucion horizontal de la camara
         */
        int anchoCamara;

        /**
         * Altura de la camara
         */
        int alturaCamara;
       
        /**
         * Distancia maxima entre dos imagenes para considerarlas
         * de la misma persona
         */
        float toleranciaIden;

        /**
         * Tolerancia o acuracy para las detecciones
         */
        float toleranciaDetec;   
               
        /**
         * Ejecuta el proceso
         */
        void ejecutar();
};

#endif 