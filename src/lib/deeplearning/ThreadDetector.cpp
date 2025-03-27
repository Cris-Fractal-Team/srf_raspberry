#include "lib/deeplearning/ThreadDetector.h"
#include "lib/graphics/GDibujo.h"

#include <thread>
#include <mutex>
#include <chrono>
// #include <opencv2/opencv.hpp>
#include "lib/utils/timedate.h"

using namespace std;
// using namespace cv;


ThreadDetector::ThreadDetector() : GThread("ThreadDetector")
{
    deteccionesLeidas = 0;
    finalizarBucle  =  0;
    toleranciaDet = 0.5;
}

ThreadDetector::~ThreadDetector()
{
    
}

/**
 * Bucle del thread
 */
void ThreadDetector::runThread()
{
    cout << "Run Thread" << endl;
    bucleDetecciones();
}

/**
 * Establece la tolerancia de las detecciones
 */
void ThreadDetector::setToleranciaDet( float tolerancia )
{
    toleranciaDet = tolerancia;
    detector->confianza = tolerancia;
}


/**
 * Retorna las ultimas detecciones.
 * Es responsabilidad del programa que llama a esta instruccion de borrar las detecciones
 */
GLinkedList<DeteccionVO> ThreadDetector::getDetecciones()
{
    GLinkedList<DeteccionVO> rpta;

    mtx.lock();
    rpta = lstDetecciones.getClone();
    if ( lstDetecciones.size() > 0 )
    {
        // hacemos NULL porque se ha obtenido la lista de detecciones
        lstDetecciones.reset();
    }    
    mtx.unlock();

    return rpta;
}

/**
 * Retorna el tiempo de la ultima inferencia
 */
float ThreadDetector::getTiempoUltInf()
{
    return tiempoInferencia;
}

/**
 * Bucle que se ejecuta en paralelo
 */
void ThreadDetector::bucleDetecciones( )
{
    GLinkedList<DeteccionVO> detec;
    DeteccionVO* deteccion;
    GImage imagen,imagenOrig,img;    
    int i,n,numDet;
    std::chrono::milliseconds intervaloEspera(1);

    std::cout << "******  Thread de detecciones iniciado " << this->id_unico <<  endl;

    std::cout << "Modelo de inferencia cargado"  << endl;

    long long ahora, anterior, delta, numBucles, sumaTiempo;
    
    anterior = 0;
    numBucles = 0;
    sumaTiempo = 0;

    // while( debeFinalizar() == 0 )
    while( isFinalizado() == false )
    {                
        // imageSource->bloqueImagenes();
        if ( imageSource->estaVacio() == true )
        {
            sleepMS(1);
            continue;
        }

        imagen = imageSource->getImagenModelo(true);
        imagenOrig = imageSource->getImage();

        // tIni = chrono::steady_clock::now();
        detec = detector->detectarLst(&imagen);
        // tFin = chrono::steady_clock::now();
        // tiempoInferencia = chrono::duration_cast <chrono::milliseconds> (tFin - tIni).count();
       
        numDet = detec.size();
        for(i=0;i<numDet;i++)
        {                    
            deteccion = detec.getAddr(i);
            deteccion->imagen = imagenOrig;
        }   

        mtx.lock();        
        lstDetecciones.reset();        
        lstDetecciones = detec;
        mtx.unlock();        
        
        std:this_thread::sleep_for(intervaloEspera);
    }

    mtx.lock();
    if ( lstDetecciones.size() > 0 )
    {
        lstDetecciones.reset();
    }
    mtx.unlock();

    std::cout <<  "Finalizando el thread detector" << endl;
}

/**
 * Retorna el valor que indica si se debe o no finalizar el bucle
 */
char ThreadDetector::debeFinalizar()
{
    char rpta;

    mtx.lock();
    rpta = finalizarBucle;
    mtx.unlock();

    return rpta;
}


/**
 * Establece la ruta de las etiquetas y carga el archivo
 */
bool ThreadDetector::cargaEtiquetas( string path )
{
    return detector->cargaEtiquetas(path);
}

/**
 * Establece las dimensiones de las imagenes originales
 */
void ThreadDetector::setDimImgOrig( int ancho, int altura )
{
    dimImgOrigX  = ancho;
    dimImgOrigY = altura;
}

/**
 * Establece las dimensiones de las imagenes del modelo
 */
void ThreadDetector::setDimImgModelo ( int ancho, int altura, int numBytesPorPixel )
{
    dimImgModeloY = altura;
    dimImgModeloX = ancho;
    numBytesPorPixelModelo  = numBytesPorPixel;
}

/**
 *  Establece el path del modelo
 */
void ThreadDetector::setPathModelo( string path )
{
    pathModelo = path;
}
