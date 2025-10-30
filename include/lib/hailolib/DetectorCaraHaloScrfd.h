
#include <string.h>
#include "hailo/hailort.hpp"
#include <iostream>
#include "lib/general/GThread.h"
#include "lib/graphics/GDibujo.h"
#include <opencv2/opencv.hpp>
#include "lib/hailolib/hailo8l.h"

#ifndef _DETECTOR_CARAS_HAILO_SCRFD_
#define _DETECTOR_CARAS_HAILO_SCRFD_

#define _DetectorCarasHailoSCRFD_500m_ 1
#define _DetectorCarasHailoSCRFD_2_5g_ 2
#define _DetectorCarasHailoSCRFD_10g_ 3

using namespace std;

/**
 * Representa un punto
 */
class PuntoHailo
{
    public:
        
        float x;

        float y;

};


/**
 * Detecion de una cara
 */
class DeteccionCaraHailo
{
    public:

        /**
         * Coordenada de la esquina superior izquierda donde
         * se detecto el rostro
         */
        PuntoHailo ptoSupIzq;

        /**
         * Coordenada de la esquina inferior derecha donde
         * se tecto el rostro
         */
        PuntoHailo ptoInfDer;

        /**
         * Confidencia de la deteccion
         */
        float confidencia;

        /**
         * Clase de la deteccion
         */
        int clase;

        /**
         * Coordenada del ojo izquierdo
         */
        PuntoHailo ojoIzq;

        /**
         * Coordenada del ojo derecho
         */
        PuntoHailo ojoDer;

        /**
         * Coordenada de la nariz
         */
        PuntoHailo nariz;

        /**
         * Coordenada izquierda de la boca
         */
        PuntoHailo bocaIzq;

        /**
         * Coordenada derecha de la boca
         */
        PuntoHailo bocaDer;

        /**
         * Retorna el acho de la cara
         */
        int getAncho();

        /**
         * Retorna la altura de la cara
         */
        int getAltura();

        /**
         * Area del a deteccion
         */
        int getArea();

        /**
         * Agrega un valor a las coordenadas x y otro a las coordenadas y
         * de todos los puntos de la cara
         */
        void desplazaPtosCara( float deltaX, float deltaY );

        /**
         * Agrega un valor a las coordenadas x y otro a las coordenadas y
         * de los puntos de la region de detecion
         */
        void desplazaRegion( float deltaX, float deltaY );

        /**
         * Retorna el punto central del area detectada
         */
        PuntoHailo getCentro();

        /**
         * Valida si un punto (x,y) esta dentro de la deteccion
         */
        bool contienePunto( int x, int y );

        /**
         * Calcula la intereccion sobre la union de dos detecciones
         */
        float calcularIoU( DeteccionCaraHailo *det );

        /**
         * Ajusta las cooredanas de la deteccion para que sea un cuadrado
         * tomando como base la dimension mayor del rostro
         * 
         *  param anchoMax : ancho de la imagen en la que se hizo la deteccion
         * 
         *  param alturaMax : altura de la imagen en la que se hizo la deteccion
         */
        void ajustarCuadrado( int anchoMax, int alturaMax );

        /**
         * Ajusta las cooredanas de la deteccion para que sea un cuadrado
         * tomando como base la dimension minima del rostro
         * 
         *  param anchoMax : ancho de la imagen en la que se hizo la deteccion
         * 
         *  param alturaMax : altura de la imagen en la que se hizo la deteccion
         */
        void ajustarMiniCuadrado( int anchoMax, int alturaMax );

        /**
         * Retorna el bounding box o caja que redea a la deteccion
         * con formato soportado por opencv
         */
        cv::Rect getOpenCV2DRectBoundingBox();

        /**
         * Valida si al cara esta de perfil
         */
        bool caraDePerfil();

        /**
         * Valida si la cara es muy frontal
         */
        bool caraFrontal( std::vector<cv::Point2f> *lstPuntosReferencia, cv::Mat transAlineacion, float afinidadMaxima = 4.0 );
};


/**
 * Clase que detecta caras con Hailo usando
 * redes neuronales SCRFD
 */
class DetectorCarasHailoSCRFD
{
    public:

        /**
         * Ancho de las imagens aceptadas por el modelo
         */
        int imgModeloAncho;

        /**
         * Altura de las imagens aceptadas por el modelo
         */
        int imgModeloAltura;

        /**
         * Indica si se debe previsualizar la imagen
         * que se envia al detector de rostros
         */
        bool previsualizaImgParaDeteccion;

        /**
         * Indica si se debe esperar la presion de una tecla
         * una vez que se ha previsualizado la imagen
         * que se envia al detector de rostros
         */
        bool esperarPrevImgParaDeteccion;

        /*
        * Indica si se presento un error en la deteccion
        */
        bool errorDeteccion;

        /**
         * Runner empleado por la clase
         */
        Hailo8LRunner runner;

        /**
         * Ultima imagen redimensionada a la dimension esperada por la red neuronal
         */
        cv::Mat imagenRedim;

        /**
         * Rango de evluacion del proceso de eliminar
         * detecciones repetidas
         */
        float nms_iou_thresh;

        /**
         * Prefijo de las imagenes de deteccion guardadas
         */
        string prefijoImgDeteccion;

        /**
         * Contador del numero de imagenes guardadas
         */
        int secuencialImgDeteccion;

        /**
         * Constructor
         */
        DetectorCarasHailoSCRFD();

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
        bool ejecutar( cv::Mat imagen );


        /**
         * Retorna las detecciones.
         * El analiza el tipo de modelo y llama al metodo de deteccion correcto
         * 
         *      prec : 
         *          Porcentaje de precicion o accurary esperada
         */
        std::vector<DeteccionCaraHailo> detectar( GImage image, float prec );


        /**
         * Retorna las detecciones
         * 
         *      prec : 
         *          Porcentaje de precicion o accurary esperada
         */
        std::vector<DeteccionCaraHailo> detectar_2_5g( GImage image, float prec );

        /**
         * Retorna las detecciones
         * 
         *      prec : 
         *          Porcentaje de precicion o accurary esperada
         */
        std::vector<DeteccionCaraHailo> detectar_500m( GImage image, float prec );

        
        /**
         * Retorna las detecciones
         * 
         *      image:
         *          Imagen desde la que se hacen las detecciones
         * 
         *      prec : 
         *          Porcentaje de precicion o accurary esperada
         */
        std::vector<DeteccionCaraHailo> detectar_10g( GImage image, float prec );


        /**
         * Retorna las detecciones
         * 
         *      prec : 
         *          Porcentaje de precicion o accurary esperada
         */
        std::vector<DeteccionCaraHailo> getDetecciones_2_5g( float prec );

        /**
         * Retorna las detecciones
         * 
         *      prec : 
         *          Porcentaje de precicion o accurary esperada
         */
        std::vector<DeteccionCaraHailo> getDetecciones_500m( float prec );


        /**
         * Retorna las detecciones
         * 
         *      prec : 
         *          Porcentaje de precicion o accurary esperada
         */
        std::vector<DeteccionCaraHailo> getDetecciones_10g( float prec );

    private:

        /**
         * Codigo del tipo de modelo que al finel representa las salidas
         */
        int tipoModelo;
        
        /**
         * Extrae las cajas que rodean a las detecciones
         * 
         *      lstDet : 
         *          Lista en la que se guardan las detecciones
         * 
         *      nomCapaCajas :
         *          Nombre de la capa de salida que tiene las cajas
         * 
         *      nomCapaConf :
         *          Nombre de la capa de salida con las confidencias
         * 
         *      anchoImg : 
         *          Ancho de la imagen original
         * 
         *      alturaImg : 
         *          Altura de la imagen original
         * 
         *      confMin : 
         *          Es el valor minimo de la confianza para retornar la deteccion
         * 
         *      factorEscalaX :
         *          Factor por el que se debe multiplicar todas las coordenadas X para que se vean con respecto
         *          a la imagen original
         * 
         *      factorEscalaY :
         *          Factor por el que se debe multiplicar todas las coordenadas X para que se vean con respecto
         *          a la imagen original
         */
        void extraeDetec( vector<DeteccionCaraHailo> *rpta, string nomCapaCajas, string nomCapaConf, string nomCapaPuntosCara, float anchoImg, float alturaImg, float confMin, float escala[2], float factorEscalaX, float factorEscalaY );

        /**
         * Ejecuta non max supression
         * Para eliminar detecciones repetidas
         */
        vector<DeteccionCaraHailo> apply_nms(vector<DeteccionCaraHailo>& detections, float nms_iou_thresh);

        /**
         * Usada para el calculo de nms
         */
        float compute_iou(const DeteccionCaraHailo& a, const DeteccionCaraHailo& b);

        

        /**
         * Ancho de la imagen original
         */
        float anchoImgOrig;

        /**
         * Altura de la imagen oricinal
         */
        float alturaImgOrig;
        
};  

#endif
