
#ifndef _DETECTION_TRACKER_
#define _DETECTION_TRACKER_

#include "lib/hailolib/FaceRecHailo.h"
#include "lib/general/GLinkedList.h"

/**
 * Representa el seguimiento que se hace a una deteccion
 */
class TrackedDetectionHailo
{
    public:

        /**
         * Deteccion a la que se le hace seguimiento
         */
        CaraDescrita cara;

        /**
         * Cantidad de ciclos o eventos de deteccion en los que no se encontro el objeto
         */
        int ciclosNoDetectados;         
        
        /**
         * ID unico del objeto detectado
         */
        long long id;

        /**
         * Constructor
         */
        TrackedDetectionHailo();
};

/**
 * Clase que controla objetos detectados por una camara
 * hace seguimiento a los objetos en base a las coordenadas entre
 * detecciones coninuas.
 */
class DetectionTrackerHailo
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
        GLinkedList<TrackedDetectionHailo> lstUniverso;

        /**
         * Vector con una lista temporal de objetos antes de hacer el tracking
         */
        GLinkedList<TrackedDetectionHailo> lstUniversoTemp;

        /**
         * Constructor
         */
        DetectionTrackerHailo();

        /**
         * Destructor
         */
        ~DetectionTrackerHailo();

        /**
         * Analiza las detecciones actualesy reconocidas, les asigna un ID unico
         * para poder hacer un tracking, pero principalmente para poder llevar
         * estadisticas del rostro y agrupar las detecciones 
         */
        vector<TrackedDetectionHailo *> analizaPorIdentificacion( vector<TrackedDetectionHailo *> *lstDetActuales );

        /**
         * Analiza las detecciones actuales y le asigna un ID a cada deeccion
         */
        vector<TrackedDetectionHailo *> analizaRapido( vector<DeteccionCaraHailo> *lstDetActuales );

        /**
         * Genera una lista de decciones a las que se les puede hacer tracking
         * las detecciones actuales y le asigna un ID a cada deeccion.
         */
        vector<TrackedDetectionHailo *> generaListaTrackTemporal( vector<DeteccionCaraHailo> *lstDetActuales );
        
        /**
         * ID del siguiente elemento que se agregara
         */
        long long sgteId;

    private:

        /**
         * Agrega una nueva deteccion al universo de detecciones
         */
        TrackedDetectionHailo agregaDeteccion ( DeteccionCaraHailo det );        

        /**
         * Agrega una nueva deteccion temporal
         */
        TrackedDetectionHailo agregaDeteccionTemporal ( DeteccionCaraHailo det );        
};



#endif