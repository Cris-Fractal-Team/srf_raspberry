

#ifndef _FACE_REC_YP_
#define _FACE_REC_YP_

#include "lib/deeplearning/DeteccionVO.h"
#include "lib/deeplearning/DetectorBO.h"
#include "lib/facerec/facerec.h"


/**
 * Clase que representa una deteccion hecha por la libreroa
 * FACERECYN que incorpora FaceLandMark en la deteccion
 */
class DeteccionFaceVO : public GObject
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
        DeteccionFaceVO();

        /**
         * Destructor
         */
        ~DeteccionFaceVO();        
              
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

        /**
         * Retorna el tipo de dato
         */
         string getType();
        
};


/**
 * Clase que detecta rostros
 * Retorna instancias DateccionFaceVO
 */
class DetectorRostrosYN : public DetectorObjetos
{
    public:

        /**
         * Constructor
         */
        DetectorRostrosYN();

        /**
         * Destructor
         */
        ~DetectorRostrosYN();


        /**
         * Carga el modelo
         * @param path ruta del modelo
         */
        void cargaModelo( std::string path ) override;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @return lista de instancias de DateccionFaceVO
         */
        std::shared_ptr<GVector> detectar( GImage *imagen ) override;

        /**
         * Detecta elementos
         * @param imagen imagen que se procesa
         * @param numDet variable que se actualiza con la cantidad de detecciones
         * @return puntero a un arreglo de detecciones
         */
        GLinkedList<DeteccionVO> detectarLst(GImage *imagen ) override;

    private:

        /**
         * Detector empleado
         */
        cv::Ptr<cv::FaceDetectorYN> detector;

        /**
         * Dimension del modelo
         */
        cv::Size dimModelo;

        /**
         * Escala horizontal de la imagen a detectar vs la imagen original
         */
        float escalaHor;

        /**
         * Escala vertical de la imagen a detectar vs la imagen original
         */
        float escalaVer;
};


/**
 * Clase que ejecuta el reconocimiento facial
 * usando SFace
 */
class FaceRecProcessorSFace: public FaceRecProcessor
{
    public:

        /**
         * Constructor
         */
        FaceRecProcessorSFace();

        /**
         * Destructor
         */
        ~FaceRecProcessorSFace();

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

        /**
         * Red que genera la descripcion de un rostro
         */
        cv::Ptr<cv::FaceRecognizerSF> recognizer;
};


#endif