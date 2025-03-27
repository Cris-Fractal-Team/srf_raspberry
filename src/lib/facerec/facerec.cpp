
#include "lib/facerec/facerec.h"
#include <math.h>
#include <arm_neon.h>
#include <cmath>
#include <iostream>
#include <vector>



 /**
  * Retorna un arreglo de floats con los datos
  */
 float* FaceDescriptor::getFloats()
 {
    return descriptorFloat;
 }


 /**
  * Retorna la cantidad de elementos del arreglo descriptor
  */
int FaceDescriptor::getSize()
{
    return descriptorSize;
}


 /**
  * Calcula la diferencia con otro descriptor de rostro
  */
 float FaceDescriptor::calculaDiferencia( FaceDescriptor *faceDesc )
 {    
    int i,n;
    float delta, suma = 0;

    n = faceDesc->getSize();

    for(i=0; i < n; i++)
    {
        delta = descriptorFloat[i] - faceDesc->descriptorFloat[i];
        suma+= (delta*delta);
    }
    suma = sqrt(suma);

    return suma;
 }

/**
 *  Función para calcular la distancia euclidiana usando NEON SIMD
 */ 
float FaceDescriptor::calculaDiferenciaSIMD( FaceDescriptor *faceDesc ) 
{
    size_t size = faceDesc->getSize();

    size_t simd_size = size / 4;  // Cada registro SIMD maneja 4 floats
    float32x4_t sum_vec = vdupq_n_f32(0.0f);  // Inicializa a cero el acumulador SIMD

    // Procesamiento en bloques de 4 floats
    for (size_t i = 0; i < simd_size * 4; i += 4) {
        float32x4_t v1 = vld1q_f32(&descriptorFloat[i]);    // Carga 4 elementos de arr1
        float32x4_t v2 = vld1q_f32(&faceDesc->descriptorFloat[i]);    // Carga 4 elementos de arr2
        float32x4_t diff = vsubq_f32(v1, v2);    // Resta los vectores
        float32x4_t sq = vmulq_f32(diff, diff);  // Eleva al cuadrado
        sum_vec = vaddq_f32(sum_vec, sq);        // Acumula
    }

    // Reduce el vector SIMD a un escalar
    float result_array[4];
    vst1q_f32(result_array, sum_vec);
    float sum = result_array[0] + result_array[1] + result_array[2] + result_array[3];

    // Procesa los elementos restantes (si el tamaño no es múltiplo de 4)
    for (size_t i = simd_size * 4; i < size; ++i) {
        float diff = descriptorFloat[i] - faceDesc->descriptorFloat[i];
        sum += diff * diff;
    }

    return std::sqrt(sum);  // Devuelve la raíz cuadrada del resultado
}


 /**
  * Cantidad de elementos del descriptor
  */
 void FaceDescriptor::setSize( int numElems )
 {
    descriptorSize = numElems;
 }


 /**
  * Constructor
  */
 FaceDescriptor::FaceDescriptor()
 {    
    descriptorSize = 128;
    textDesc = "";
    serverFaceId = "";
 }


 /**
  * Destructor
  */
 FaceDescriptor::~FaceDescriptor()
 {
 }

 /**
 * Retorna el tipo de dato
 */
string FaceDescriptor::getType() 
{ 
    return string("FaceDescriptor"); 
}


 /**
 * Constructor
 */
PersonFaceDescriptor::PersonFaceDescriptor()
{

}

/**
 * Retorna el tipo de dato
 */
string PersonFaceDescriptor::getType() 
{ 
    return string("PersonFaceDescriptor"); 
}

/**
 * Destructor
 */
PersonFaceDescriptor::~PersonFaceDescriptor()
{

}

/**
 * Calcula la diferencia con un descriptor de rostro.
 * Retorna la minima diferencia comparando con todos los rostros que tiene la persona
 */
float PersonFaceDescriptor::calculaDiferencia( FaceDescriptor *faceDesc )
{
    FaceDescriptor *desc;    
    float minimo,dif;
    int i,n;

    minimo = 100;

    n = lstDescriptores.size();
    for(i=0; i < n; i++)
    {
        desc = lstDescriptores.getAddr(i);
        dif = desc->calculaDiferencia(faceDesc);

        if ( dif < minimo )
        {
            minimo = dif;
        }
    }

    return minimo;
}


/**
 * Agrega un descriptor a la persona
 */
void PersonFaceDescriptor::addDescriptor( FaceDescriptor *descriptor )
{
    lstDescriptores.add(*descriptor);
}

/**
 * Destructor
 */
FaceRecProcessor::~FaceRecProcessor()
{

}


/**
 * Destructor
 */
SFFaceDetector::~SFFaceDetector()
{

}

/**
 * Carga las redes por defecto
 */
void SFFaceDetector::initRedesPorDefecto()
{

}

/**
 * Carga la red para detectar puntos del rostro
 * 
 *  path: ruta del archivo
 *  
 *  numPuntos : dantidad de puntos que se deben detectar en el rostro
 *              para garantizar una correcta alineacion
 */
void SFFaceDetector::cargaModeloFaceLandMark( std::string path, int numPuntos )
{

}

/**
 * Carga el modelo para calcular el ID de un rostro
 */
void SFFaceDetector::cargaModeloFaceRec( std::string path )
{
    _recognizer = cv::FaceRecognizerSF::create(path, "", 1,2);
}

/**
 * Calcula el descriptor de un rostro
 * 
 *  imagenRostro : imagen de un rostro
 *
 * Retorna el descriptor de un rostro o NULL en caso de ERROR
 */
FaceDescriptor SFFaceDetector::getDescriptorRostro( GImage imagenRostro, DeteccionVO param )
{
    FaceDescriptor desc;

    cv::Mat target_aligned;
    cv::Mat rectangulo(1,1,CV_32SC4);
    cv::Rect rc; 
    cv::Mat caracteristicas;

    rc.x = 0;
    rc.y = 0;
    rc.width = imagenRostro.ancho-1;
    rc.height = imagenRostro.altura-1;

    rectangulo.at<cv::Rect>(0,0) = rc;

    _recognizer->alignCrop(imagenRostro.imagenOpencv,rectangulo, target_aligned);
    _recognizer->feature(target_aligned, caracteristicas);

    for(int i=0;i<caracteristicas.rows; ++i)
    {
        for(int j=0;j<caracteristicas.cols;++j)
        {
            cout << "Fila " << i << " Columna " << j << " = " << caracteristicas.at<float>(i,j) << endl;
        }
    }

    // cv::imshow("Original", imagenRostro.imagenOpencv);
    // cv::imshow("Alineado", target_aligned);
    
    return desc;
}