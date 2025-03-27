
#ifndef _IDENTIFICADOR_FACIAL_
#define _IDENTIFICADOR_FACIAL_

#include "lib/graphics/ImageSource.h"
#include "lib/deeplearning/DetectionTracker.h"
#include "lib/deeplearning/DetectorBO.h"
#include "lib/facerec/facedetection.h"
#include "lib/general/GThread.h"
#include "app/ThProcDescFaciales.h"
#include "app/EmisorImagenesHTTP.h"
#include "app/ThreadGeneradorEventosDet.h"


class ServidorHttpImagenes;
// class ThProcDescFaciales;

/**
 * Clase que se encarga de obtener imagenes desde un proveedor de imagenes
 * hacer la deteccion de rostros y hacer la identificacion facial del mismo
 */
class IdentificadorFacial 
{
    public:

        /**
         * Creador de imagen
         */
        shared_ptr<ImageSourceFactory> imgFactory;

        /**
         * Objeto que detecta las caras
         */
        shared_ptr<DetectorObjetos> detectorCaras;

        /**
         * Lista de parametros que se configuran a nivel del
         * app web de la aplicacion, es decir son variables
         */
        shared_ptr<GHashMap>lstParamsApp;

        /**
         * Maximo de rostros por thread que caclula descripciones
         * de rostros
         */
        int maxRosPorThCalcDesc;

        /**
         * Ancho minimo de un rostro para ser procesado
         */
        int anchoRostroMinimo;

        /**
         * Altura minima de un rostro para ser procesada
         */
        int alturaRostroMinima;

        /**
         * Ancho minimo de un rostro para ser visualizado
         */
        int anchoRostroMinVis;

        /**
         * Altura minima de un rostro para ser visualizado
         */
        int alturaRostroMinVis;

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
         * Ancho con el que se visualiza la imagen
         */
        int anchoVisualiza;

        /**
         * Altura visualiza
         */
        int alturaVisualiza;

        /**
         * Factor horizontal para visualizar datos
         */
        float factorHorVisualiza;

        /**
         * Factor vertical para visualizar datos
         */
        float factorVerVisualiza;

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
         * Tiempo en milisegundos que debe haber pasado
         * para recalcular el descriptor facial de un rostro
         */
        long long tiempoRenovacionMs;

        /**
         * Thread calculador de descriptores de rostros
         */
        ThreadCalculaFaceDescriptor thCalculadorDesc;
    
        /**
         * Thread generador de eventos por cada deteccion
         */
        // ThreadGeneradorEventosDet thGenEventos; 

        /**
         * Servidor WEB con el que se trabaja
         */
        shared_ptr<ServidorHttpImagenes> servidorWeb;

        /**
         * Referencia al objeto que genera imagenes
         */
        shared_ptr<ImageSource> imageSource;

        /**
         * Constructor
         */
        IdentificadorFacial();

        /**
         * Procesa eventos del mouse
         */
        void mouseEvent( int event, int x, int y, void *userdata );

        /**
         * Inicia el proceso
         */
        void iniciar();

        /**
         * Carga los rostros conocidos desde un archivo de texto
         * Donde cada fila tiene los datos de una persona conocida, separados por coma
         * El primer elemento de una fila es un texto que identifica a la persona.
         * Desde el 2do elemento hasta el final de un archivo de texto se tienen
         * los valores del decripctor del rostro.
         */
        void cargaRostrosConocidos( string path );

        /**
         * Ajusta los parametros de captura de las imagenes desde la camara
         */
        void ajustaVideo( double brillo, double contraste );


        /**
         * Lee los parametros de ejecucion 
         */
        void leeParametros( string path );

        /**
         * Guarda los parametros modificados
         */
        void guardaParametros();

        /**
         * Metodo invocado cuando el thread que calcula las descripciones de rostros ha terminado
         * 
         *  lstFaceDet : lista de detecciones procesada
         */
        void calculoDescFinalizado( GLinkedList<FaceDetection> lstFaceDet );
                    
    private:

        /**
         * Ruta del archivo de parametros
         */
        string pathParametros;

        /**
         * Thread procesador de descriptores faciales
         */
        ThProcDescFaciales thProcDescFaciales;

        /**
         * Lista con los rostros detectados visibles
         */
        GLinkedList<FaceDetection> lstRostros;

        /**
         * Lista de rostros pendientes de deteccion, visibles y no visibles
         */
        GLinkedList<FaceDetection> lstRostrosPend;

        /**
         * Lista con rostros conocidos con los que se comparan
         * los rosotros a los que se les ha obtenido una descripcion
         * para luego asignarles un texto descriptor 
         */
        shared_ptr<GVector> lstRostrosConocidos;

        /**
         * Lista de detecciones anuladas por el tracking
         * Son instancias de DeteccionVO
         */
        shared_ptr<GVector> lstBorradosTracking;

        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Dibuja los rostros
         */
        void dibujaRostros( GImage imagen );
        

        /**
         * Solicita al Thread que genera eventos de notificacion
         * que se notifique al servidor la ocurrencia de detecciones
         * 
         *  lstFaceDetProc : lista on las detecciones (Instancias de FaceDetection) a las que se les hizo el calculo
         *              de descriptores faciales
         */
        void generaEventoDetecciones( GLinkedList<FaceDetection> lstFaceDetProc );

        /**
         * Procesa las detecciones actuales
         * 
         * lstDet : lista de detecciones que se deben procesar
         * 
         * imagen : imagen desde la que se obtuvieron las detecciones
         * 
         * imagenVisor : imagen del visor que se envia cuando ocurre un evento
         */
        void procesaDetecciones( GLinkedList<DeteccionVO> lstDet, GImage imagen, GImage imagenVisor );

        /**
         * Solicita calcular la descripcion de los ultimos N rostros de la 
         * lista de rostros pendientes de descripcion.
         * 
         * numPeticiones: cantidad de rostros que se envian a calcular
         * 
         * imagen : imagen desde la que se debe sacar los rostros
         */
        void solicitaCalcularFaceDesc( int numPeticiones, GImage imagen);

        /**
         * Busca si un rostro conocido coincide con un rostro detectado
         */
        void buscaRostroConocido( FaceDetection *face );
        
};

#endif