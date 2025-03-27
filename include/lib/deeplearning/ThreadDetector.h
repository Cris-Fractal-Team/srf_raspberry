#ifndef THREADDETECTOR_H
#define THREADDETECTOR_H

#include <thread>
#include <mutex>
#include <memory>
#include <string>

#include "DetectorBO.h"
#include "lib/general/GVector.h"
#include "lib/general/GLinkedList.h"
#include "DetectorImageSource.h"
#include "lib/general/GThread.h"

using namespace std;

/**
 * Clase que encapsula la deteccion en un thread.
 * Solicita una imagen cada vez que necesita hacer detecciones.
 * Siempre tendra una lista de las ultimas detecciones que realizo.
 */
class ThreadDetector : public GThread
{
    public:

        /**
         * Fuente de imagenes con el que trabaja el detector
         */
        DetectorImageSource *imageSource;
        
        /**
         * Constructor
         */
        ThreadDetector();

        /**
         * Destructor
         */
        virtual ~ThreadDetector();

        /**
         * Retorna las ultimas detecciones
         */
        GLinkedList<DeteccionVO> getDetecciones();


         /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Bucle que se ejecuta en paralelo
         */
        void bucleDetecciones( );

         /**
         * Objeto con el que hace las detecciones
         */
        shared_ptr<DetectorObjetos> detector;

        /**
         * Establece la ruta de las etiquetas y carga el archivo
         */
        bool cargaEtiquetas( string path );

        /**
         * Establece las dimensiones de las imagenes originales
         */
        void setDimImgOrig( int ancho, int altura );

        /**
         * Establece las dimensiones de las imagenes del modelo
         */
        void setDimImgModelo ( int ancho, int altura, int numBytesPorPixel );

        /**
         *  Establece el path del modelo
         */
        void setPathModelo( string path );

        /**
         * Retorna el tiempo de la ultima inferencia
         */
        float getTiempoUltInf();

        /**
         * Establece la tolerancia de las detecciones
         */
        void setToleranciaDet( float tolerancia );

    protected:

        /**
         * Lista con las ultimas detecciones
         */
        GLinkedList<DeteccionVO> lstDetecciones;

    private:

        /**
         * Tolerancia para las detecciones
         */
        float toleranciaDet;


        /**
         * Flag para saber se se ha o no obenido una referencia al vetor
         * con las ultimas detecciones.
         * Para saber si se debe o no liberar la RAM del vector
         *  1: Indica que se ha obtenido una referencia externa, es responsabilidad del externo de liberar la ram
         *  0: No se ha obtenido una referencia, el thread liberara la ram
         */
        char deteccionesLeidas;

        /**
         *  Indica si se debe finalizar el bucle de detecciones
         */
        char finalizarBucle;

        /**
         * Retorna el valor que indica si se debe o no finalizar el bucle
         */
        char debeFinalizar();

        /**
         * Thread en el que se ejecuta
         */
        // std::thread *th;

        /**
         * Dimensiones en X de las imagenes originales
         */
        int dimImgOrigX;

        /**
         * Dimensiones en Y de las imagenes originales
         */
        int dimImgOrigY;

        /**
         * Dimensiones X de las imagenes del modelo de inferencia
         */
        int dimImgModeloX;

        /**
         *  Dimensiones Y de las imagenes  del modelo de inferencia
         */
        int dimImgModeloY;

        /**
         * Numero de bytes por pixel del modelo
         */
        int numBytesPorPixelModelo;

        /**
         * Ruta para el modelo
         */
        string pathModelo;

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Tiempo de la ultima inferencia
         */
        float tiempoInferencia; 
};

#endif // THREADDETECTOR_H
