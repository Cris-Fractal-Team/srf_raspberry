
#include <stdio.h>
#include <cmath>
#include <fstream>
#include <iostream>

#include "lib/general/GObject.h"


#ifndef gdibujo
#define gdibujo


#define GDIBUJO_USAR_OPENCV 1


#ifdef GDIBUJO_USAR_OPENCV

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

#endif


/**
 * Constante que identifica el tipo de dato de imagenes RGB o colores
 */
#define GDIBUJO_IMAGEN_MODO_RGB 1

/**
 * Constante que identifica el tipo de dato de imagenes en modo de grises
 */
#define GDIBUJO_IMAGEN_MODO_GRAY 2

/**
 * Constante que identifica el tipo de codificacion de una imagen en formato JPEG
 */
#define GDIBUJO_ENCODE_JPEG 1

/**
 * Constante que identifica el tipo de codificacion de una imagen en formato PNG
 */
#define GDIBUJO_ENCODE_PNG 2

/**
 * Constante que identifica el fonto HELVETICA
 */
#define GDIBUJO_FONT_HELVETICA 1

/**
 * Constante que identifica el fonto HELVETICA
 */
#define GDIBUJO_FONT_SANS_SERIF 2

/**
 * Constante que identifica un FLIP horizontal
 */
#define GDIBUJO_FLIP_HOR 1

/**
 * Constante que identifica un FLIP vertical
 */
#define GDIBUJO_FLIP_VER 2

/**
 * Constante que identifica un FLIP horizontal y vertical
 */
#define GDIBUJO_FLIP_HOR_VER 3



/**
 * Representa un color
 */
class GColor : public GObject
{
    public:

        /**
         * Color del componente rojo
         */
        unsigned char cred;

        /**
         * Color del componente verde
         */
        unsigned char cgreen;

        /**
         * Color del componente azul
         */
        unsigned char cblue;

        /**
         * Constructor, por defecto el color es negro
         */
        GColor();

        /**
         * Constructor
         */
        GColor( unsigned char red, unsigned char green, unsigned char blue );
};

/**
 * Representa un punto
 */
class GPoint : public GObject
{
    public:

        /**
         * Coordenada X del punto
         */
        int x;

        /**
         * Coordenada y del punto
         */
        int y;

        /**
         * Constructor, por defecto el punto esta en las coordenadas X=0, Y=0
         */
        GPoint();

        /**
         * Constructor, por defecto el punto esta en las coordenadas X=0, Y=0
         */
        GPoint( int px, int py);

        /**
         * Calcula la distancia hacia otro punto
         */
        int calculaDistancia( GPoint punto );
        

        /**
         * Mueve relativamente al punto, incrementando valores a las coordenadas x,y
         * Retonra una copia del punto
         */
        GPoint mover( int dx, int dy );
};


/**
 * Representa un rectangulo
 */
class GRect : public GObject
{
    public:

        /**
         * Coordenada X del punto superior izquierdo
         */
        int x1;

        /**
         * Coordenada Y del punto superior izquierdo
         */
        int y1;

        /**
         * Coordenada X del punto inferior derecho
         */
        int x2;

        /**
         * Coordenada Y del punto inferior derecha
         */
        int y2;

        /**
         * Constructor. Por defecto el rectangulo tiene sus coordenadas en (0,0),(100,100)
         */
        GRect();

        /**
         * Constructor
         * 
         * param px1 : Coordenada X del punto superior izquierdo
         * 
         * param py1 : Coordenada Y del punto superior izquierdo
         * 
         * param px2 : Coordenada X del punto inferior derecho
         * 
         * param py2 : Coordenada Y del punto inferior derecho
         */
        GRect( int px1, int py1, int px2, int py2 );

        /**
         * Multiplica las coordenadas X e Y por dos factores
         * para escalar la recta
         */
        void escala( float factorX, float factorY );

        /**
         * Retorna el ancho del rectangulo
         */
        int getWidth();

        /**
         * Retorna la altura del rectangulo
         */
        int getHeight();

        /**
         * Valida si un punto esta dentro del rectangulo
         */
        bool contains( int x, int y );

        /**
         * Calcula la interseccion de dos rectangulos.
         * En caso no se intersecten el ancho o la altura del rectangulo resultante sera menor a 0
         */
        GRect intersect( GRect rect );

        /**
         * Retorna el punto medio del rectangulo
         */
        GPoint getCenter();

        /**
         * Retorna un clone de la region
         */
        GRect getClone();
};


/**
 * Clase que representa una imagen
 */
class GImage : public GObject
{
    public:        

        /**
         * Codigo del formato de la imagen, definido en las constantes GDIBUJO_IMAGEN_MODO_XXXXXX
         */
        int formato;

        /**
         * Ancho en pixeles de la imagen
         */
        int ancho;

        /**
         * Ancho en pixeles de la imagen
         */
        int altura;      

        #ifdef GDIBUJO_USAR_OPENCV
        Mat imagenOpencv;
        #endif

        /**
         * Constructor
         */
        GImage();

        #ifdef GDIBUJO_USAR_OPENCV
        /**
         * Constructor
         */
        GImage( Mat imgOpenCV );
        #endif

        /**
         * Destructor
         */
        ~GImage();

        /**
         * Crea una imagen en blanco con un formato dado
         * 
         * param ancho: ancho en pixeles de la imagen
         * 
         * param altura: altura en pixeles de la imagen
         * 
         * param formato : codigo del formato de la imagen
         */
        GImage( int ancho, int altura, int formato );

        /**
         * Aplica una mejora de contraste a la imagen
         *
         *  param clipLimit: Ajustar con valores entre 2 y 4 
         * 
         */
        void aplicarCLAHE( float clipLimit);

        /**
         * Inicializa la imagen desde un buffer que contiene los datos de un archivo JPEG
         */
        void decodeJPEGImage( uint8_t *buffer, uint32_t longitud );

        /**
         * Genera una copia de la imagen
         */
        GImage clone();

        /**
         * Retorna una nueva imagen que es una version escalada de la imagen actual
         * 
         * param ancho : ancho de la nueva imagen
         * 
         * param altura : altura de la nueva imagen
         */
        GImage cloneResize( int ancho, int altura );

        /**
         * Retorna una nueva imagen que es una version escalada de la imagen actual
         * Esta version mantiene las proporciones de la imagen
         * 
         * param ancho : ancho de la nueva imagen
         * 
         * param altura : altura de la nueva imagen
         */
        GImage clonePropResize( int ancho, int altura );

        /**
         * Retorna una nueva version de la imagen pero con un formato diferente.
         * 
         * param formato: codigo del formato de la nueva imagen
         */
        GImage cloneConvert( int formato );        

        /**
         * Retorna la data de imagenes
         */
        unsigned char *getCharData();

        /**
         * Establece los datos de la imagen, desde un buffer que tiene la informacion a colores
         * 
         * param buffer: puntero donde esta la data
         * 
         * param ancho: ancho de la imagen
         * 
         * param altura: altura de la imagen
         */
        void setCharData( char *buffer, int ancho, int altura );

        /**
         * Retorna una imagen a partir de una seccion rectangular
         */
        GImage getRect( int x1, int y1, int xy, int y2);

        /**
         * Retorna una imagen a partir de una seccion rectangular
         */
        GImage getRect( GRect rect );

        /**
         * Retorna true en caso la imagen esta vacia o no diene data
         */
        bool isEmpty();

        /**
         * Elimina el motion blur
         */
        void deblurMotionWiener( int len = 15, double angle = 0.0, double K = 0.01);

        /**
         * Aclara un poco la imagen
         */
        void sharpenUnsharp( float amount=1.0f, float radius=1.2f, float threshold=0.0f);

        /**
         * Valida si la imagen esta desenfocada
         * 
         *  param tolerancia: un valor menor a la tolerancia indica desenfocada.
         */
        bool estaDesenfocada( double tolerancia = 80 );

        /**
         * Valida si la imagen esta ruidosa o granulada por oscuridad
         * 
         *  param dark_abs : valor que indica si un pixel de la version blanco y negro debe ser considerado
         *      oscuro o no.
         * 
         *  param mediaLuminancia: si la luminancia media es menor a este valor se considera ruidosa
         * 
         *  param limiteOscuridad : si el factor de pixeles oscuros (0 a 1) es mayor a este valor se considera ruidosa u oscura
         * 
         *  param limiteEnergia : si el valor de energia es mayor a este limite se considera ruidosa u oscura
         */
        bool estaRuidosa( int dark_abs = 25 , double mediaLuminancia = 60.0, double limiteOscuridad = 0.06 , double limiteEnergia = 9.0 );

    private:

    /**
     * Desplazamiento circular 2D (tipo "roll") para centrar el PSF en (0,0) antes de la DFT
     */
    void circShift(const Mat& src, Mat& dst, int shiftY, int shiftX);

    /**
     * Genera un PSF de "motion blur" (línea anti-aliased) de longitud 'len' y ángulo 'theta' (grados)
     */
    Mat makeMotionPSF(int len, double thetaDeg);

    /** 
     * Convierte PSF (espacio) a OTF (frecuencia) del tamaño de salida 'outSz'
     */
    void psf2otf(const Mat& psf, Mat& otf, Size outSz);
       
    /**
     * Deconvolución de Wiener (1 canal float [0..1])
     */
    void wienerDeconvSingle(const Mat& srcGray32, Mat& dstGray32, const Mat& psf, double K);


};

/**
 * Clase con metodos estaticos que presenta primitivas para manipular imagenes
 */
class GDibujo
{
    public:

        /**
         * Lee una imagen desde disco
         * 
         * param path: Ruta desde la que se lee la imagen
         */
        static GImage read( string path );

        /**
         * Guarda ina imagen en disco.
         * 
         * param imagen: imagen que se guarda
         * 
         * param path : Ruta en la que se guarda el archivo
         * 
         * param calidad : valor de la calidad de compresion de la imagen. Depende del formato, 0 indica usar valor por defecto.
         */
        static void write( GImage imagen, string path, int calidad );

        /**
         * Codifica una imagen en un formato de imagen como JPEG, PNG, etc.
         * 
         * param imagen: imagen que se guarda
         * 
         * param formato : formato de la imagen, definido en las constantes GDIBUJO_ENCODE_XXXX
         * 
         * param calidad : valor de la calidad de compresion de la imagen. Depende del formato, 0 indica usar valor por defecto.
         * 
         * param len : cantidad de bytes del buffer retornado
         * 
         * Retorna putero a un buffer con la data codificada.
         */
        static char * encode( GImage imagen, int formato , int calidad, int &len );

        /**
         * Visualiza la imagen en una ventana
         * 
         * param imagen : imagen que se visualiza
         * 
         * param tituloVentana : titulo de la ventana en la que se visualiza la imagen
         * 
         */
        static void show( GImage imagen, string tituloVentana );

        /**
         * Cierra todas las ventanas
         */
        static void closeAllWindows();

        /**
         * Espera que se precione una tecla hasta por una cantidad de tiempo
         * 
         * param timeMS : tiempo en milisegundos que se espera
         * 
         * retorna el codigo de la tecla o 0 en caso no se haya presionado nada
         */
        static int waitForKey( int timeMS );

        /**
         * Dibuja un rectangulo
         * 
         * param imagen : imagen sobre la que se dibuja
         * 
         * param rectangulo : rectangulo que se dibuja
         * 
         * param color : color del rectangulo
         * 
         * param ancho : ancho del borde del rectangulo
         */
        static void drawRect( GImage imagen, GRect rectangulo, GColor color, int ancho );

        /**
         * Rellena un rectangulo
         * 
         * param imagen : imagen sobre la que se dibuja
         * 
         * param rectangulo : rectangulo que se dibuja
         * 
         * param color : color del rectangulo
         */
        static void fillRect( GImage imagen, GRect rectangulo, GColor color );


        /**
         * Dibuja una linea entre dos puntos.
         * 
         * param imagen : imagen sobre la que se dibuja
         * 
         * param orig : punto de origen de la linea
         * 
         * param dest : punto destina de la linea
         * 
         * param color : color de la linea
         * 
         * param ancho : ancho en pixeles de la linea
         */
        static void drawLine( GImage imagen, GPoint orig, GPoint dest, GColor color, int ancho );

        /**
         * Dibuja una elipse
         * 
         * param imagen : imagen sobre la que se dibuja
         * 
         * param orig : punto central de la elipse
         * 
         * param anchoEjeX : ancho en el eje X de la elipse
         * 
         * param anchoEjeY : ancho en el eje Y de la elipse
         * 
         * param color : color de la linea
         * 
         * param ancho : ancho en pixeles de la linea
         */
        static void drawElipse( GImage imagen, GPoint orig, int anchoEjeX, int anchoEjeY, GColor color, int ancho );

        /**
         * Rellena una elipse
         * 
         * param imagen : imagen sobre la que se dibuja
         * 
         * param orig : punto central de la elipse
         * 
         * param anchoEjeX : ancho en el eje X de la elipse
         * 
         * param anchoEjeY : ancho en el eje Y de la elipse
         * 
         * param color : color de la linea
         * 
         * param ancho : ancho en pixeles de la linea
         */
        static void fillElipse( GImage imagen, GPoint orig, int anchoEjeX, int anchoEjeY, GColor color );

        /**
         * Dibuja una cadena de texto 
         * 
         * param imagen : imagen sobre la que se dibuja
         * 
         * param x : coordenada X del punto desde el que se dibuja el texto
         * 
         * param y : coordenada Y del punto desde el que se dibuja el texto
         * 
         * param texto : cadena que se dibuja
         * 
         * param tipoLetra : codigo del tipo de letra, definido en constantes GDIBUJO_FONT_XXXX
         * 
         * param color : color de la letra
         * 
         * param tamFont : tamano del font, depende de la librearia
         */
        static void drawText( GImage imagen, int x, int y, string texto, int tipoLetra, GColor color, float tamFont, int ancho = 1  );

        /**
         * Hace un reflejo de la imagen
         * 
         * param imagen : iamgen a la que se le hace el flip.
         * 
         * param modo : modo del flip, definido en las constantes GDIBUJO_FLIP_XXXXX
         */
        static GImage flip( GImage imagen, int modo);

        /**
         * Dibuja una imagen pequena o chica en una coordenadas de una imagen grande
         * 
         *  param imagen: imagen grande sobre la que se dibuja la imagen chica
         * 
         *  param x: coordenada X donde se diguja la imagen chica
         * 
         *  param y: coordenada y donde se dibuja la imagen chica
         * 
         *  param imagenChica: imagen que se dibuja sobre imagen.
         */
        static void drawImage( GImage imagen, int x, int y, GImage imagenChica );
};

#endif
