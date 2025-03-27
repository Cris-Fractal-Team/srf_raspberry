#ifndef DETECTORIMAGESOURCE_H
#define DETECTORIMAGESOURCE_H

#include <cmath>
// #include <opencv2/opencv.hpp>
#include "lib/graphics/GDibujo.h"

// using namespace cv;
using namespace std;


/**
 * Clase que representa un proveedor de imagenes a ser detectadas.
 *
 * Su objetivo es que en el bucle principal que captura las imagenes para ser procesadas
 * debe llamar al metodo setImage de esta clase para indicar que esta es la imagen que
 * actualmente debe de ser procesada.
 *
 * El proceso que hace inferencia neuronal necesita una version escalada de la imagen a una resolucion
 * compatible con la red neuronal, para ello debe llamar al metodo getImagenModelo la cual
 * retornara la version adecuada de la imagen.
 *
 * Lo interesante de esta clase es que solo se escalara la imagen cuando el primer proceso neuronal
 * lo necesite, no sera necesario escalar la imagen cada vez que se tenga disponible una imagen
 * desde su fuente, de esta manera se hace mas rapido el proceso.
 */
class DetectorImageSource
{
    public:


        DetectorImageSource();
        virtual ~DetectorImageSource();

        /**
         * Retorna la imagen para procesar
         * Esta funcion se bloquea mientras que no se libere la imagen
         */
        // Mat getImage();
        GImage getImage();

        /**
         * Establece la imagen que debe enviarse al detector
         */
        // void setImage( Mat image );
        void setImage( GImage image );


        /**
         *  Retorna la imagen para el modelo
         */
        // Mat getImagenModelo();
        GImage getImagenModelo( bool bloquea );

        /**
         * Bloquea el source para obtener imagenes
         */
        void bloqueImagenes();

        /**
         * Libre el source para que se puedan modificar las imagenes
         */
        void liberaImagenes();

        /**
         * Establece la dimensiones de la imagen del modelo
         */
        void setDimImgModelo( int ancho, int altura );


        /**
         * Valida si esta vacio o no tiene imagenes que enviar.
         * @return 1 si esta vacio, 0 si no esta vacio.
         */
        char estaVacio();

    protected:

    private:

        /**
         * Indica que la imagen ha cambiado entonces, la primera vez que se llame a getImagenModelo
         * primero se debera escalar la imagen
         */
        char imagenCambiada;
        
        /**
         *  Imagen que se debe procesar
         */
        // Mat imagenProc;
        GImage imagenProc;

        /**
         * Imagen que se debe enviar a los modelos
         */
        // Mat imageModelo;
        GImage imageModelo;

        /**
         * Ancho en pixeles de la imagen del modelo
         */
        int imgModeloAncho;

        /**
         * Altura en pixeles de la imagen del modelo
         */
        int imgModeloAltura;

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Indica si esta vacio
         */
        char vacio;

};

#endif // DETECTORIMAGESOURCE_H
