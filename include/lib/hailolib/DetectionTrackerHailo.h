
#ifndef _DETECTION_TRACKER_
#define _DETECTION_TRACKER_

#include "lib/hailolib/CaraDescrita.h"
#include "lib/general/GLinkedList.h"
#include <opencv2/tracking.hpp>

/**
 * Representa la distancia entre dos rostros
 */
class DistanciaRostros
{
    public:

        /**
         * Distancia en coordenada X con respecto al centroide
         */
        int deltaX;

        /**
         * Distancia en coordinada Y con respecto al centroida
         */
        int deltaY;

        /**
         * Distancia en coordenada X con respecto al centroide
         */
        int deltaAncho;

        /**
         * Distancia en coordinada Y con respecto al centroida
         */
        int deltaAltura;


        /**
         * Diferencia facial
         */
        float deltaFacial;

        /**
         * Indica lista pendientes
         */
        int indicePendiente;
};


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
         * Puntero al objeto que se emplea para hacer tracking del objeto
         */
        cv::Ptr<cv::Tracker> tracker;

        /**
         * Recta en la que el tracker ha detectado al objeto
         */
        cv::Rect trackerBox;

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
         * Libera la RAM usada
         */
        void reset();

        /**
         * Analiza las detecciones actualesy reconocidas, les asigna un ID unico
         * para poder hacer un tracking, pero principalmente para poder llevar
         * estadisticas del rostro y agrupar las detecciones 
         */
        vector<TrackedDetectionHailo *> analizaPorIdentificacion( vector<TrackedDetectionHailo *> *lstDetActuales );

        /**
         * Analiza las detecciones actualesy reconocidas, les asigna un ID unico
         * para poder hacer un tracking, pero principalmente para poder llevar
         * estadisticas del rostro y agrupar las detecciones adicionalmente aplica un algoritmo
         * de tracking para poder identificar rostros que en la imagen anterior fueron identificados
         * pero en la actual no, pero existen rostro sin identificacion o anonimos cercanos.
         */
        vector<TrackedDetectionHailo *> analizaPorIdentificacionYTracker( vector<TrackedDetectionHailo *> *lstDetActuales, GImage *imagenVisorActual, GImage *imagenPreviaVisor, float factorXVisor, float factorYVisor );        

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
         * Analiza las detecciones nuevas anonimas, las compara con las detecciones universales que no tienen matching
         * busca los mas cercanos en distancia y comparacion facial, para finalmente aplicar un tracking y
         * decidir si hay o no una coincidencia
         */
        void analizaDetNuevasAnonimas( vector<TrackedDetectionHailo *> *lstTracActual, GLinkedList<TrackedDetectionHailo *> *lstUnivPen,
            GLinkedList<TrackedDetectionHailo *> *lstNuevos, GLinkedList<int>*lstUnivPenIndices, GImage *imagenVisorActual, GImage *imagenPreviaVisor, float factorXVisor, float factorYVisor );

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