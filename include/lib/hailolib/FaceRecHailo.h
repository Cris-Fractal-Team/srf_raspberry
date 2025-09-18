
#ifndef _FACEREC_HAILO_
#define _FACEREC_HAILO_

#include <string.h>
#include "hailo/hailort.hpp"
#include <iostream>
#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"
#include <opencv2/opencv.hpp>
#include "lib/hailolib/hailo8l.h"
#include "lib/hailolib/DetectorCaraHaloScrfd.h"
#include "lib/graphics/GDibujo.h"

// Referencia simple para evitar referencias circulares
class TrackedDetectionHailo;

/**
 * Seleccionar el tipo de dato con el que se desea trabajar
 * las operaciones que calculan la diferencia euclidiadana
 */
#define USE_FLOAT16
// #define USE_FLOAT32
// #define USE_INT16

#ifdef USE_FLOAT16
    typedef float16_t SIMD_TYPE;    
    typedef float16x8_t SIMD_VECTOR;
    #define LOAD_SIMD vld1q_f16
    #define SUB_SIMD vsubq_f16
    #define MUL_SIMD vmulq_f16
    #define ADD_SIMD vaddq_f16
    #define SUM_SIMD vaddvq_f16
    #define DUP_SIMD vdupq_n_f16
    #define STORE_SIMD vst1q_f16

#elif defined USE_INT16
    typedef int16_t SIMD_TYPE;
    typedef int16x8_t SIMD_VECTOR;
    #define LOAD_SIMD vld1q_s16
    #define SUB_SIMD vsubq_s16
    #define MUL_SIMD vmulq_s16
    #define ADD_SIMD vaddq_s16
    #define SUM_SIMD vaddvq_s16
    #define DUP_SIMD vdupq_n_s16
    #define STORE_SIMD vst1q_s16 
    #define NUMELEM_VECTOR_SIMD 8   
#else
    typedef float SIMD_TYPE;  // FP32 por defecto
    typedef float32x4_t SIMD_VECTOR;
    #define LOAD_SIMD vld1q_f32
    #define SUB_SIMD vsubq_f32
    #define MUL_SIMD vmulq_f32
    #define ADD_SIMD vaddq_f32
    #define SUM_SIMD vaddvq_f32
    #define DUP_SIMD vdupq_n_f32
    #define STORE_SIMD vst1q_f32 
    #define NUMELEM_VECTOR_SIMD 4
#endif

#define NUM_ELEMS_DESC_FACIAL 512

class UnivIdenPersona;

/**
 * Representa la deteccion de una persona
 */
class IdentificacionPersona
{
    public:

        /**
         * Fecha de deteccion
         */
        long long fecDet;

        /**
         * Factor de la comparacion o distancia con respecto
         * a una persona con la que se comparo
         */
        float comparacion;

        /**
         * Vector de la descripcion facial
         */
        SIMD_TYPE vecDescripcion[NUM_ELEMS_DESC_FACIAL];

        /**
         * Destructor
         */
        ~IdentificacionPersona()
        {            
        }
};

/**
 * Datos que describen a una persona en un sistema externo
 */
class DescPersonaExterno
{
    public:

        /**
         * Constructor
         */
        DescPersonaExterno()
        {
            norm_coseno = 0.0;
            norm_coceno_calculado = false;
        }


        /**
         * Codigo con el que se identifica a la persona
         */
        string id;

        /**
         * Nombre de la persona
         */
        string nombre;

        /**
         * Indica si el descriptor es anonimo.
         * Es decir fue creado durante la ejecucion de un programa
         * porque la persona no fue reconocida en una lista de personas 
         * 100% identificadas
         */
        bool anonimo;

        /**
         * Indica si el valor de norm_coceno se inicializo
         */
        bool norm_coceno_calculado;

        /**
         * Ultima fecha en la que se detecto a la persona en ms
         */
        long long ultimaFechaDetectada;

        /**
         * Vector de la descripcion facial
         */
        SIMD_TYPE vecDescripcion[NUM_ELEMS_DESC_FACIAL];        

        /**
         * Factor empleado para calcular la similaridad de coceno
         */
        float norm_coseno;

        /**
         * Lista de referencia a personas que tuvieron coincidencia
         * con esta descripcion externa
         */
        GLinkedList<UnivIdenPersona *> lstPersonas ;

        /**
         * Elimina las referencias que los universos de persona
         * tienen de este descriptor externo
         */
        void reset();

        /**
         * Normaliza el descriptor
         */
        void normaliza();
};


/**
 * Representa un grupo de detecciones que hacen
 * referencia a la misma persona
 */
class GrupoIdenPersona
{
    public:

        /**
         * Descriptor de la persona base externo
         */
        DescPersonaExterno *descExterno;        

        /**
         * Fecha de creacion del objeto 
         */
        long long fecCrea;

        /**
         * Lista de identificaciones
         */
        GLinkedList<IdentificacionPersona>lstIdentificaciones;
};

/**
 * Universo de identificaciones de una persona
 * En el se almancenan los N grupos de identificaciones para la misma persona.
 * Una persona puede tener N grupos identificadores debido a los falsos positivos,
 * esta clase tiene una lista por cada una de las coincidencias y se encarga 
 * de retornar el ID y el Nombre de la identificacion que ha tenido mas coincidencias
 */
class UnivIdenPersona
{
    public:

        /**
         * Lista con todos los identificadores de personas
         */
        GLinkedList<GrupoIdenPersona>lstGrupoIden;

        /**
         * ID unico del objeto, usado para el operador de igualdad
         */
        long long id;

        /**
         * Indice del grupo que tiene la mayor cantidad
         * de identificaciones, motivo por el cual se considera
         * que una cara corresponde a esa peronsa
         */
        int indiceGrupoPrin;

        /**
         * Cantidad de elementos del grupo con mayor numero de
         * identificaciones
         */
        int numIden;

        /**
         * Indica que la identificacion ha cambiado
         * ya se porque se detecto por primera vez a una persona o
         * porque ha cambiado el ID de la persona que tiene mas detecciones
         */
        bool cambioIdentificacion;

        /**
         * Fecha del ultimo evento en el que se reporto el rostro
         */
        long long fechaUltEvento;

        /**
         * Retorna un puntero con los datos de la persona
         * identificada.
         * Si no existe retorna NULL
         */
        DescPersonaExterno* getDatosPerIden();


        /**
         * Retorna el promedio de las similaridades o distancias de comparacion
         * del grupo actual
         */
        float getPromedioComparacion();

        /**
         * Retorna la cantidad de identificaciones que tiene el grupo actual
         */
        int getNumIdentificaciones();

        /**
         * Constructor
         */
        UnivIdenPersona();

        /**
         * Agrega una identificacion realizada al universo
         * 
         *      descExterno:
         *          Referencia al desriptor externo conocido de la persona.
         *          Si su valor es NULL significa que se trata de una persona no identificada.ADJ_OFFSET_SINGLESHOT
         * 
         *      iden:
         *          Identificacion de una persona que se proceso y a la que se busco un descriptor externo
         * 
         *      fecDet: 
         *          Fecha en la que se hizo la deteccion
         * 
         */
        void agregaIdentif( DescPersonaExterno *descExterno, IdentificacionPersona iden, long long fecDet );

        /**
         * Borra las referencias a descriptor externo
         */
        void borrarDescPersonaExterno( DescPersonaExterno *descExterno );

        /**
         * Retorna la ultima identificacion
         */
        IdentificacionPersona getUltimaIdentificacion();

        /**
         * Libera RAM del objeto
         */
        void reset();
};


/**
 * Clase que represanta una cara con descriptor facial
 */
class CaraDescrita
{
    public:

        /**
         * Deteccion asociada a la cara relativa a la imagen original
         */
        DeteccionCaraHailo deteccion;

        /**
         * Deteccion asociada a la cara con 
         * las coordenadas de los ojos, nariz y boca relativos
         * a la esquina superior izquierda
         */
        DeteccionCaraHailo detRelCara;

        /**
         * Foto de la cara
         */
        GImage fotoCara;

        /**
         * Universo identificador de una persona
         */
        UnivIdenPersona identificador;
        
        /**
         * Indica si la persona fue o no identificada
         */
        bool personaIdent;

        /**
         * descriptor facial
         */
        SIMD_TYPE descriptor[NUM_ELEMS_DESC_FACIAL];

        /**
         * Indica si se calculo o no el descriptor facial
         */
        bool descCalculado;

        /**
         * ID unico de la cara
         */
        long long id;

        /**
         * Constructor
         */
        CaraDescrita();

        /**
         * Calcula la foto de la cara en base a la deteccion 
         * extrallendola desde la foto en la que se realizo la deteccion
         */
        void calculaFotoCara( GImage foto );
};



/**
 * Clase que brinda el servicio que genera descriptores faciales
 */
class FaceRecHailo
{
    public:

        /**
         * Indica si se presento un error en el calculo
         */
        bool errorCalculo;

        /**
         * Similaridad frontal maxima
         */
        float similaridadFrontal;

        /**
         * Runner empleado por la clase
         */
        Hailo8LRunner runner;

        /**
         * Tamaño maximo del batch para procesar inferencias en bloque
         */
        size_t maxBatchSize;

        /**
         * Nombre de la ultima capa del modelo de red neuronal
         */
        string nombreUltimaCapaModelo;

        /**
         * Factor para calcular el contraste
         */
        float paramContraste;

        /**
         * Indica si se debe visualizar el rostro original
         * y su version transformada en posicion frontal
         */
        bool previsualizaImgReconocimiento;

        /**
         * Indica si se debe guardar las caras frontales
         * alineadas, ellas se pueden usar para entrenamiento
         * o mejora en modelos de reconocimiento
         */
        bool guardarCarasFrontalesAlineadas;


        /**
         * Indica si se debe esperar la presion de una tecla
         * luego que se muestran las imagenes que
         * se envian para reconocimiento
         */
        bool esperarPrevImgReconocimiento;

        /**
         * Indica si se debe guardar 
         */
        bool guardarPrevImgReconocimiento;

        /**
         * Codigo de seguimiento
         */
        int secuencialImgReconocimiento;

        /**
         * Prefijo de las imagenes que se guardan en disco
         */
        string prefijoImgReconocimiento;

        /**
         * Constructor
         */
        FaceRecHailo();

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
        bool ejecutar( cv::Mat imagen, DeteccionCaraHailo caraDet );

        /**
         * Ejecuta la deteccion
         */
        bool ejecutar( GLinkedList<cv::Mat> *lstImagenes, GLinkedList<DeteccionCaraHailo> *lstDetecciones );


        /**
         * Extrae el descriptor facial de la ultima inferencia
         */
        void extraeDescriptor( SIMD_TYPE *descriptor, int offset = 0 );

        /**
         * Calcula el descriptor facial para una cara
         * Retorna true en caso de exito, false en caso de error
         */
        bool calculaDescriptor( GImage imagen, DeteccionCaraHailo deteccion, SIMD_TYPE *descriptor );

        /**
         * Calcula el descriptor facial para una cara
         * Retorna true en caso de exito, false en caso de error
         */
        // bool calculaDescriptor( GLinkedList<GImage> *lstImagenes, GLinkedList<DeteccionCaraHailo> *lstDetecciones, GLinkedList<SIMD_TYPE *> *lstDescriptores );
        bool calculaDescriptor( GLinkedList<TrackedDetectionHailo *> *lstDeteccionesTrack );


        /**
         * Extrae el descriptor facial de la ultima inferencia en formato de OpenCV
         */
        cv::Mat extraeDescriptorMat();
        
    private:

        /**
         * Lista de puntos de referencia
         */
        std::vector<cv::Point2f> lstPuntosReferencia;

        /**
         * Calcula la matriz de alineacion para un rostro detectado
         */
        cv::Mat calculaMatrizAlineacion( DeteccionCaraHailo deteccion );

        /**
         * Lista de indices de rostros que se han enviado a procesar
         * no todos los rostros se procesan porque algunos no estan cercanos
         * a ser frontales
         */
        GLinkedList<int>lstIndicesRostrosProc;

        /**
         * Ancho de las imagens aceptadas por el modelo
         */
        int imgModeloAncho;

        /**
         * Altura de las imagens aceptadas por el modelo
         */
        int imgModeloAltura;

        /**
         * Dada una cara detectada , la alinea para ejecutar el reconocimiento
         * facial, usando como dato los puntos de una deteccion previa
         */
        cv::Mat aliearRostro( cv::Mat fotoCara, DeteccionCaraHailo deteccion, cv::Mat transformacion );

        /**
         * Vector en el que se almacenan los datos de las fotos de caras
         */
        std::vector<uint8_t> input_buffer;

        /**
         * Indica si ya se reservo RAM en el vector para almacenar los datos 
         * de los rostros
         */
        bool vectorIniciado;

};

#endif
