#include "lib/deeplearning/DetectorImageSource.h"
// #include <opencv2/opencv.hpp>
#include "lib/graphics/GDibujo.h"

// using namespace cv;
using namespace std;

DetectorImageSource::DetectorImageSource()
{
    imagenCambiada = 0;
    vacio = 1;
}

DetectorImageSource::~DetectorImageSource()
{
    //dtor
}


/**
 * Retorna al referencia a la imagen que se puede procesar.
 * Esta funcion se bloquea mientras que no se libere la imagen.
 */
// Mat DetectorImageSource::getImage()
GImage DetectorImageSource::getImage()
{
    return imagenProc;
}


/**
 * Establece la imagen que debe enviarse al detector
 */
// void DetectorImageSource::setImage( Mat image )
void DetectorImageSource::setImage( GImage image )
{
    if ( image.imagenOpencv.empty() )
    {
        return;
    }

    mtx.lock();

    imagenProc=image.clone();
    imagenCambiada = 1;
    vacio = 0;

    mtx.unlock();
}


/**
 *  Retorna la imagen para el modelo
 */
GImage DetectorImageSource::getImagenModelo( bool bloquea = true)
{
    if ( bloquea )  mtx.lock();
    if ( imagenCambiada  ==  1 )
    {
        imageModelo = imagenProc.cloneResize(imgModeloAncho,imgModeloAltura);        
        imagenCambiada  = 0;
    }
    if ( bloquea ) mtx.unlock();

    return imageModelo;
}

/**
 * Bloquea el source para obtener imagenes
 */
void DetectorImageSource::bloqueImagenes()
{
    mtx.lock();
}

/**
 * Libre el source para que se puedan modificar las imagenes
 */
void DetectorImageSource::liberaImagenes()
{
    mtx.unlock();
}


/**
 * Establece la dimensiones de la imagen del modelo
 */
void DetectorImageSource::setDimImgModelo( int ancho, int altura )
{
    imgModeloAncho = ancho;
    imgModeloAltura = altura;

    cout << ">> DetectorImageSource Dim Imagen : " << imgModeloAncho << " x " << imgModeloAltura << endl;
}


/**
 * Valida si esta vacio o no tiene imagenes que enviar.
 * @return 1 si esta vacio, 0 si no esta vacio.
 */
char DetectorImageSource::estaVacio()
{
    char rpta;

    mtx.lock();
    // rpta = vacio;
    if ( imagenCambiada == 1 ) rpta = false;
    else rpta = true;
    mtx.unlock();

    return rpta;
}
