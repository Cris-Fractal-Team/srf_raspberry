
#ifndef _FACEREC_MAKVAL_
#define _FACEREC_MAKVAL_

#include "lib/general/GObject.h"
#include "lib/general/GVector.h"
#include "lib/general/GLinkedList.h"
#include "lib/graphics/GDibujo.h"
#include "lib/deeplearning/DeteccionVO.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core/types.hpp"

/**
 * Clase que representa el ID de un rostro
 */
class FaceDescriptor : public GObject
{
    public:

        /**
         * Descripcion de texto 
         */
        string textDesc;

        /**
         * ID del rostro manejado por un servidor 
         */
        string serverFaceId;
    
        /**
         * Retorna un arreglo de floats con los datos
         */
        float* getFloats();
       

        /**
         * Calcula la diferencia con otro descriptor de rostro
         */
        virtual float calculaDiferencia( FaceDescriptor *faceDesc );

        /**
         * Calcula la diferencia con otro descriptor de rostro 
         * usando instrucciones SIMD 
         */
        virtual float calculaDiferenciaSIMD( FaceDescriptor *faceDesc );

        /**
         * Cantidad de elementos del descriptor
         */
        void setSize( int numElems );

        /**
         * Retorna la cantidad de elementos del arreglo descriptor
         */
        int getSize();


        /**
         * Constructor
         */
        FaceDescriptor();


        /**
         * Destructor
         */
        ~FaceDescriptor();

        /**
         * Retorna el tipo de dato
         */
        virtual string getType();


    private :

        /**
         * Descriptor float
         */
        float descriptorFloat[128];

        /**
         * Numero de elementos del arreglo descriptor
         */
        int descriptorSize;
};


/**
 * Representa el rostro de una persona para hacerle
 * reconocimiento facial.
 * Contiene una lista de varias descripciones de rostros.
 */
class PersonFaceDescriptor : public GObject
{
    public:

        /**
         * Constructor
         */
        PersonFaceDescriptor();

        /**
         * Destructor
         */
        ~PersonFaceDescriptor();

        /**
         * Calcula la diferencia con un descriptor de rostro.
         * Retorna la minima diferencia comparando con todos los rostros que tiene la persona
         */
        float calculaDiferencia( FaceDescriptor *faceDesc );


        /**
         * Agrega un descriptor a la persona
         */
        void addDescriptor( FaceDescriptor *descriptor );


        /**
         * Retorna el tipo de dato
         */
        virtual string getType();


        /**
         * Lista de descriptores de rostros
         */
        GLinkedList<FaceDescriptor> lstDescriptores;
};


/**
 * Clase base que representa un procesador de reconocimiento facial
 */
class FaceRecProcessor
{
    public:

        /**
         * Numero de puntos que se deben detectar en el rostro
         * para poder hacer la alineacion correcta
         */
        int numPuntosRostro;

        /**
         * Carga las redes por defecto
         */
        virtual void initRedesPorDefecto() = 0;

        /**
         * Carga la red para detectar puntos del rostro
         * 
         *  path: ruta del archivo
         *  
         *  numPuntos : dantidad de puntos que se deben detectar en el rostro
         *              para garantizar una correcta alineacion
         */
        virtual void cargaModeloFaceLandMark( std::string path, int numPuntos ) = 0;

        /**
         * Carga el modelo para calcular el ID de un rostro
         */
        virtual void cargaModeloFaceRec( std::string path ) = 0;

        /**
         * Calcula el descriptor de un rostro
         * 
         *  imagenRostro : imagen de un rostro
         *
         * Retorna el descriptor de un rostro o NULL en caso de ERROR
         */
        virtual FaceDescriptor getDescriptorRostro( GImage imagenRostro, DeteccionVO param ) = 0;

        /**
         * Destructor
         */
        ~FaceRecProcessor();

};


class SFFaceDetector : public FaceRecProcessor
{

    public :

        /**
         * Destructor
         */
        ~SFFaceDetector();

        /**
         * Carga las redes por defecto
         */
        void initRedesPorDefecto() override;

        /**
         * Carga la red para detectar puntos del rostro
         * 
         *  path: ruta del archivo
         *  
         *  numPuntos : dantidad de puntos que se deben detectar en el rostro
         *              para garantizar una correcta alineacion
         */
        void cargaModeloFaceLandMark( std::string path, int numPuntos ) override;

        /**
         * Carga el modelo para calcular el ID de un rostro
         */
        void cargaModeloFaceRec( std::string path ) override;

        /**
         * Calcula el descriptor de un rostro
         * 
         *  imagenRostro : imagen de un rostro
         *
         * Retorna el descriptor de un rostro o NULL en caso de ERROR
         */
        FaceDescriptor getDescriptorRostro( GImage imagenRostro, DeteccionVO param ) override;

    private:

        cv::Ptr<cv::FaceRecognizerSF> _recognizer;


};

#endif