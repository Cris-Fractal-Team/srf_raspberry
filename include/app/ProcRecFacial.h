

#ifndef _PROC_REC_FACIAL
#define _PROC_REC_FACIAL

#include "app/EmisorImagenesHTTP.h"
#include "app/GeneradorEventos.h"
#include "app/GeneradorPings.h"
#include "lib/graphics/ImageSource.h"
#include "lib/general/GThread.h"
#include "lib/hailolib/hailo8l.h"
#include "lib/hailolib/DetectorCaraHaloScrfd.h"
#include "lib/hailolib/FaceRecHailo.h"
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/hailolib/IdentificadorPersonasHailo.h"

/**
 * Clase que representa todo el proceso que hace el reconocimiento facial
 */
class ProcesoRecFacial
{
    public:

        /**
         * Codigo unico del equipo, coincide con el ID unico del Raspberry PI
         */
        string idEquipo;

        /**
         * Contador de cuadros
         */
        long long numCuadro;

        /**
         * Generador de pings que envia los datos en paralelo
         */
        GeneradorPings generadorPings;

        /**
         * Generador de eventos que envia los datos en paralelo
         */
        GeneradorEventos generadorEventos;

        /**
         * Lista de parametros que se configuran a nivel del
         * app web de la aplicacion, es decir son variables
         */
        shared_ptr<GHashMap>lstParamsApp;

        /**
         * Universo de personas, gestiona la busqueda de las mismas
         */
        IdentificadorPerHailo universoPersonas;

        /**
         * Numero total de eventos 
         */
        int numIndentificacionesMin;

        /**
         * Indica si se debe o no extraer una zona cuadrada en la que este
         * centrado el rostro detectado
         */
        string encuadrarRostros;

        /**
         * Ruta del modelo de red neuronal a usar para generar la descripcion facial
         */
        string pathModeloDescFacial;

        /**
         * Nombre de la ultima capa o capa de salida del modelo 
         * para calcular el descriptor facial
         */
        string nombreCapaSalidaRedFacial;

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
         * Distancia maxima entre dos imagenes para considerarlas
         * de la misma persona
         */
        float toleranciaIden;

        /**
         * Tolerancia o acuracy para las detecciones
         */
        float toleranciaDetec;   

        /**
         * Factor para calcular contraste 
         */
        float paramContraste;
        
        /**
         * Tiempo en el que se debe reportar a una persona si es que ella esta mucho
         * tiempo al frente de la camara
         * Este tiempo es en segundos
         */
        long long tiempoReEvento;

        /**
         * Indica si se debe usar o no distancia euclideana para calcular similaridad
         */
        bool usarDistEuclideana;

        /**
         * ID del siguiente desconocido
         */
        long long idDesconocidoSgte;

        /**
         * Cantidad de segundos maxima que puede estar una persona no
         * reconocida sin que se le vuelva a hacer una deteccion
         */
        int tiempoMaxNoReconocido;

        /**
         * Factor de compresion de JPEG el valor va de 0 a 100 , 100 maxima calidad
         */
        int compresionJpeg;

        /**
         * Indica si se debe o no reportar personas desconocidas
         */
        bool reportarDesconocidos;

        /**
         * Ruta del archivo de parametros
         */
        string pathParametros;

        /**
         * Posicion Inicial X de la region de interes
         */
        int regionInteresInicioX;

        /**
         * Posicion Final X de la region de interes
         */
        int regionInteresFinX;
        
        /**
         * Servidor web que permite configurar el dispositivo
         */
        shared_ptr<ServidorHttpImagenes> servidorWeb;

        /**
         * Referencia al objeto que genera imagenes
         */
        shared_ptr<ImageSource> imageSource;

        /**
         * Constructor
         */
        ProcesoRecFacial();

        /**
         * Bucle que realiza todo el flujo
         */
        void iniciar();

        /**
         * Establece el creador de imagenes
         */
        void setImageFactory( shared_ptr<ImageSourceFactory> factory );

        /**
         * Procesa el evento click sobre la pantalla
         */
        static void onMouse( int event, int x, int y, int flags, void *userdata );

        /**
         * Guarda los parametros de configuracion
         */
        void guardaParametros();

        /**
         * Lee los parametros de ejecucion
         */
        void leeParametros( string path );

    private:        
        
        /**
         * Lista con las ultimas caras detectadas
         */
        static vector<TrackedDetectionHailo *> lstUltCarasDet;
        
        /**
         * Indica si debe o no finalizar el bucle de ejecucion
         */
        bool finalizar;

        /**
         * Indica si se presento un error al iniciar el proceso
         */
        bool errorInicial;

        /**
         * Detector de rostros
         */
        DetectorCarasHailoSCRFD detector;        
    
        /**
         * Generador de descriptres faciales
         */
        FaceRecHailo generadorDesc;
        
        /**
         * Mutex para areas criticas
         */
        std::mutex mtx;

        /**
         * Analizador y seguidor de detecciones
         */
        DetectionTrackerHailo detTracker;

        /**
         * Calcula los descriptores faciales y califica los objetos
         * Retorne una lista de detecciones clasificados
         * 
         *      generador:
         *          Objeto con el que se generan las descripciones faciales
         * 
         *      foto :
         *          Imagen desde la que se extrae la cara
         * 
         *      lstRostros: 
         *          Lista de detecciones hechas en la foto
         */
        void calculaDescriptores( FaceRecHailo *generador, GImage foto, vector<TrackedDetectionHailo *> *lstRostros );

        /**
         * Analiza todas las caras de la lista de rostros detectados
         * para ver si alguno coincide con una persona conocida
         */
        void identificarPersonas( vector<TrackedDetectionHailo *> *lstRostros );

        /**
         * Dibuja las caras encontradas
         * Retorna la imagen que se debe enviar al visor
         */
        GImage dibujaCaras( GImage imagen, vector<TrackedDetectionHailo *> *lstCaras  );

        /**
         * Notifica en caso se necesarios a un servidor de la ocurrencia de
         * una o varias detecciones
         */
        void notificaDetecciones( GImage imagen, vector<TrackedDetectionHailo *> *lstCaras );

        /**
         * Factor de la escala X para la imagen de visualizacion o la imagen que se envia al servidor
         */
        double factorEscalaVisualizaX;

        /**
         * Factor de la escala X para la imagen de visualizacion o la imagen que se envia al servidor
         */
        double factorEscalaVisualizaY;

        /**
         * Inicializa el generador de descriptores faciales
         */
        bool inicializaGeneradorDescriptores( hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr );

        /**
         * Inicializa el detector de rostros
         * Returna true si fue exitoso
         */
        bool inicializaDetectorRostros( hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr );
};


#endif