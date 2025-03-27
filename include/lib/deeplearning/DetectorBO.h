#ifndef DETECTORBO_H
#define DETECTORBO_H

#include <stdio.h>
// #include <opencv2/opencv.hpp>
// #include <opencv2/dnn.hpp>
// #include <opencv2/highgui.hpp>
#include <fstream>
#include <iostream>
// #include <opencv2/core/ocl.hpp>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/model.h"
#include <cmath>

#include "DeteccionVO.h"
#include "lib/general/GVector.h"
#include "lib/graphics/GDibujo.h"
#include "lib/general/GLinkedList.h"

#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;


/**
 * Clase que representa un ancla de detecciones
 */
class AnclaDeteccion : public GObject
{
    public:

        float xcenter;
        float ycenter;
        float altura;
        float ancho;
};


/**
 * Clase que representa los parametros para generar una lista de anclas
 */
class ConfigGeneradorAnclas
{
    public:

        float input_size_width;

        float input_size_height;

        float min_scale;

        float max_scale;

        float anchor_offset_x;

        float anchor_offset_y;

        int num_layers;

        GVector strides;

        GVector aspectRatios;

        bool reduce_boxes_in_lowest_layers;

        float interpolated_scale_aspect_ratio;

        bool fixed_anchos_size;

        GVector feature_map_width;

        GVector featura_map_height;
};


/**
 * Clase que representa la base para detectores de objetos
 */
class DetectorObjetos
{
    public:

        /**
         * Constructor
         */
        DetectorObjetos();

        /**
         * Destructor
         */
        ~DetectorObjetos();
                

        /**
         * Etiquetas de los codigos de objetos detectados
         */
        GVector etiquetas;

        /**
         * Probabilidad o confianza de las predicciones
         */
        float confianza;

         /**
         * Ancho de la imagen original al que se traducen las detecciones
         */
        float anchoImgOrig;

        /**
         * Altura de la imagen original al que se traducen las detecciones
         */
        float alturaImgOrig;

        /**
         * Ancho de la imagen con la que trabaja el modelo
         */
        int anchoImgModelo;

        /**
         * Altura de la imagen con la que trabaja el modelo
         */
        int alturaImgModelo;

        /**
         * Carga el modelo
         * @param path ruta del modelo
         */
        virtual void cargaModelo( std::string path ) = 0;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @return lista de rectangulos detectados
         */
        // std::shared_ptr<GVector> detectar( Mat *imagen );
        virtual std::shared_ptr<GVector> detectar( GImage *imagen ) = 0;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @param numDet variable que se actualiza con la cantidad de detecciones
         * @return puntero a un arreglo de detecciones
         */
        virtual GLinkedList<DeteccionVO> detectarLst(GImage *imagen ) = 0;

        /**
         * Carga las etiquetas
         * @param path ruta de las etiquetas
         */
        bool cargaEtiquetas( std::string path );

        /**
         * Estable las dimensiones de la imagen original
         * @param ancho ancho en pixeles de la imagen original
         * @param altura altura en pixeles de la imagen original
         */
        void setDimImgOrig( int ancho, int altura);

        /**
         * Establece las dimensiones de la imagen que procesa el modelo
         * @param ancho ancho en pixeles de la imagen original
         * @param altura altura en pixeles de la imagen original
         * @param numBytesPorPixel cantidad de bytes que usa cada pixel de la imagen original
         */
        void setDimImgModelo( int ancho, int altura, int numBytesPorPixel  );

        /**
         * Establece el numero de threads con el que trabaja
         */
        virtual void setNumThreads( int numThreads );


        /**
         * Retorna un vector de anclas
         */
        void calculaAnclas( ConfigGeneradorAnclas opciones );

    protected:

       shared_ptr<GVector> lstAnclas;

        /**
         * Numero de threads con los que trabaja el detector
         */
        int numThreads;

        /**
         * Cantidad de bytes que tiene el buffer que almacena la imagen que procesara el modelo
         */
        long int numBytesBuffer;

        /**
         * Ancho minimo de las detecciones
         */
        int anchoMinDeteccion;

        /**
         * Ancho maximo de las detecciones
         */
        int anchoMaxDeteccion;

        /**
         * Altura minima de las detecciones
         */
        int alturaMinDeteccion;

        /**
         * altura maxima de las detecciones
         */
        int alturaMaxDeteccion;
};


/**
 * Detector basado en TensorFlow
 */
class DetectorTensorFlowLite : public DetectorObjetos
{
    public:

        /**
         * Constructor
         */
        DetectorTensorFlowLite();

        /**
         * Destructor
         */
        ~DetectorTensorFlowLite();

        /**
         * Carga el modelo
         * @param path ruta del modelo
         */
        void cargaModelo( std::string path ) override;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @return lista de rectangulos detectados
         */
        // std::shared_ptr<GVector> detectar( Mat *imagen );
        std::shared_ptr<GVector> detectar( GImage *imagen ) override;

         /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @param numDet variable que se actualiza con la cantidad de detecciones
         * @return puntero a un arreglo de detecciones
         */
        GLinkedList<DeteccionVO> detectarLst(GImage *imagen ) override;

         /**
         * Establece el numero de threads con el que trabaja
         */
        void setNumThreads( int numThreads ) override;

    protected:
        

        /**
         * Interprete de tensorflow
         */
        std::unique_ptr<tflite::Interpreter> interpreter;

        /**
         * Modelo con el que se trabaja
         */
        std::unique_ptr<tflite::FlatBufferModel> model;

};


/**
 * Detector de objetos que usa las clases antiguas de Haar
 */
class DetectorHaar : public DetectorObjetos
{
    public:

        /**
         * Constructor
         */
        DetectorHaar();

        /**
         * Destructor
         */
        ~DetectorHaar();

        /**
         * Carga el modelo
         * @param path ruta del modelo
         */
        void cargaModelo( std::string path ) override;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @return lista de rectangulos detectados
         */
        // std::shared_ptr<GVector> detectar( Mat *imagen );
        std::shared_ptr<GVector> detectar( GImage *imagen ) override;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @param numDet variable que se actualiza con la cantidad de detecciones
         * @return puntero a un arreglo de detecciones
         */
        GLinkedList<DeteccionVO>detectarLst(GImage *imagen ) override;

    private:

        cv::CascadeClassifier detector;

        /**
         * Factor de escala
         */
        double factorEscala;

        /**
         * NUmero de vecinos del detector
         */
        int numVecinos;

        /**
         * Escala horizontal de la imagen original vs la imagen del modelo
         */
        double escalaHorImgOrig;

        /**
         * Escala vertical de la imagen original vs la imagen del modelo
         */
        double escalaVerImgOrig;
};


/**
 * Detector de personas
 */
class BlazeFaceDetector : public DetectorTensorFlowLite
{
    public:

        /**
         * Constructor
         */
        BlazeFaceDetector();

        /**
         * Constructor
         */
        ~BlazeFaceDetector();


        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @return lista de rectangulos detectados
         */
        // std::shared_ptr<GVector> detectar( Mat *imagen );
        std::shared_ptr<GVector> detectar( GImage *imagen ) override;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @param numDet variable que se actualiza con la cantidad de detecciones
         * @return puntero a un arreglo de detecciones
         */
        GLinkedList<DeteccionVO>detectarLst(GImage *imagen ) override;

    private:

        float puntajeMinimo;

};


#endif // DETECTORBO_H
