
#ifndef _FACEREC_HAILO_
#define _FACEREC_HAILO_

#include <string.h>
#include "hailo/hailort.hpp"
#include <iostream>
#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"
#include <opencv2/opencv.hpp>
#include "lib/hailolib/hailo8l.h"
#include "lib/hailolib/DetectorCaraHaloScrfd.h"
#include "lib/hailolib/FaceTypes.h"
#include "lib/graphics/GDibujo.h"

// Referencia simple para evitar referencias circulares
class TrackedDetectionHailo;

/**
 * Clase que brinda el servicio que genera descriptores faciales
 */
class FaceRecHailo
{
    public:

        /**
         * Indica si se presento un error en el calculo
         */
        bool errorCalculo;

        /**
         * Similaridad frontal maxima
         */
        float similaridadFrontal;

        /**
         * Runner empleado por la clase
         */
        Hailo8LRunner runner;

        /**
         * Tamaño maximo del batch para procesar inferencias en bloque
         */
        size_t maxBatchSize;

        /**
         * Nombre de la ultima capa del modelo de red neuronal
         */
        string nombreUltimaCapaModelo;

        /**
         * Factor para calcular el contraste
         */
        float paramContraste;

        /**
         * Indica si se debe visualizar el rostro original
         * y su version transformada en posicion frontal
         */
        bool previsualizaImgReconocimiento;

        /**
         * Indica si se debe guardar las caras frontales
         * alineadas, ellas se pueden usar para entrenamiento
         * o mejora en modelos de reconocimiento
         */
        bool guardarCarasFrontalesAlineadas;


        /**
         * Indica si se debe esperar la presion de una tecla
         * luego que se muestran las imagenes que
         * se envian para reconocimiento
         */
        bool esperarPrevImgReconocimiento;

        /**
         * Indica si se debe guardar 
         */
        bool guardarPrevImgReconocimiento;

        /**
         * Codigo de seguimiento
         */
        int secuencialImgReconocimiento;

        /**
         * Prefijo de las imagenes que se guardan en disco
         */
        string prefijoImgReconocimiento;

        /**
         * Constructor
         */
        FaceRecHailo();

        /**
         * Dimensiones de las imagenes que acepta la red
         */
        void setDimImagenes( int ancho, int altura );
        
        /**
         * Carga el modelo
         */
        int cargarModelo( string path );

        /**
         * Ejecuta la deteccion
         */
        bool ejecutar( cv::Mat imagen, DeteccionCaraHailo caraDet );

        /**
         * Ejecuta la deteccion
         */
        bool ejecutar( GLinkedList<cv::Mat> *lstImagenes, GLinkedList<DeteccionCaraHailo> *lstDetecciones );


        /**
         * Extrae el descriptor facial de la ultima inferencia
         */
        void extraeDescriptor( SIMD_TYPE *descriptor, int offset = 0 );

        /**
         * Calcula el descriptor facial para una cara
         * Retorna true en caso de exito, false en caso de error
         */
        bool calculaDescriptor( GImage imagen, DeteccionCaraHailo deteccion, SIMD_TYPE *descriptor );

        /**
         * Calcula el descriptor facial para una cara
         * Retorna true en caso de exito, false en caso de error
         */
        // bool calculaDescriptor( GLinkedList<GImage> *lstImagenes, GLinkedList<DeteccionCaraHailo> *lstDetecciones, GLinkedList<SIMD_TYPE *> *lstDescriptores );
        bool calculaDescriptor( GLinkedList<TrackedDetectionHailo *> *lstDeteccionesTrack );


        /**
         * Extrae el descriptor facial de la ultima inferencia en formato de OpenCV
         */
        cv::Mat extraeDescriptorMat();

        void configure(const std::string &serieEquipo, const std::string &nombreCapaSalidaRedFacial);
        
    private:

        /**
         * Lista de puntos de referencia
         */
        std::vector<cv::Point2f> lstPuntosReferencia;

        /**
         * Calcula la matriz de alineacion para un rostro detectado
         */
        cv::Mat calculaMatrizAlineacion( DeteccionCaraHailo deteccion );

        /**
         * Lista de indices de rostros que se han enviado a procesar
         * no todos los rostros se procesan porque algunos no estan cercanos
         * a ser frontales
         */
        GLinkedList<int>lstIndicesRostrosProc;

        /**
         * Ancho de las imagens aceptadas por el modelo
         */
        int imgModeloAncho;

        /**
         * Altura de las imagens aceptadas por el modelo
         */
        int imgModeloAltura;

        /**
         * Dada una cara detectada , la alinea para ejecutar el reconocimiento
         * facial, usando como dato los puntos de una deteccion previa
         */
        cv::Mat aliearRostro( cv::Mat fotoCara, DeteccionCaraHailo deteccion, cv::Mat transformacion );

        /**
         * Vector en el que se almacenan los datos de las fotos de caras
         */
        std::vector<uint8_t> input_buffer;

        /**
         * Indica si ya se reservo RAM en el vector para almacenar los datos 
         * de los rostros
         */
        bool vectorIniciado;

};

#endif
