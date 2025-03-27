
#include "lib/facerec/facerecyn.h"


/**
 * Constructor
 */
DeteccionFaceVO::DeteccionFaceVO()
{

}

/**
 * Destructor
 */
DeteccionFaceVO::~DeteccionFaceVO()
{

}

/**
 * Retorna el tipo de dato
 */
string DeteccionFaceVO::getType() 
{ 
    return string("DeteccionFaceVO"); 
}


/**
 * Retorna la posicion del ojo izquierdo
 */
GPoint DeteccionFaceVO::getPosOjoIzq()
{
    return ptoOjoIzq;
}

/**
 * Retorna la posicion del ojo izquierdo
 */
GPoint DeteccionFaceVO::getPosOjoDer()
{
    return ptoOjoDer;
}

/**
 * Retorna la posicion de la nariz
 */
GPoint DeteccionFaceVO::getPosNariz()
{
    return ptoNariz;
}

/**
 * Retorna la posicion izquierda de la boca
 */
GPoint DeteccionFaceVO::getPosBocaIzq()
{
    return ptoBocaIzq;
}

/**
 * Retorna la posicion derecha de la boca
 */
GPoint DeteccionFaceVO::getPosBocaDer()
{
    return ptoBocaDer;
}


/**
 * Constructor
 */
DetectorRostrosYN::DetectorRostrosYN()
{
    escalaHor = 0;
    detector = NULL;
}

/**
 * Destructor
 */
DetectorRostrosYN::~DetectorRostrosYN()
{

}


/**
 * Carga el modelo
 * @param path ruta del modelo
 */
void DetectorRostrosYN::cargaModelo( std::string path )
{
    dimModelo.width = anchoImgModelo;
    dimModelo.height = alturaImgModelo;

    detector = cv::FaceDetectorYN::create(path, "", dimModelo, confianza, 0.3f, 500, 0,0);

}

/**
 * Detecta elementos
 * @param imagen imagen que se procesa
 * @return lista de instancias de DateccionFaceVO
 */
std::shared_ptr<GVector> DetectorRostrosYN::detectar( GImage *imagen )
{
    shared_ptr<GVector>lstFinal;
    shared_ptr<DeteccionFaceVO> det;
    cv::Mat imgEscalada;
    cv:Mat faces;
    int i,n;
    float conf,x1,y1,w,h,x0,y0,x2,y2,px,py,escHor,escVer;

    if ( escalaHor == 0 )
    {
        escalaHor = anchoImgOrig / ((float)anchoImgModelo);
        escalaVer = alturaImgOrig / ((float)alturaImgModelo);
    }

  
    detector->detect(imagen->imagenOpencv, faces);

    lstFinal = make_shared<GVector>();

    n = faces.rows;
    for(i=0; i<n; i++)
    {
        conf = faces.at<float>(i, 14);
        if ( conf < confianza )
        {
            continue;
        }

        det = make_shared<DeteccionFaceVO>();
        det->confianza = conf;
        det->indiceClase = 0;

        // escala las coordenadas con respecto a la imagen original

        x1 = (faces.at<float>(i, 0));
        y1 = (faces.at<float>(i, 1));
        w = (faces.at<float>(i, 2));
        h = (faces.at<float>(i, 3));

        px = x1;
        py = y1;

        x1*= escalaHor;
        w*= escalaHor;
        y1*= escalaVer;
        h*= escalaVer;

        escHor = 112.0 / w;
        escVer = 112.0 / h;

        if ( x1 < 0 ) x1 = 0;
        if ( y1 < 0 ) y1 = 0;

        // se guardan las coordenadas relativas a la imagen original en region
        det->region.x1 = x1;
        det->region.y1 = y1;
        det->region.x2 = x1+w-1.0;
        det->region.y2 = y1+h-1.0;
        
        // Mueve las coordenadas de los puntos relativas al origen de la cara
        for (int j = 0; j < 5; ++j)
        {
            x0 = faces.at<float>(i, 2*j+4);
            y0 = faces.at<float>(i, 2*j+5);
            x0-= px;
            y0-= py;
            faces.at<float>(i, 2*j+4) = x0*escalaHor;
            faces.at<float>(i, 2*j+5) = y0*escalaVer;
        }

        // las coordenadas se mueven de manera relativa al rostro
        // porque las detecciones trabajan con rostros ya cortados
        faces.at<float>(i, 0) = 0;
        faces.at<float>(i, 1) = 0;
        faces.at<float>(i, 2) = w;
        faces.at<float>(i, 3) = h;

        // Calcula coordenadas de puntos
        det->ptoOjoDer.x = faces.at<float>(i, 4);
        det->ptoOjoDer.y = faces.at<float>(i, 5);
        det->ptoOjoIzq.x = faces.at<float>(i, 6);
        det->ptoOjoIzq.y = faces.at<float>(i, 7);
        det->ptoNariz.x = faces.at<float>(i, 8);
        det->ptoNariz.y = faces.at<float>(i, 9);
        det->ptoBocaDer.x = faces.at<float>(i, 10);
        det->ptoBocaDer.y = faces.at<float>(i, 11);
        det->ptoBocaIzq.x = faces.at<float>(i, 12);
        det->ptoBocaIzq.y = faces.at<float>(i, 13);

        // Escala los puntos a la resolucion 112x112 
        for (int j = 0; j < 5; ++j)
        {
            x0 = faces.at<float>(i, 2*j+4);
            x0*= escHor;
            y0 = faces.at<float>(i, 2*j+5);            
            y0*= escVer;
            faces.at<float>(i, 2*j+4) = x0;
            faces.at<float>(i, 2*j+5) = y0;
        }

        // guardamos la deteccion unica
        det->deteccionOrig = faces.row(i).clone();

        lstFinal->add(det);
    }

    return lstFinal;
}


/**
 * Constructor
 */
FaceRecProcessorSFace::FaceRecProcessorSFace()
{
    recognizer = nullptr;
}

/**
 * Destructor
 */
FaceRecProcessorSFace::~FaceRecProcessorSFace()
{

}

/**
 * Carga las redes por defecto
 */
void FaceRecProcessorSFace::initRedesPorDefecto()
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
void FaceRecProcessorSFace::cargaModeloFaceLandMark( std::string path, int numPuntos )
{

}


/**
 * Carga el modelo para calcular el ID de un rostro
 */
void FaceRecProcessorSFace::cargaModeloFaceRec( std::string path )
{
    recognizer = cv::FaceRecognizerSF::create(path, "");
}


/**
 * Calcula el descriptor de un rostro
 * 
 *  imagenRostro : imagen de un rostro
 *
 * Retorna el descriptor de un rostro o NULL en caso de ERROR
 */
FaceDescriptor FaceRecProcessorSFace::getDescriptorRostro( GImage imagenRostro, DeteccionVO param )
{
    FaceDescriptor descriptor;
    cv::Mat target_features;
    cv::Mat aligned_face;
    float *datos;
    DeteccionVO deteccion;
    
    deteccion = param;
    
    recognizer->alignCrop(imagenRostro.imagenOpencv, deteccion.deteccionOrig, aligned_face);    
    recognizer->feature(aligned_face, target_features);
    
    double valor,suma = 0;

    datos = descriptor.getFloats();

    for(int i=0; i<128; i++)
    {
        valor = target_features.at<float>(0,i);        
        datos[i] = valor;
        suma+= (valor*valor);
    }
    suma = sqrt(suma);
    for(int i=0; i<128; i++)
    {
        datos[i]/= suma;
    }
        
    return descriptor;
}


/**
 * Detecta elementos
 * @param imagen imagen que se procesa
 * @param numDet variable que se actualiza con la cantidad de detecciones
 * @return puntero a un arreglo de detecciones
 */
GLinkedList<DeteccionVO> DetectorRostrosYN::detectarLst(GImage *imagen )
{
    GLinkedList<DeteccionVO> lstFinal;
    DeteccionVO det;
    cv::Mat imgEscalada;
    cv:Mat faces;
    int i,n;
    float conf,x1,y1,w,h,x0,y0,x2,y2,px,py,escHor,escVer;

    if ( escalaHor == 0 )
    {
        escalaHor = anchoImgOrig / ((float)anchoImgModelo);
        escalaVer = alturaImgOrig / ((float)alturaImgModelo);
    }

    detector->detect(imagen->imagenOpencv, faces);
    n = faces.rows;
    for(i=0; i<n; i++)
    {
        conf = faces.at<float>(i, 14);
        if ( conf < confianza )
        {
            continue;
        }

        det.confianza = conf;
        det.indiceClase = 0;

        // escala las coordenadas con respecto a la imagen original

        x1 = (faces.at<float>(i, 0));
        y1 = (faces.at<float>(i, 1));
        w = (faces.at<float>(i, 2));
        h = (faces.at<float>(i, 3));

        px = x1;
        py = y1;

        x1*= escalaHor;
        w*= escalaHor;
        y1*= escalaVer;
        h*= escalaVer;

        escHor = 112.0 / w;
        escVer = 112.0 / h;

        if ( x1 < 0 ) x1 = 0;
        if ( y1 < 0 ) y1 = 0;

        // se guardan las coordenadas relativas a la imagen original en region
        det.region.x1 = x1;
        det.region.y1 = y1;
        det.region.x2 = x1+w-1.0;
        det.region.y2 = y1+h-1.0;
        
        // Mueve las coordenadas de los puntos relativas al origen de la cara
        for (int j = 0; j < 5; ++j)
        {
            x0 = faces.at<float>(i, 2*j+4);
            y0 = faces.at<float>(i, 2*j+5);
            x0-= px;
            y0-= py;
            faces.at<float>(i, 2*j+4) = x0*escalaHor;
            faces.at<float>(i, 2*j+5) = y0*escalaVer;
        }

        // las coordenadas se mueven de manera relativa al rostro
        // porque las detecciones trabajan con rostros ya cortados
        faces.at<float>(i, 0) = 0;
        faces.at<float>(i, 1) = 0;
        faces.at<float>(i, 2) = w;
        faces.at<float>(i, 3) = h;

        // Calcula coordenadas de puntos
        det.ptoOjoDer.x = faces.at<float>(i, 4);
        det.ptoOjoDer.y = faces.at<float>(i, 5);
        det.ptoOjoIzq.x = faces.at<float>(i, 6);
        det.ptoOjoIzq.y = faces.at<float>(i, 7);
        det.ptoNariz.x = faces.at<float>(i, 8);
        det.ptoNariz.y = faces.at<float>(i, 9);
        det.ptoBocaDer.x = faces.at<float>(i, 10);
        det.ptoBocaDer.y = faces.at<float>(i, 11);
        det.ptoBocaIzq.x = faces.at<float>(i, 12);
        det.ptoBocaIzq.y = faces.at<float>(i, 13);

        // Escala los puntos a la resolucion 112x112 
        for (int j = 0; j < 5; ++j)
        {
            x0 = faces.at<float>(i, 2*j+4);
            x0*= escHor;
            y0 = faces.at<float>(i, 2*j+5);            
            y0*= escVer;
            faces.at<float>(i, 2*j+4) = x0;
            faces.at<float>(i, 2*j+5) = y0;
        }

        // guardamos la deteccion unica
        det.deteccionOrig = faces.row(i).clone();

        lstFinal.add(det);
    }

    return lstFinal;
}