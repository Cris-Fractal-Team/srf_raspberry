
#ifndef _DETECTION_TRACKER_
#define _DETECTION_TRACKER_

#include "lib/general/GVector.h"
#include "lib/general/GLinkedList.h"
#include "lib/deeplearning/DeteccionVO.h"


/**
 * Representa el seguimiento que se hace a una deteccion
 */
class TrackedDetection : public GObject
{
    public:

        /**
         * Deteccion a la que se le hace seguimiento
         */
        DeteccionVO deteccion;

        /**
         * Cantidad de ciclos o eventos de deteccion en los que no se encontro el objeto
         */
        int ciclosNoDetectados;
        

        /**
         * Destructor
         */
        ~TrackedDetection();

};

/**
 * Clase que controla objetos detectados por una camara
 * hace seguimiento a los objetos en base a las coordenadas entre
 * detecciones coninuas.
 */
class DetectionTracker
{
    public:

        /**
         * Variacion maxima entre las coordenadas X de un objeto para que se pueda
         * considerar el mismo objeto detectado anteriormente
         * este es un factor del ancho de la deteccion mas angosta que se compara 
         */
        float deltaMaxX;

        /**
         * Variacion maxima entre las coordenadas X de un objeto para que se pueda
         * considerar el mismo objeto detectado anteriormente
         * este es un factor de la altura de la deteccion mas baja que se compara
         */
        float deltaMaxY;

        /**
         * Maximo de ciclos o eventos en los que no se encuentra un objeto
         * para ignorarlo
         */
        int maxCiclosInactivo;

        /**
         * Vector con la lista de objetos detectados validos
         */
        GLinkedList<TrackedDetection> lstUniverso;

        /**
         * Constructor
         */
        DetectionTracker();

        
        /**
         * Analiza las detecciones actuales y le asigna un ID a cada deeccion
         * 
         *  lstDetecActuales : lista con punteros a instancias de DeteccionVO
         * 
         *  numDetAct : numero de detecciones de lstDetecActuales
         * 
         *  numDet : cantidad de detecciones reetornadas
         */
        GLinkedList<DeteccionVO> analizaRapido( GLinkedList<DeteccionVO> lstDetActuales );

        /**
         * Analiza las detecciones actuales y le asigna un ID a cada deeccion.
         * Este usa una tecnica optimizada un poco lenta y consume mas memoria
         * 
         *  lstDetecActuales : lista con punteros a instancias de DeteccionVO
         */
        // void analizaOpt( std::shared_ptr<GVector> lstDetecActuales ); 

        /**
         * ID del siguiente elemento que se agregara
         */
        long long sgteId;

    private:

        /**
         * Agrega una nueva deteccion al universo de detecciones
         */
        void agregaDeteccion ( DeteccionVO *det );

        /**
         * Busca la deteccion mas cercana, considerando los deltas maximos en X e Y
         */
        // shared_ptr<TrackedDetection> buscaDeteccionMasCercana( shared_ptr<DeteccionVO> det, shared_ptr<GVector> lstTrac );

        /**
         * Retorna la lista de todos los TrackerDetection que podrian ser iguales
         * a una deteccion dada.
         * La lista esta ordenada de forma ascendente en base a la distancia hacia la deteccion
         * 
         * Retorna una lista de vectores con dos elementos cada uno:
         * 
         * 1ro: instancia de TrackerDetection
         * 2do: int distancia del TrackerDetecion a la deteccion pasada como parametro
         */
        // shared_ptr<GVector> calculaTrackersEquivalentes( shared_ptr<DeteccionVO> deteccion );

        /**
         * Busca en la deteccion mas cercana a un track.
         * Solo se consideran las detecciones que no tienen ID.
         * 
         *  track : tracking para el que se busca la deteccion mas cercana
         * 
         *  lst : vector que tiene una lista de vectores generados por calculaTrackersEquivalentes
         * 
         *  lstDetec : lista de detecciones, desde ella se retorna la instancia de deteccion
         *
         *  Retorna la deteccion mas cercana
         */
        // shared_ptr<DeteccionVO> calculaDetecionCercanoTracker( shared_ptr<TrackedDetection> track, shared_ptr<GVector> lst, shared_ptr<GVector> lstDetec );
};



#endif