

#ifndef _EXTRACTOR_FACIAL_ARCHIVO_
#define _EXTRACTOR_FACIAL_ARCHIVO_

#include <iostream>
#include <string>

#include "app/EmisorImagenesHTTP.h"
#include "lib/graphics/ImageSource.h"
#include "lib/general/GThread.h"
#include "lib/hailolib/hailo8l.h"
#include "lib/hailolib/DetectorCaraHaloScrfd.h"
#include "lib/hailolib/FaceRecHailo.h"
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/hailolib/IdentificadorPersonasHailo.h"
#include "app/LectorConfig.h"


/**
 * Clase que procesa un archivo de datos y un directorio con fotos para 
 * generar un archivo final que representa una BD de reconocimiento.ADJ_OFFSET_SINGLESHOT
 * 
 * El archivo de datos es un CSV:
 * 
 *      idPersona, nombre, pathFoto
 * 
 * idPersona : ID en un servicio externo con el que se identifica a una persona
 * nombre : Nombre de la persona con el idPersona
 * pathFoto : ruta del archivo JPEG con la foto, que debera estar dentro del directorio de fotos
 * 
 * Al final se genera un archivo con al BD de reconocimiento que tiene el siguiente formato: 
 * 
 * nombrePersona, idPersona, lista valores descriptor facial
 * 
 */
class ExtractorFacialArchivo
{
    public:

        hailort::Expected<std::unique_ptr<hailort::VDevice>> *vdevice = nullptr;
        /**
         * Ruta del archivo de da
         */
        std::string pathArchivoDatos;

        /**
         * Nombre del directorio en el que estan las fotos
         */
        std::string pathFotos;

        /**
         * Ruta del archivo con la BD de personas conocidas
         */
        std::string pathArchivoBD;

        /**
         * Lista de parametros que se configuran a nivel del
         * app web de la aplicacion, es decir son variables
         */
        shared_ptr<GHashMap>lstParamsApp;

        /**
         * PATH del modelo que genera descriptores faciales
         */
        string pathModeloDescFacial;

        /**
         * Nombre de la ultima capa del modelo
         */
        string nombreCapaSalidaRedFacial;

        /**
         * Indica si se debe o no guardar las caras alineadas frontalmente
         * antes de generar un reconocimiento facial
         */
        bool guardarCarasFrontalesAlineadas;

         /**
         * Indica si se debe previsualizar la imagen a la que se le hara el reconocimiento
         */
        bool previsualizaImgReconocimiento;

        /**
         * INdica si se debe esperar una vez que se ha visualizado la imagen de reconocimiento
         */
        bool esperarPrevImgReconocimiento;


        /**
         * Indica si se deben centrar los rostros detectados en un cuadrado
         */
        string encuadrarRostros;

        /**
         * Indica si se debe o no invertira verticalmente las imagenes
         */
        bool invertirVertical;
        
        /**
         * Ancho minimo de un rostro para ser procesado
         */
        int anchoRostroMinimo;

        /**
         * Altura minima de un rostro para ser procesada
         */
        int alturaRostroMinima;       

        /**
         * Ancho con el que se procesa la deteccion de un rostro
         */
        int anchoRostroDet;

        /**
         * Altura con la que se procesa la deteccion de un rostro
         */
        int alturaRostroDet;

        /**
         * Resolucion horizontal de la camara
         */
        int anchoCamara;

        /**
         * Altura de la camara
         */
        int alturaCamara;

        /**
         * Pixeles de suavizado para emular suavizado de camara IP
         * 0 No suaviza nada
         */
        int pixelSuavizado;

        /**
         * Parametro que se aplica al rostro antes de realizar una deteccion
         */
        int paramContraste;

        /**
         * Porcentaje de reduccion de resolucion para eliminar detalles
         * 1 no reduce nada, son valores de 0 a 1
         */
        float resizeSuavizado;

        /**
         * Compresion del JPEG para agregar artefactos de compresion
         * valores de 0 a 100, 100 no comprime nada
         */
        float jpegSuavizado;

        /**
         * Altura de la imagen sobre la que se hace la deteccion y se aplican los
         * filtros anteriores.
         * 0 No modifica la imagen
         */
        int alturaImagenBase;
       
        /**
         * Distancia maxima entre dos imagenes para considerarlas
         * de la misma persona
         */
        float toleranciaIden;

        /**
         * Tolerancia o acuracy para las detecciones
         */
        float toleranciaDetec;   


        /**
         * Constructor
         */
        ExtractorFacialArchivo();
        
               
        /**
         * Ejecuta el proceso
         */
        void ejecutar();

        /**
         * Método de configuración de parámetros
         */
        void configure() {
            const auto cfg = LectorConfig::getInstance().getParams();
    
            alturaRostroMinima = cfg->getStringLong("anchoMinCaraRec", 90);
            anchoRostroMinimo  = cfg->getStringLong("alturaMinCaraRec", 90);
            toleranciaDetec    = cfg->getStringDouble("presicionDeteccion", 0.40);
            toleranciaIden     = cfg->getStringDouble("deltaRostroMax", 0.60);
    
            paramContraste     = cfg->getStringDouble("paramContraste", 0);
            pixelSuavizado     = cfg->getStringLong("pixelSuavizado", 5);
            alturaImagenBase   = cfg->getStringLong("alturaImagenBase", 0);
            jpegSuavizado      = cfg->getStringLong("jpegSuavizado", 100);
            resizeSuavizado    = cfg->getStringDouble("resizeSuavizado", 1);
    
            guardarCarasFrontalesAlineadas =
                cfg->getStringBool("guardarCarasFrontalesAlineadas", false);
    
            encuadrarRostros = cfg->getString("encuadrarRostros");
            pathModeloDescFacial = cfg->getString("pathModeloDescFacial");
            nombreCapaSalidaRedFacial = cfg->getString("nombreUltimaCapaRedNeuronal");
    
            previsualizaImgReconocimiento =
                cfg->getStringBool("previsualizaImgReconocimiento", false);
            esperarPrevImgReconocimiento =
                cfg->getStringBool("esperarPrevImgReconocimiento", false);
        }

    private:

        /**
         * L2-normaliza in-place (si el vector es todo ceros, lo deja tal cual)
         */        
        void l2_normalize(SIMD_TYPE *v);

        // coseno entre dos vectores (si ya están L2-normalizados, es el dot product)
        float cosine_sim(SIMD_TYPE *a, SIMD_TYPE *b);

        /**
         * Fusiona dos embeddings (e_orig y e_flip) en uno solo:
         * 1) L2-normaliza ambos
         * 2) Promedia con pesos (w_orig, w_flip)
         * 3) L2-normaliza el resultado
         * Si la similitud coseno entre ambos < min_cos_to_merge, devuelve solo e_orig normalizado.
         *
         * @param e_orig  embedding del rostro original
         * @param e_flip  embedding del mismo rostro con imagen espejada
         * @param w_orig  peso del embedding original (default 0.5)
         * @param w_flip  peso del embedding flip (default 0.5)
         * @param min_cos_to_merge  umbral de similitud para fusionar (p.ej., 0.3–0.4)
         */
        void fuse_flip_embeddings(SIMD_TYPE * e_orig, SIMD_TYPE *e_flip, SIMD_TYPE *e_unificado,
            float w_orig = 0.5f,
            float w_flip = 0.5f,
            float min_cos_to_merge = 0.6f);
};

#endif 