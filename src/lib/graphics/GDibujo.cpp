
#include <math.h>

#include <stdio.h>
#include <cmath>
#include <fstream>
#include <iostream>

#include "lib/graphics/GDibujo.h"

#ifdef GDIBUJO_USAR_OPENCV

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#endif

using namespace cv;

/**
 * Constructor, por defecto el color es negro
 */
GColor::GColor()
{
    cred = 0;
    cgreen = 0;
    cblue = 0;
}

/**
 * Constructor
 */
GColor::GColor( unsigned char red, unsigned char green, unsigned char blue )
{
    cred = red;
    cgreen = green;
    cblue = blue;
}

/**
 * Constructor, por defecto el punto esta en las coordenadas X=0, Y=0
 */
GPoint::GPoint()
{
    x = 0;
    y = 0;
}

/**
 * Constructor, por defecto el punto esta en las coordenadas X=0, Y=0
 */
GPoint::GPoint( int px, int py)
{
    x = px;
    y = py;
}

/**
 * Calcula la distancia hacia otro punto
 */
int GPoint::calculaDistancia( GPoint punto )
{
    long long deltaX,deltaY,suma;

    deltaX = this->x - punto.x;
    deltaY = this->y - punto.y;

    suma = deltaX*deltaX + deltaY*deltaY;

    return sqrt(suma);
}


/**
 * Mueve relativamente al punto, incrementando valores a las coordenadas x,y
 */
GPoint GPoint::mover( int dx, int dy )
{
    GPoint pt;

    x+= dx;
    y+= dy;

    pt.x = x;
    pt.y = y;

    return pt;
}



/**
 * Constructor. Por defecto el rectangulo tiene sus coordenadas en (0,0),(100,100)
 */
GRect::GRect()
{
    x1 = 0;
    y1 = 0;
    x2 = 99;
    y2 = 99;
}

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
GRect::GRect( int px1, int py1, int px2, int py2 )
{
    x1 = px1;
    y1 = py1;
    x2 = px2;
    y2 = py2;
}

/**
 * Retorna el ancho del rectangulo
 */
int GRect::getWidth()
{
    return x2-x1+1;   
}

/**
 * Retorna la altura del rectangulo
 */
int GRect::getHeight()
{
    return y2-y1+1;
}

/**
 * Valida si un punto esta dentro del rectangulo
 */
bool GRect::contains( int x, int y )
{
    if (( ( x >= x1 ) && ( x <= x2) ) && ( ( y >= y1 ) && ( y <= y2 )))
    {
        return true;
    }
    return false;
}

/**
 * Calcula la interseccion de dos rectangulos.
 * En caso no se intersecten el ancho o la altura del rectangulo resultante sera menor a 0
 */
GRect GRect::intersect( GRect rect )
{
    GRect rpta;

    if (( rect.x1 > x2 ) || ( rect.x2 < x1))
    {
        return GRect(0,0,-1,-1);
    }
    if (( rect.y1 > y2 ) || ( rect.y2 < y1))
    {
        return GRect(0,0,-1,-1);
    }

    if ( x1 > rect.x1 ) rpta.x1 = x1;
    else rpta.x1 = rect.x1;

    if ( x2 < rect.x2 ) rpta.x2 = x2;
    else rpta.x2 = rect.x2;

    if ( y1 > rect.y1 ) rpta.y1 = y1;
    else rpta.y1 = rect.y1;

    if ( y2 < rect.y2 ) rpta.y2 = y2;
    else rpta.y2 = rect.y2;

    return rpta;
}   

/**
 * Retorna el punto medio del rectangulo
 */
GPoint GRect::getCenter()
{
    GPoint pt((x1+x2)/2, (y1+y2)/2);
    return pt;
}

/**
 * Multiplica las coordenadas X e Y por dos factores
 * para escalar la recta
 */
void GRect::escala( float factorX, float factorY )
{
    x1 = int(((float)x1)*factorX);
    x2 = int(((float)x2)*factorX);
    y1 = int(((float)y1)*factorY);
    y2 = int(((float)y2)*factorY);
}

/**
 * Retorna un clone de la region
 */
GRect GRect::getClone()
{
    GRect r;

    r.x1 = this->x1;
    r.y1 = this->y1;
    r.x2 = this->x2;
    r.y2 = this->y2;

    return r;
}


/**
 * Constructor
 */
GImage::GImage()
{
    ancho = 0;
    altura = 0;
}

#ifdef GDIBUJO_USAR_OPENCV
/**
 * Constructor
 */
GImage::GImage( Mat imgOpenCV )
{
    imagenOpencv = imgOpenCV;
    ancho = imagenOpencv.cols;
    altura = imagenOpencv.rows;
}
#endif

/**
 * Destructor
 */
GImage::~GImage()
{
    ancho = 0;
    altura = 0;
}

/**
 * Crea una imagen en blanco con un formato dado
 * 
 * param ancho: ancho en pixeles de la imagen
 * 
 * param altura: altura en pixeles de la imagen
 * 
 * param formato : codigo del formato de la imagen
 */
GImage::GImage( int ancho, int altura, int formato )
{
    this->ancho = ancho;
    this->altura = altura;
    this->formato = formato;

    #ifdef GDIBUJO_USAR_OPENCV
    if ( formato == GDIBUJO_IMAGEN_MODO_RGB )
    {
        imagenOpencv = Mat::zeros(altura, ancho, CV_8UC3);
    }
    else
    {
        imagenOpencv = Mat::zeros(altura, ancho, CV_8UC1);
    }                  
    #endif
}

/**
 * Inicializa la imagen desde un buffer que contiene los datos de un archivo JPEG
 */
void GImage::decodeJPEGImage( uint8_t *buffer, uint32_t longitud )
{
    #ifdef GDIBUJO_USAR_OPENCV
    std::vector<uint8_t> bufferJPEG(buffer, buffer + longitud);
    cv::InputArray data(bufferJPEG);

    // Decodificar la imagen desde el buffer
    cv::Mat imagen = cv::imdecode(data, cv::IMREAD_COLOR);

    // Verificar que la imagen se haya cargado correctamente
    if (imagen.empty()) {
        std::cerr << "Error al decodificar la imagen." << std::endl;
    }
    else
    {
        imagenOpencv = imagen;
        ancho = imagenOpencv.cols;
        altura = imagenOpencv.rows;
    }
    #endif
}

/**
 * Retorna true en caso la imagen esta vacia o no diene data
 */
bool GImage::isEmpty()
{
    if (( ancho == 0 ) || ( altura == 0 ))
    {
        return true;
    }
    return false;
}

/**
 * Aplica una mejora de contraste a la imagen
 *
 *  param clipLimit: Ajustar con valores entre 2 y 4 
 * 
 */
void GImage::aplicarCLAHE( float clipLimit)
{
    // Convertir a espacio de color LAB (mejor que YUV o HSV para mejorar contraste)
    cv::Mat lab;
    cv::cvtColor(imagenOpencv, lab, cv::COLOR_BGR2Lab);

    // Dividir en canales L, A y B
    std::vector<cv::Mat> labChannels(3);
    cv::split(lab, labChannels);

    // Aplicar CLAHE solo al canal L (luminosidad)
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(clipLimit); 
    clahe->setTilesGridSize(cv::Size(8, 8)); // Tamaño del grid (8x8 es estándar)
    clahe->apply(labChannels[0], labChannels[0]);

    // Unir los canales y volver a BGR
    cv::merge(labChannels, lab);
    cv::Mat outputBGR;
    cv::cvtColor(lab, outputBGR, cv::COLOR_Lab2BGR);

    imagenOpencv = outputBGR;
}

/**
 * Genera una copia de la imagen
 */
GImage GImage::clone()
{
    GImage rpta;

    #ifdef GDIBUJO_USAR_OPENCV
    if (( imagenOpencv.empty() == false ) && (( imagenOpencv.size[0] > 0 ) && ( imagenOpencv.size[1] > 0  )))
    {
        rpta.imagenOpencv = imagenOpencv.clone();
    }    
    #endif

    rpta.formato = formato;
    rpta.ancho = ancho;
    rpta.altura = altura;

    return rpta;
}

/**
 * Retorna una imagen a partir de una seccion rectangular
 */
GImage GImage::getRect( int x1, int y1, int x2, int y2)
{
    GImage rpta;

    if ( x1 < 0 ) x1 = 0;
    if ( y1 < 0 ) y1 = 0;
    if ( x1 >= ancho ) x1 = ancho-2;
    if ( y1 >= altura) y1 = altura-2;

    if ( x2 < x1 ) x2 = x1+1;
    if ( y2 < y1 ) y2 = y1+1;
  
    if ( x2 >= ancho ) x2 = ancho-1;
    if ( y2 >= altura) y2 = altura-1;

    rpta.ancho = x2-x1+1;
    rpta.altura = y2-y1+1;

    #ifdef GDIBUJO_USAR_OPENCV
    cv::Rect roi(x1,y1,rpta.ancho,rpta.altura);
    cv::Mat seccion = imagenOpencv(roi);
    
    rpta.imagenOpencv = seccion.clone();
    #endif
    
    return rpta;
}

/**
 * Retorna una imagen a partir de una seccion rectangular
 */
GImage GImage::getRect( GRect rect )
{
    return getRect(rect.x1, rect.y1, rect.x2, rect.y2);
}


/**
 * Retorna una nueva imagen que es una version escalada de la imagen actual
 * 
 * param anchoNuevo : ancho de la nueva imagen
 * 
 * param alturaNuevo : altura de la nueva imagen
 */
GImage GImage::cloneResize( int anchoNuevo, int alturaNuevo )
{
    GImage rpta;

    if (( ancho == 0 ) || ( altura == 0 ))
    {
        return rpta;
    }

    #ifdef GDIBUJO_USAR_OPENCV
    cv::resize(imagenOpencv, rpta.imagenOpencv, Size(anchoNuevo,alturaNuevo));
    #endif

    rpta.formato = formato;
    rpta.ancho = anchoNuevo;
    rpta.altura = alturaNuevo;

    return rpta;
}


/**
 * Retorna una nueva imagen que es una version escalada de la imagen actual
 * Esta version mantiene las proporciones de la imagen
 * 
 * param ancho : ancho de la nueva imagen
 * 
 * param altura : altura de la nueva imagen
 */
GImage GImage::clonePropResize( int ancho, int altura )
{
    int deltax,deltay;
    float factor;
    
    deltax = abs(this->ancho - ancho);
    deltay = abs(this->altura - altura);

    if ( deltax < deltay )
    {
        factor = ((float)ancho) / ((float)this->ancho);
        altura = (int)(((float)this->altura)*factor);
    }
    else
    {
        factor = ((float)altura) / ((float)this->altura);
        ancho = int(((float)this->ancho)*factor);
    }

    return cloneResize(ancho, altura);
}



/**
 * Retorna una nueva version de la imagen pero con un formato diferente.
 * 
 * param formato: codigo del formato de la nueva imagen
 */
GImage GImage::cloneConvert( int formato )
{
    GImage rpta;

    #ifdef GDIBUJO_USAR_OPENCV
    
    if ( formato == GDIBUJO_IMAGEN_MODO_RGB )
    {
        if ( formato == GDIBUJO_IMAGEN_MODO_GRAY )
        {
            cv::cvtColor(imagenOpencv, rpta.imagenOpencv, COLOR_BGR2GRAY);
        }
    }
    else
    if ( formato == GDIBUJO_IMAGEN_MODO_GRAY )
    {
        if ( formato == GDIBUJO_IMAGEN_MODO_RGB )
        {
            cv::cvtColor(imagenOpencv, rpta.imagenOpencv, COLOR_GRAY2BGR);
        }
    }

    #endif

    rpta.formato = formato;
    rpta.ancho = ancho;
    rpta.altura = altura;

    return rpta;
}

/**
 * Retorna la data de imagenes
 */
unsigned char *GImage::getCharData()
{
    #ifdef GDIBUJO_USAR_OPENCV

    return (unsigned char*)imagenOpencv.data;

    #endif
}

/**
 * Establece los datos de la imagen, desde un buffer que tiene la informacion a colores
 * 
 * param buffer: puntero donde esta la data
 * 
 * param ancho: ancho de la imagen
 * 
 * param altura: altura de la imagen
 */
void GImage::setCharData( char *buffer, int ancho, int altura )
{
    #ifdef GDIBUJO_USAR_OPENCV

    imagenOpencv = cv::Mat(altura,ancho,CV_8UC3, (unsigned*)buffer);

    #endif
}

 /**
 * Desplazamiento circular 2D (tipo "roll") para centrar el PSF en (0,0) antes de la DFT
 */
void GImage::circShift(const Mat& src, Mat& dst, int shiftY, int shiftX)
{
    dst.create(src.size(), src.type());
    int rows = src.rows, cols = src.cols;

    shiftY = ((shiftY % rows) + rows) % rows; // normaliza
    shiftX = ((shiftX % cols) + cols) % cols;

    Mat q0(src, Rect(0,          0,           cols-shiftX, rows-shiftY));
    Mat q1(src, Rect(cols-shiftX,0,           shiftX,      rows-shiftY));
    Mat q2(src, Rect(0,          rows-shiftY, cols-shiftX, shiftY     ));
    Mat q3(src, Rect(cols-shiftX,rows-shiftY, shiftX,      shiftY     ));

    Mat d0(dst, Rect(shiftX,     shiftY,      cols-shiftX, rows-shiftY));
    Mat d1(dst, Rect(0,          shiftY,      shiftX,      rows-shiftY));
    Mat d2(dst, Rect(shiftX,     0,           cols-shiftX, shiftY     ));
    Mat d3(dst, Rect(0,          0,           shiftX,      shiftY     ));

    q0.copyTo(d0); q1.copyTo(d1); q2.copyTo(d2); q3.copyTo(d3);
}

/**
 * Genera un PSF de "motion blur" (línea anti-aliased) de longitud 'len' y ángulo 'theta' (grados)
 */
Mat GImage::makeMotionPSF(int len, double thetaDeg)
{
    int k = std::max(3, (len | 1));             // tamaño de kernel (impar)
    Mat psf = Mat::zeros(k, k, CV_32F);

    // Línea horizontal centrada de longitud 'len'
    int y = k/2;
    int x0 = (k - len)/2;
    int x1 = x0 + len - 1;
    line(psf, Point(x0, y), Point(x1, y), Scalar::all(1.f), 1, LINE_AA);

    // Rotar a 'thetaDeg'
    Point2f c(k/2.f, k/2.f);
    Mat R = getRotationMatrix2D(c, thetaDeg, 1.0);
    Mat rot;
    warpAffine(psf, rot, R, psf.size(), INTER_AREA, BORDER_CONSTANT, Scalar::all(0));

    // Normalizar (suma = 1)
    rot /= sum(rot)[0] + 1e-12f;
    return rot;
}

/** 
 * Convierte PSF (espacio) a OTF (frecuencia) del tamaño de salida 'outSz'
 */
void GImage::psf2otf(const Mat& psf, Mat& otf, Size outSz)
{
    Mat psfPadded = Mat::zeros(outSz, CV_32F);
    psf.copyTo(psfPadded(Rect(0,0, psf.cols, psf.rows)));
    // Mover el centro del PSF al (0,0) antes de la DFT (shifteo circular)
    Mat psfShifted; circShift(psfPadded, psfShifted, -psf.rows/2, -psf.cols/2);

    Mat planes[] = {psfShifted, Mat::zeros(outSz, CV_32F)};
    merge(planes, 2, otf);
    dft(otf, otf, DFT_COMPLEX_OUTPUT);
}
    
/**
 * Deconvolución de Wiener (1 canal float [0..1])
 */
void GImage::wienerDeconvSingle(const Mat& srcGray32, Mat& dstGray32, const Mat& psf, double K)
{
    // Tamaño óptimo para DFT (pad con reflejo para reducir "ringing" en bordes)
    int R = getOptimalDFTSize(srcGray32.rows);
    int C = getOptimalDFTSize(srcGray32.cols);

    Mat srcPad; copyMakeBorder(srcGray32, srcPad, 0, R - srcGray32.rows,
                               0, C - srcGray32.cols, BORDER_REFLECT);

    // OTF del PSF
    Mat H; psf2otf(psf, H, srcPad.size());

    // DFT de la imagen
    Mat G; {
        Mat planes[] = {srcPad.clone(), Mat::zeros(srcPad.size(), CV_32F)};
        merge(planes, 2, G);
        dft(G, G, DFT_COMPLEX_OUTPUT);
    }

    // |H|^2 + K
    Mat planesH[2]; split(H, planesH);
    Mat mag2; magnitude(planesH[0], planesH[1], mag2); mag2 = mag2.mul(mag2) + (float)K;

    // conj(H)
    Mat Hconj; {
        Mat planesC[] = {planesH[0], -planesH[1]};
        merge(planesC, 2, Hconj);
    }

    // Numerador = G .* conj(H)
    Mat numer; mulSpectrums(G, Hconj, numer, 0); // 0: sin conj extra

    // Resultado en frecuencia = numer / (|H|^2 + K)
    Mat nPlanes[2]; split(numer, nPlanes);
    nPlanes[0] /= mag2; nPlanes[1] /= mag2;
    Mat Fhat; merge(nPlanes, 2, Fhat);

    // IDFT
    Mat rec; dft(Fhat, rec, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);

    // Recortar
    dstGray32 = rec(Rect(0,0, srcGray32.cols, srcGray32.rows)).clone();
}

/**
 * Elimina el motion blur
 */
void GImage::deblurMotionWiener( int len, double angle, double K )
{
    Mat psf = makeMotionPSF(len, angle);

    Mat out;
    if (imagenOpencv.channels() == 1) {
        Mat f; imagenOpencv.convertTo(f, CV_32F, 1.0/255.0);
        Mat r; wienerDeconvSingle(f, r, psf, K);
        r = min(max(r, 0.0f), 1.0f);
        r.convertTo(out, CV_8U, 255.0);
    } else {
        // Procesar cada canal
        std::vector<Mat> ch; split(imagenOpencv, ch);
        std::vector<Mat> chOut(3);
        for (int i=0;i<3;++i) {
            Mat f; ch[i].convertTo(f, CV_32F, 1.0/255.0);
            Mat r; wienerDeconvSingle(f, r, psf, K);
            r = min(max(r, 0.0f), 1.0f);
            r.convertTo(chOut[i], CV_8U, 255.0);
        }
        merge(chOut, out);
    }

    imagenOpencv = out;
}

/**
 * Aclara un poco la imagen
 */
void GImage::sharpenUnsharp( float amount, float radius, float thr)
{
    cv::Mat src32, blur32, high, dst32, out;
    imagenOpencv.convertTo(src32, CV_32F);                          // [0..255] en float
    cv::GaussianBlur(src32, blur32, cv::Size(0,0), std::max(0.01f, radius));

    high = src32 - blur32;                                 // detalle (alta frecuencia)

    if (thr > 0.0f) 
    {
        // Calcula magnitud de detalle y crea máscara 8-bit donde el detalle es BAJO
        cv::Mat absHigh, gray, mask8u;
        cv::absdiff(src32, blur32, absHigh);               // CV_32F
        if (imagenOpencv.channels() == 3) cv::cvtColor(absHigh, gray, cv::COLOR_BGR2GRAY);
        else gray = absHigh;
        cv::compare(gray, cv::Scalar(thr), mask8u, cv::CMP_LT); // 255 donde |detalle| < thr
        high.setTo(0, mask8u);                             // no afilar zonas planas
    }

    dst32 = src32 + amount * high;                         // aplica realce
    cv::min(dst32, 255.0f, dst32);
    cv::max(dst32, 0.0f, dst32);
    dst32.convertTo(out, CV_8U);

    imagenOpencv = out;
}    


/**
 * Valida si la imagen esta desenfocada
 * 
 *  param tolerancia: un valor menor a la tolerancia indica desenfocada .
 */
bool GImage::estaDesenfocada( double tolerancia )
{
    cv::Mat lap;
    cv::Mat gray8;
    
    cv::cvtColor(imagenOpencv, gray8, cv::COLOR_BGR2GRAY );

    cv::Laplacian(gray8, lap, CV_64F);
    cv::Scalar mean, stddev;
    cv::meanStdDev(lap, mean, stddev);

    double variacion_laplaciana = stddev[0] * stddev[0];

    if ( variacion_laplaciana < tolerancia ) return true;

    return false;
}

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
bool GImage::estaRuidosa( int dark_abs, double mediaLuminancia, double limiteOscuridad, double limiteEnergia )
{
    cv::Mat gray8;
    
    cv::cvtColor(imagenOpencv, gray8, cv::COLOR_BGR2GRAY );

    // calcula luminancia media
    cv::Scalar meanY, stdY;
    cv::meanStdDev(gray8, meanY, stdY);
    double mean_y = meanY[0];

    // calcula porcentaje de pixeles muy oscuros
    cv::Mat darkMask; 
    cv::threshold(gray8, darkMask, dark_abs, 255, cv::THRESH_BINARY_INV);
    double dark_frac = cv::countNonZero(darkMask) / double(gray8.total());

    // Calcula energif HF 
    cv::Mat base, residual32f;
    cv::blur(gray8, base, cv::Size(3,3));               // box 3x3 (más rápido que Gauss)
    cv::Mat residual16s; cv::subtract(gray8, base, residual16s, cv::noArray(), CV_16S);
    residual16s.convertTo(residual32f, CV_32F);
    cv::Scalar meanR, stdR; cv::meanStdDev(residual32f, meanR, stdR);
    double hf_std = stdR[0];

    if ( mean_y < mediaLuminancia ) return true;

    if ( dark_frac > limiteOscuridad ) return true;

    if (( hf_std > limiteEnergia ) && ( mean_y < 110.0 )) return true;

    return false;
}


/**
 * Lee una imagen desde disco
 * 
 * param path: Ruta desde la que se lee la imagen
 */
GImage GDibujo::read( string path )
{
    GImage rpta;

    #ifdef GDIBUJO_USAR_OPENCV

    rpta.imagenOpencv = cv::imread(path,IMREAD_COLOR);
    rpta.ancho = rpta.imagenOpencv.cols;
    rpta.altura = rpta.imagenOpencv.rows;

    #endif

    return rpta;
}

/**
 * Guarda ina imagen en disco.
 * 
 * param imagen: imagen que se guarda
 * 
 * param path : Ruta en la que se guarda el archivo
 * 
 * param calidad : valor de la calidad de compresion de la imagen. Depende del formato, 0 indica usar valor por defecto.
 */
void GDibujo::write( GImage imagen, string path, int calidad )
{
    #ifdef GDIBUJO_USAR_OPENCV

    cv::imwrite(path,imagen.imagenOpencv);

    #endif
}

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
char * GDibujo::encode( GImage imagen, int formato , int calidad, int &len  )
{
    #ifdef GDIBUJO_USAR_OPENCV

    std::vector<uchar>buffer;
    std::vector<int> param(2);
    char *rpta;
    char *buf;

    if ( formato == GDIBUJO_ENCODE_JPEG )
    {
        param[0] = cv::IMWRITE_JPEG_QUALITY;
        param[1] = calidad;//default(95) 0-100
        cv::imencode(".jpg",imagen.imagenOpencv,buffer,param);
    }
    else
    {
        cv::imencode(".png",imagen.imagenOpencv,buffer,param);
    }

    len = buffer.size();
    rpta = (char *)malloc(len);

    buf = (char *)buffer.data();
    for(int i=0;i<len;i++) rpta[i] = buf[i];
    
    return rpta;

    #endif
}

/**
 * Visualiza la imagen en una ventana
 * 
 * param imagen : imagen que se visualiza
 * 
 * param tituloVentana : titulo de la ventana en la que se visualiza la imagen
 * 
 */
void GDibujo::show( GImage imagen, string tituloVentana )
{
    if (( imagen.ancho == 0 ) || ( imagen.altura == 0 )) return;
    
    #ifdef GDIBUJO_USAR_OPENCV

    cv::imshow(tituloVentana,imagen.imagenOpencv);

    #endif
}

/**
 * Cierra todas las ventanas
 */
void GDibujo::closeAllWindows()
{
    #ifdef GDIBUJO_USAR_OPENCV

    cv::destroyAllWindows();

    #endif
}

/**
 * Espera que se precione una tecla hasta por una cantidad de tiempo
 * 
 * param timeMS : tiempo en milisegundos que se espera
 * 
 * retorna el codigo de la tecla o 0 en caso no se haya presionado nada
 */
int GDibujo::waitForKey( int timeMS )
{
    #ifdef GDIBUJO_USAR_OPENCV

    return cv::waitKey(timeMS);

    #endif
}

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
void GDibujo::drawRect( GImage imagen, GRect rectangulo, GColor color, int ancho )
{
    #ifdef GDIBUJO_USAR_OPENCV

    cv::rectangle(imagen.imagenOpencv,Rect(rectangulo.x1,rectangulo.y1,rectangulo.getWidth(),rectangulo.getHeight()),Scalar(color.cblue,color.cgreen,color.cred),ancho);

    #endif
}

/**
 * Rellena un rectangulo
 * 
 * param imagen : imagen sobre la que se dibuja
 * 
 * param rectangulo : rectangulo que se dibuja
 * 
 * param color : color del rectangulo
 */
void GDibujo::fillRect( GImage imagen, GRect rectangulo, GColor color )
{
    #ifdef GDIBUJO_USAR_OPENCV

    cv::rectangle(imagen.imagenOpencv,Rect(rectangulo.x1,rectangulo.y1,rectangulo.getWidth(),rectangulo.getHeight()),Scalar(color.cblue,color.cgreen,color.cred),-1);

    #endif
}


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
void GDibujo::drawLine( GImage imagen, GPoint orig, GPoint dest, GColor color, int ancho )
{
    #ifdef GDIBUJO_USAR_OPENCV

    cv::line(imagen.imagenOpencv,Point(orig.x,orig.y),Point(dest.x,dest.y),Scalar(color.cblue,color.cgreen,color.cred),ancho);

    #endif
}

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
void GDibujo::drawElipse( GImage imagen, GPoint orig, int anchoEjeX, int anchoEjeY, GColor color, int ancho )
{
    #ifdef GDIBUJO_USAR_OPENCV

    cv::ellipse(imagen.imagenOpencv,Point(orig.x,orig.y),Size(anchoEjeX,anchoEjeY),0,0,360,Scalar(color.cblue,color.cgreen,color.cred),ancho);

    #endif
}

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
void GDibujo::fillElipse( GImage imagen, GPoint orig, int anchoEjeX, int anchoEjeY, GColor color )
{
    #ifdef GDIBUJO_USAR_OPENCV

    cv::ellipse(imagen.imagenOpencv,Point(orig.x,orig.y),Size(anchoEjeX,anchoEjeY),0,0,360,Scalar(color.cblue,color.cgreen,color.cred),-1);

    #endif
}

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
void GDibujo::drawText( GImage imagen, int x, int y, string texto, int tipoLetra, GColor color, float tamFont, int ancho  )
{
    #ifdef GDIBUJO_USAR_OPENCV

    if ( tipoLetra == GDIBUJO_FONT_HELVETICA )
    {
        putText(imagen.imagenOpencv, texto,Point(x,y),FONT_HERSHEY_SIMPLEX, tamFont, Scalar(color.cblue, color.cgreen, color.cred), ancho);
    }
    else
    if ( tipoLetra == GDIBUJO_FONT_SANS_SERIF )
    {
        putText(imagen.imagenOpencv, texto,Point(x,y),FONT_HERSHEY_COMPLEX, tamFont, Scalar(color.cblue, color.cgreen, color.cred), ancho);
    }
    

    #endif
}

/**
 * Hace un reflejo de la imagen
 * 
 * param imagen : iamgen a la que se le hace el flip.
 * 
 * param modo : modo del flip, definido en las constantes GDIBUJO_FLIP_XXXXX
 */
GImage GDibujo::flip( GImage imagen, int modo)
{
    GImage rpta;

    #ifdef GDIBUJO_USAR_OPENCV

    if ( modo == GDIBUJO_FLIP_HOR )
    {
        cv::flip(imagen.imagenOpencv,rpta.imagenOpencv,0);
    }
    else
    if ( modo == GDIBUJO_FLIP_VER )
    {
        cv::flip(imagen.imagenOpencv,rpta.imagenOpencv,1);
    }
    else
    if ( modo == GDIBUJO_FLIP_HOR_VER )
    {
        cv::flip(imagen.imagenOpencv,rpta.imagenOpencv,-1);
    }
    
    #endif

    rpta.formato = imagen.formato;
    rpta.ancho = imagen.ancho;
    rpta.altura = imagen.altura;

    return rpta;
}

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
void GDibujo::drawImage( GImage imagen, int x, int y, GImage imagenChica )
{
    #ifdef GDIBUJO_USAR_OPENCV
    cv::Rect r(x,y, imagenChica.ancho, imagenChica.altura);
    imagenChica.imagenOpencv.copyTo(imagen.imagenOpencv(r));
    #endif
}  