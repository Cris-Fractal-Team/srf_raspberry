
#ifndef _HAILO_LIB_
#define _HAILO_LIB_

#include <string.h>
#include "hailo/hailort.hpp"
#include <iostream>
#include "lib/general/GThread.h"

using namespace std;

/**
 * Estado que indica el componente recian ha sido creado
 */
#define H8LR_LIB_ESTADO_NUEVO 1

/**
 * Estado que indica el componente recian ha sido creado
 */
#define H8LR_LIB_ESTADO_PEND_CARGAR_RED 2

/**
 * Estado que indica se ha cargado la red neuronal
 */
#define H8LR_LIB_ESTADO_RED_CARGADA 3

/**
 * Estado que indica se producto un error al cargar la red
 */
#define H8LR_LIB_ESTADO_ERROR_CARGA_RED 4

/**
 * Estado que indica el componente aun no puede ejecutar
 */
#define H8LR_LIB_INF_PENDIENTE 0

/**
 * Estado que indica el componente esta esperando data para inferencia
 */
#define H8LR_LIB_INF_ESPERA_DATA 1

/**
 * Estado que indica el componente esta esperando que se haga la inferencia 
 */
#define H8LR_LIB_INF_ESPERA_INF 2

/**
 * Estado que indica error al ejecutar la inferencia
 */
#define H8LR_LIB_INF_ERRADA 3

/**
 * Codigo de error que indica no se pudo crear el dispotivio virtual
 */
#define H8LR_LIB_ERR_CREAR_DISPOSITIVO 1

/**
 * Codigo de error que indica no se pudo cargar el modelo de red
 */
#define H8LR_LIB_ERR_CARGA_MODELO 2

/**
 * Codigo de error que indica no se pudo  obtener informacion del 
 * stream de salida
 */
#define H8LR_LIB_ERR_OBT_INFO_STREAM_SALIDA 3

/**
 * Codigo de error que indica no se pudo crear los parametros por defecto
 */
#define H8LR_LIB_ERR_CREA_PARAM_DEFECTO 4

/**
 * Codigo de error que indica no se pudo crear el grupo de redes
 */
#define H8LR_LIB_ERR_CREAR_GRUPO_RED 5

/**
 * Codigo de error que indica el grupo de redes no tien una sola red
 */
#define H8LR_LIB_ERR_GRUP_SIN_UNA_RED 6

/**
 * Codigo de error que indica no se pudo crear el stream de entrada de datos
 */
#define H8LR_LIB_ERR_CREAR_STREAM_ENTRADA 7

/**
 * Codigo de error que indica nose pudo crear el stream de salida
 */
#define H8LR_LIB_ERR_CREAR_STREAM_SALIDA 8

/**
 * Codigo de error que indica que no se pudo crear el pipeline de ejecucion
 */
#define H8LR_LIB_ERR_CREAR_PIPELINE_INF 9

/**
 * Codigo de error que indica que no se pudo obtener el stream de ingreso de datos
 */
#define H8LR_LIB_ERR_GET_STREAM_ENTRADA 10




/**
 * Clase que ejecuta redes neuronales con el
 * acelerador Hilo 8L
 */
class Hailo8LRunner : public GThread
{
    public:
        
        /**
         * Contrustor
         */
        Hailo8LRunner();

        /**
         * Destructor
         */
        ~Hailo8LRunner();  

        /**
         * Referencia a un dispositivo
         */
        hailort::Expected<std::unique_ptr<hailort::VDevice>> *device;

        /**
         * Carga una red neuronal
         * 
         *  path:
         *      Ruta de la red
         * 
         *  Retorno:
         *      0 : En caso de exito 
         *      Diferente a 0 en base a constantes definidas Hailo8LRunner_ERROR_CARGA_XXXXXX 
         */
        int cargarRed( string path );

        /**
         * Funcion que se ejecuta en paralelo desde el thread
         */
        void runThread() override;
    
        /**
         * Retorna el codigo del ultimo error
         */
        int getError();

        /**
         * Ejecuta la inferencia sobre los datos que se pasan
         * 
         *      data : puntero con los datos
         * 
         *      dataSize : cantidad de datos que tiene el puntero
         */
        bool ejecutarInferencia( uint8_t *data, uint32_t dataSize );

        /**
         * Imprime la informacion sobre el stream de salida
         */
        void printInfoStreamSalida();

        /**
         * Convierte un valor quantizado a float
         */
        float dequantize(uint8_t value, hailo_quant_info_t *quant_info );

        /**
         * Retorna infomacion sobre los streams de salida
         */
        std::vector<hailo_vstream_info_t> getOutVStreamsInfo();

        /**
         * Retorna la informacion de una capa dado su nombre
         */
        hailo_vstream_info_t getOutVsTramInfo( string nomCapa );

        /**
         * Retorna un stream de salida dado su nombre
         */
        vector<uint8_t> getOutputStream( string nombre );

    private:

        /**
         * Informacion de los streams de salida
         */
        std::vector<hailo_vstream_info_t> outVStreamsInfo;
       
        /**
         * Streams en los que se cargan los datos 
         */
        std::vector<std::reference_wrapper<hailort::InputVStream>> input_vstreams;

        /**
         * Ultimo ouputdata empleado
         */
        std::map<std::string, std::vector<uint8_t>> lastOutputData;

        /**
         * Establece el estado de ejecucion
         */
        void setEstadoEjec( int estado );
       
        /**
         * Retorna el estado de ejecucion
         */
        int getEstadoEjec();

        /**
         * Establece el estado de ejecucion de inferencias
         */
        void setEstadoInf( int estado );

        /**
         * Retorna le esado de ejecucion de inferencias
         */
        int getEstadoInf();


        /**
         * Ruta de la red neuronal
         */
        string pathModelo;

        /**
         * Estado de ejecucion
         */
        int estadoEjec;

        /**
         * Estado de ejecucion de inferencia
         */
        int estadoInferencia;

        /**
         * Error de ejecucion
         */
        int errorEjec;

        /**
         * Puntero al buffer de datos para hacer la inferencia
         */
        uint8_t *inferData;

        /**
         * Cantidad de datos de la data para hacer inferencia
         */
        uint32_t inferDataSize;
};




#endif

