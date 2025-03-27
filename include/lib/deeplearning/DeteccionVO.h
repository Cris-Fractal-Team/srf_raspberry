#ifndef DETECCIONVO_H
#define DETECCIONVO_H

#include "lib/graphics/GDibujo.h"

// #include <opencv2/opencv.hpp>
// #include <opencv2/dnn.hpp>
// #include <opencv2/highgui.hpp>

#include "lib/general/GObject.h"

// using namespace cv;
using namespace std;

// #define new_DeteccionVO(  ) (DeteccionVO *)&(GPointer(new DeteccionVO()))


class DeteccionVO : public GObject
{
    public:

        /**
         * Region de la deteccion
         */
        GRect region;

        /**
         * Imagen detectada
         */
        GImage imagen;

        /**
         * Indica o codigo de la clase detectada
         */
        int indiceClase;

        /**
         * Confianza de la clase
         */
        float confianza;

        /**
         * Identificador que hace unica la deteccion.
         * Es responsabilidad del programa que usa esta clase el uso que se le da
         */
        long long id;

        /**
         * Deteccion original
         */
        cv::Mat deteccionOrig;

        /**
         * Punto Ojo Izquierdo
         */
        GPoint ptoOjoIzq;

        /**
         * Punto Ojo Der
         */
        GPoint ptoOjoDer;

        /**
         * Punto Nariz
         */
        GPoint ptoNariz;

        /**
         * Punto Boca Izquierdo
         */
        GPoint ptoBocaIzq;

        /**
         * Punto Boca Derecha
         */
        GPoint ptoBocaDer;

        /**
         * Constructor
         */
        DeteccionVO();

        /**
         * Destructor
         */
        virtual ~DeteccionVO();

        /**
         * Retorna el tipo de dato
         */
        string getType() override;

        /**
         * Asigna la imagen
         */
        void setImage( GImage image );

        /**
         * Retorna la imagen
         */
        GImage getImage();

        /**
         * Clona una deteccion
         */
        DeteccionVO getClone();

        /**
         * Copia los valores desde otra instancia de DeteccionVO.
         * Util para cuando hay herencia, se puede sobre escribir
         * este metodo y se puede igualar una clase padre a una hija
         */
        void copyValues( DeteccionVO value );

        /**
         * Retorna la posicion del ojo izquierdo
         */
        GPoint getPosOjoIzq();

        /**
         * Retorna la posicion del ojo izquierdo
         */
        GPoint getPosOjoDer();
       
        /**
         * Retorna la posicion de la nariz
         */
        GPoint getPosNariz();

        /**
         * Retorna la posicion izquierda de la boca
         */
        GPoint getPosBocaIzq();

        /**
         * Retorna la posicion derecha de la boca
         */
        GPoint getPosBocaDer();

    protected:

    private:
};

#endif // DETECCIONVO_H
