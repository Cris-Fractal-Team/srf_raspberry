#include "lib/deeplearning/DetectorBO.h"

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


/**
 * Constructor
 */
DetectorObjetos::DetectorObjetos()
{
    anchoMaxDeteccion = 0;
    anchoMinDeteccion = 0;
    alturaMinDeteccion = 0;
    alturaMaxDeteccion = 0;
}

/**
 * Destructor
 */
DetectorObjetos::~DetectorObjetos()
{
    
}


/**
 * Carga las etiquetas
 * @param path ruta de las etiquetas
 */
bool DetectorObjetos::cargaEtiquetas( std::string path )
{
    // Open the File
	std::ifstream in(path.c_str());

	// Check if object is valid
	if(!in.is_open()) return false;

	std::string str;
	etiquetas.clear();

	// Read the next line from File untill it reaches the end.
	while (std::getline(in, str))
	{
		// Line contains string of length > 0 then save it in vector
		if(str.size()>0) etiquetas.add(str);
	}
	// Close The File
	in.close();
	return true;
}


/**
 * Estable las dimensiones de la imagen original
 * @param ancho ancho en pixeles de la imagen original
 * @param altura altura en pixeles de la imagen original
 */
void DetectorObjetos::setDimImgOrig( int ancho, int altura )
{
    anchoImgOrig = (double)ancho;
    alturaImgOrig = (double)altura;
}

/**
 * Establece las dimensiones de la imagen que procesa el modelo
 * @param ancho ancho en pixeles de la imagen original
 * @param altura altura en pixeles de la imagen original
 * @param numBytesPorPixel cantidad de bytes que usa cada pixel de la imagen original
 */
void DetectorObjetos::setDimImgModelo( int ancho, int altura, int numBytesPorPixel )
{
    anchoImgModelo = (double)ancho;
    alturaImgModelo = (double)altura;
    numBytesBuffer = ((unsigned long)altura) * ((unsigned long)ancho) * ((unsigned long)numBytesPorPixel);
}


/**
 * Establece el numero de threads con el que trabaja
 */
void DetectorObjetos::setNumThreads( int numThreads )
{
    numThreads = numThreads;
}


/**
 * Retorna un vector de anclas
 */
void DetectorObjetos::calculaAnclas( ConfigGeneradorAnclas opciones )
{
    lstAnclas = make_shared<GVector>();

    if ( opciones.strides.size() != opciones.num_layers  )
    {
        return;
    }

    shared_ptr<AnclaDeteccion> anclaNueva;
    int last_same_stride_layer,layer_id = 0;
    GVector anchor_height,anchor_width,aspect_ratios, scales;
    double scale;

    while( layer_id < opciones.strides.size() )
    {
        anchor_height.clear();
        anchor_width.clear();
        aspect_ratios.clear();
        scales.clear();

        last_same_stride_layer = layer_id;

        while (( last_same_stride_layer < opciones.strides.size() ) &&
               ( opciones.strides.getDouble(last_same_stride_layer) == opciones.strides.getDouble(layer_id) ))               
        {
            scale = opciones.min_scale + ( opciones.max_scale - opciones.min_scale) * 1.0 * ((float)last_same_stride_layer) / ((float)opciones.strides.size() - 1.0);
            if (( last_same_stride_layer == 0 ) && ( opciones.reduce_boxes_in_lowest_layers == true ))
            {
                aspect_ratios.add(1.0f);
                aspect_ratios.add(2.0f);
                aspect_ratios.add(0.5f);
                scales.add(0.1);
                scales.add(scale);
                scales.add(scale);
            }
            else
            {
                for(int i=0; i < opciones.aspectRatios.size(); i++)
                {
                    aspect_ratios.add(opciones.aspectRatios.getDouble(i));
                    scales.add(scale);
                }

                if ( opciones.interpolated_scale_aspect_ratio > 0.0 )
                {
                    float scale_next;
                    if ( last_same_stride_layer == (opciones.strides.size()-1) )
                    {
                        scale_next = 1.0;
                    }
                    else
                    {
                        scale_next = opciones.min_scale + ( opciones.max_scale-opciones.min_scale) * 1.0f * ((float)last_same_stride_layer+1.0) / ((float)opciones.strides.size()-1.0); 
                        scales.add(sqrt(scale*scale_next));
                        aspect_ratios.add(opciones.interpolated_scale_aspect_ratio);
                    }                    
                }                
            }
            last_same_stride_layer++;
        }
        for(int i=0; i < aspect_ratios.size(); i++)
        {
            float ratio_sqrts= sqrt(aspect_ratios.getDouble(i));
            anchor_height.add(scales.getDouble(i)/ratio_sqrts);
            anchor_width.add(scales.getDouble(i)*ratio_sqrts);
        }

        float feature_map_height = 0;
        float feature_map_width = 0;

        if ( opciones.featura_map_height.size() > 0 )
        {
            feature_map_height = opciones.featura_map_height.getDouble(layer_id);
            feature_map_width = opciones.feature_map_width.getDouble(layer_id);
        }
        else
        {
            float stride = opciones.strides.getDouble(layer_id);
            feature_map_height = ceil(1.0*opciones.input_size_height / stride );
            feature_map_width = ceil(1.0*opciones.input_size_width / stride );
        }

        for(float y=0; y<feature_map_height; y++)
        {
            for(float x=0; x<feature_map_width; x++)
            {
                for(int anchor_id=0; anchor_id < anchor_height.size(); anchor_id++)
                {
                    float xcenter = (x+opciones.anchor_offset_x)*1.0/feature_map_width;
                    float ycenter = (y+opciones.anchor_offset_y)*1.0/feature_map_height;
                    float w=0;
                    float h=0;
                    if ( opciones.fixed_anchos_size )
                    {
                        w = 1.0;
                        h = 1.0;
                    }
                    else
                    {
                        w = anchor_width.getDouble(anchor_id);
                        h = anchor_height.getDouble(anchor_id);
                    }
                    anclaNueva = make_shared<AnclaDeteccion>();
                    anclaNueva->xcenter = xcenter;
                    anclaNueva->ycenter = ycenter;
                    anclaNueva->ancho = w;
                    anclaNueva->altura = h;

                    lstAnclas->add(anclaNueva);
                }
            }
        }

        layer_id = last_same_stride_layer;
    }
}


/**
 * Constructor de la clase que hace detecciones con TensorFlowLite
 */
DetectorTensorFlowLite::DetectorTensorFlowLite()
{
    numThreads = 2;
    confianza = 0.5;
}

DetectorTensorFlowLite::~DetectorTensorFlowLite()
{
    //dtor
}


/**
 * Carga el modelo
 * @param path ruta del modelo
 */
void DetectorTensorFlowLite::cargaModelo( std::string path )
{
    model = tflite::FlatBufferModel::BuildFromFile(path.c_str());

    // Build the interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);

    interpreter->AllocateTensors();

    interpreter->SetAllowFp16PrecisionForFp32(true);
    interpreter->SetNumThreads(numThreads);      //quad core
}

/**
 * Detecta elementos
 * @param imagen imagen que se procesa
 * @return lista de rectangulos detectados
 */
// std::shared_ptr<GVector> DetectorTensorFlowLite::detectar( Mat *imagen )
std::shared_ptr<GVector> DetectorTensorFlowLite::detectar( GImage *imagen )
{
    std::shared_ptr<GVector> detecciones;
    std::shared_ptr<DeteccionVO> deteccion;
    void *ptr;

    detecciones = std::make_shared<GVector>();

    ptr = interpreter->typed_input_tensor<uchar>(0);

    // memcpy(ptr, imagen->data, numBytesBuffer);
    
    memcpy(ptr, imagen->getCharData(), numBytesBuffer);

    interpreter->Invoke();      // run your model

    const float* detection_locations = interpreter->tensor(interpreter->outputs()[0])->data.f;
    const float* detection_classes=interpreter->tensor(interpreter->outputs()[1])->data.f;
    const float* detection_scores = interpreter->tensor(interpreter->outputs()[2])->data.f;
    const int    num_detections = *interpreter->tensor(interpreter->outputs()[3])->data.f;

    for(int i = 0; i < num_detections; i++)
    {
        // cout << "Det " << i << " " << detection_scores[i] << " VS " << confianza << endl;
        if ( detection_scores[i] > confianza )
        {
            int  det_index = (int)detection_classes[i]+1;

            float y1=detection_locations[4*i  ]*alturaImgOrig;
            float x1=detection_locations[4*i+1]*anchoImgOrig;
            float y2=detection_locations[4*i+2]*alturaImgOrig;
            float x2=detection_locations[4*i+3]*anchoImgOrig;

            deteccion = make_shared<DeteccionVO>();
            deteccion->region.x1 = (int)x1;
            deteccion->region.y1 = (int)y1;
            deteccion->region.x2 = (int)x2;
            deteccion->region.y2 = (int)y2;
            // deteccion->region.width = (int)(x2-x1);
            // deteccion->region.height = (int)(y2-y1);
            deteccion->confianza = detection_scores[i];
            deteccion->indiceClase = det_index;

            if ( anchoMinDeteccion > 0 ) 
            {
                if ( anchoMaxDeteccion > 0 )
                {
                    if (( deteccion->region.getWidth() <= anchoMaxDeteccion) && ( deteccion->region.getHeight() <= alturaMaxDeteccion))
                    {
                        detecciones->add(deteccion);
                    }
                }
                else
                if (( deteccion->region.getWidth() >= anchoMinDeteccion) && ( deteccion->region.getHeight() >= alturaMinDeteccion))
                {
                    detecciones->add(deteccion);    
                }
            }
            else
            if ( anchoMaxDeteccion > 0 )
            {
                if (( deteccion->region.getWidth() <= anchoMaxDeteccion) && ( deteccion->region.getHeight() <= alturaMaxDeteccion))
                {
                    detecciones->add(deteccion);
                }
            }
            else
            {
                detecciones->add(deteccion);
            }
        
        }
    }

    // cout << "Total detecciones : " << detecciones->size() << endl;

    return detecciones;
}

/**
 * Detecta elementos
 * @param imagen imagen que se procesa
 * @param numDet variable que se actualiza con la cantidad de detecciones
 * @return puntero a un arreglo de detecciones
 */
GLinkedList<DeteccionVO>DetectorTensorFlowLite::detectarLst(GImage *imagen )
{       
    GLinkedList<DeteccionVO> detecciones;
    DeteccionVO deteccion;
    int pos;
    void *ptr;

    ptr = interpreter->typed_input_tensor<uchar>(0);

    // memcpy(ptr, imagen->data, numBytesBuffer);
    
    memcpy(ptr, imagen->getCharData(), numBytesBuffer);

    interpreter->Invoke();      // run your model

    const float* detection_locations = interpreter->tensor(interpreter->outputs()[0])->data.f;
    const float* detection_classes=interpreter->tensor(interpreter->outputs()[1])->data.f;
    const float* detection_scores = interpreter->tensor(interpreter->outputs()[2])->data.f;
    const int    num_detections = *interpreter->tensor(interpreter->outputs()[3])->data.f;

    for(int i = 0; i < num_detections; i++)
    {
        // cout << "Det " << i << " " << detection_scores[i] << " VS " << confianza << endl;
        if ( detection_scores[i] > confianza )
        {
            int  det_index = (int)detection_classes[i]+1;

            float y1=detection_locations[4*i  ]*alturaImgOrig;
            float x1=detection_locations[4*i+1]*anchoImgOrig;
            float y2=detection_locations[4*i+2]*alturaImgOrig;
            float x2=detection_locations[4*i+3]*anchoImgOrig;

            deteccion.region.x1 = (int)x1;
            deteccion.region.y1 = (int)y1;
            deteccion.region.x2 = (int)x2;
            deteccion.region.y2 = (int)y2;
            // deteccion->region.width = (int)(x2-x1);
            // deteccion->region.height = (int)(y2-y1);
            deteccion.confianza = detection_scores[i];
            deteccion.indiceClase = det_index;

            if ( anchoMinDeteccion > 0 ) 
            {
                if ( anchoMaxDeteccion > 0 )
                {
                    if (( deteccion.region.getWidth() <= anchoMaxDeteccion) && ( deteccion.region.getHeight() <= alturaMaxDeteccion))
                    {
                        detecciones.add(deteccion);
                    }
                }
                else
                if (( deteccion.region.getWidth() >= anchoMinDeteccion) && ( deteccion.region.getHeight() >= alturaMinDeteccion))
                {
                    detecciones.add(deteccion);  
                }
            }
            else
            if ( anchoMaxDeteccion > 0 )
            {
                if (( deteccion.region.getWidth() <= anchoMaxDeteccion) && ( deteccion.region.getHeight() <= alturaMaxDeteccion))
                {
                    detecciones.add(deteccion);
                }
            }
            else
            {
                detecciones.add(deteccion);
            }        
        }
    }

    // cout << "Total detecciones : " << detecciones->size() << endl;
    
    return detecciones;
}


/**
 * Establece el numero de threads con el que trabaja
 */
void DetectorTensorFlowLite::setNumThreads( int numThreads )
{
    numThreads = numThreads;
    interpreter->SetNumThreads(1);
}


/**
 * Constructor
 */
DetectorHaar::DetectorHaar()
{
    alturaMinDeteccion = 23;
    anchoMinDeteccion = 23;
    alturaMaxDeteccion = 200;
    anchoMaxDeteccion = 200;

    factorEscala = 1.3;
    numVecinos = 3;

    escalaHorImgOrig = 0;
    escalaVerImgOrig = 0;
}


/**
 * Destructor
 */
DetectorHaar::~DetectorHaar()
{

}

/**
 * Carga el modelo
 * @param path ruta del modelo
 */
void DetectorHaar::cargaModelo( std::string path )
{
    if ( detector.load(path)) 
    {
        std::cerr << "Error cargando el archivo haarcascade_frontalface_default.xml" << std::endl;
    }
}

/**
 * Detecta elementos
 * @param imagen imagen que se procesa
 * @return lista de rectangulos detectados
 */
std::shared_ptr<GVector> DetectorHaar::detectar( GImage *imagen )
{
    cv::Mat gray;
    std::vector<cv::Rect> detecciones;
    cv::Rect rec;
    shared_ptr<GVector> rpta = make_shared<GVector>();
    shared_ptr<DeteccionVO> det;
    double x1,y1;
  
    if ( escalaHorImgOrig == 0 )
    {
        escalaHorImgOrig = ((double)anchoImgOrig)/((double)anchoImgModelo);
        escalaVerImgOrig = ((double)alturaImgOrig)/((double)alturaImgModelo);
    }

    cv::cvtColor(imagen->imagenOpencv, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    detector.detectMultiScale(gray, detecciones, factorEscala, numVecinos, 0, 
        cv::Size(anchoMinDeteccion, alturaMinDeteccion), cv::Size(anchoMaxDeteccion, alturaMaxDeteccion) );

    
    for (size_t i = 0; i < detecciones.size(); i++) 
    {
        rec = detecciones[i];
        det = make_shared<DeteccionVO>();

        x1 = rec.x;
        x1*= escalaHorImgOrig;
        y1 = rec.y;
        y1*= escalaVerImgOrig;

        det->region.x1 = x1;
        det->region.y1 = y1;
        det->region.x2 = x1+((double)rec.width)*escalaHorImgOrig-1;
        det->region.y2 = y1+((double)rec.height)*escalaVerImgOrig-1;
        det->confianza = 1;
        det->indiceClase = 0;

        rpta->add(det);
    }

    return rpta;
}

/**
 * Detecta elementos
 * @param imagen imagen que se procesa
 * @return lista de rectangulos detectados
 */
GLinkedList<DeteccionVO> DetectorHaar::detectarLst( GImage *imagen )
{
    cv::Mat gray;
    std::vector<cv::Rect> detecciones;
    cv::Rect rec;
    GLinkedList<DeteccionVO> rpta;
    DeteccionVO det;
    double x1,y1;
    int pos = 0;

  
    if ( escalaHorImgOrig == 0 )
    {
        escalaHorImgOrig = ((double)anchoImgOrig)/((double)anchoImgModelo);
        escalaVerImgOrig = ((double)alturaImgOrig)/((double)alturaImgModelo);
    }

    cv::cvtColor(imagen->imagenOpencv, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    detector.detectMultiScale(gray, detecciones, factorEscala, numVecinos, 0, 
        cv::Size(anchoMinDeteccion, alturaMinDeteccion), cv::Size(anchoMaxDeteccion, alturaMaxDeteccion) );

    for (size_t i = 0; i < detecciones.size(); i++) 
    {
        rec = detecciones[i];

        x1 = rec.x;
        x1*= escalaHorImgOrig;
        y1 = rec.y;
        y1*= escalaVerImgOrig;

        det.region.x1 = x1;
        det.region.y1 = y1;
        det.region.x2 = x1+((double)rec.width)*escalaHorImgOrig-1;
        det.region.y2 = y1+((double)rec.height)*escalaVerImgOrig-1;
        det.confianza = 1;
        det.indiceClase = 0;

        rpta.add(det);
    }

    return rpta;
}


/**
 * Constructor
 */
BlazeFaceDetector::BlazeFaceDetector() : DetectorTensorFlowLite()
{
    puntajeMinimo = 0;
    confianza = 0.7;
}

/**
 * Constructor
 */
BlazeFaceDetector::~BlazeFaceDetector()
{

}

/**
 * Detecta elementos
 * @param imagen imagen que se procesa
 * @return lista de rectangulos detectados
 */
std::shared_ptr<GVector> BlazeFaceDetector::detectar( GImage *imagen )
{
    float x1,x2,y1,y2;
    std::shared_ptr<GVector> detecciones;
    std::shared_ptr<DeteccionVO> deteccion;
    shared_ptr<AnclaDeteccion> ancla;
    GColor rojo(255,0,0);

    detecciones = std::make_shared<GVector>();

    float *inputTensor = (float *)interpreter->typed_input_tensor<float>(0);

    if ( puntajeMinimo == 0 )
    {
        puntajeMinimo = log(confianza/(1.0-confianza));
        numBytesBuffer = imagen->imagenOpencv.total()*imagen->imagenOpencv.elemSize();
    }
    
    // cv::Mat img = imagen->imagenOpencv.clone();
    cv::Mat img = imagen->imagenOpencv.clone();
    img.convertTo(img, CV_32FC3,1.0/255.0);
    img = (img-0.5)/0.5;
    img.convertTo(img,CV_32F);
    memcpy(inputTensor, img.data, numBytesBuffer);
    
    interpreter->Invoke();      // run your model

    // for(int i=0; i < interpreter->outputs().size(); i++)
    // {
    //     TfLiteTensor *tensorOuput = interpreter->tensor(interpreter->outputs()[i]);
    //     cout << "Output tensor " << i <<  " " << tensorOuput->name << endl;

    //     TfLiteIntArray *tensorOutputDims = tensorOuput->dims;
    //     for(int j=0; j<tensorOutputDims->size; j++)
    //     {
    //         cout << "    Dim[" << j << "] " << tensorOutputDims->data[j] << endl; 
    //     }
    // }

    TfLiteTensor *tensorPosiciones = interpreter->tensor(interpreter->outputs()[0]);
    TfLiteTensor *tensorPuntajes = interpreter->tensor(interpreter->outputs()[1]);

    const float* detection_locations = tensorPosiciones->data.f;
    const float* detection_puntajes = tensorPuntajes->data.f;
            
    float puntajeReal;
    int pos;

    // cout << "Puntaje Minimo " << puntajeMinimo << endl; 

    for(int i = 0; i < 896; i++)
    {
        // cout << "Puntaje " << i << " : " << detection_puntajes[i] << endl; 
        if ( detection_puntajes[i] < puntajeMinimo )
        {
            continue;
        }

        // cout << "Puntaje Valido " << i << " : " << detection_puntajes[i] << endl; 
        puntajeReal = 1.0 / (1.0 + exp(-detection_puntajes[i]));
        
        if ( puntajeReal > confianza )
        {
     
            ancla = dynamic_pointer_cast<AnclaDeteccion>(lstAnclas->get(i));

            float sx = detection_locations[i*16 + 0];
            float sy = detection_locations[i*16 + 1];
            float w = detection_locations[i*16 + 2];
            float h = detection_locations[i*16 + 3];

            float cx = sx + ancla->xcenter * (float)anchoImgModelo;
            float cy = sy + ancla->ycenter * (float)alturaImgModelo;

            cx/= (float)anchoImgModelo;
            cy/= (float)alturaImgModelo;
            w/= (float)anchoImgModelo;
            h/= (float)alturaImgModelo;

            x1 = (cx-w*0.5);
            y1 = (cy-h*0.5);
            x2 = (cx+w*0.5);
            y2 = (cy-h*0.5);

            deteccion = make_shared<DeteccionVO>();
            deteccion->indiceClase = 0;
            deteccion->region.x1 = (int)(x1);
            deteccion->region.y1 = (int)(y1);
            deteccion->region.x2 = (int)(x2);
            deteccion->region.y2 = (int)(y2);
            deteccion->confianza = puntajeReal;

            GDibujo::drawRect(*imagen, deteccion->region, rojo, 2);
            GDibujo::show(*imagen, "dete");

            if ( anchoMinDeteccion > 0 ) 
            {
                if ( anchoMaxDeteccion > 0 )
                {
                    if (( deteccion->region.getWidth() <= anchoMaxDeteccion) && ( deteccion->region.getHeight() <= alturaMaxDeteccion))
                    {
                        detecciones->add(deteccion);
                    }
                }
                else
                if (( deteccion->region.getWidth() >= anchoMinDeteccion) && ( deteccion->region.getHeight() >= alturaMinDeteccion))
                {
                    detecciones->add(deteccion);    
                }
            }
            else
            if ( anchoMaxDeteccion > 0 )
            {
                if (( deteccion->region.getWidth() <= anchoMaxDeteccion) && ( deteccion->region.getHeight() <= alturaMaxDeteccion))
                {
                    detecciones->add(deteccion);
                }
            }
            else
            {
                detecciones->add(deteccion);
            }
        
        }
    }

    // cout << "Total detecciones : " << detecciones->size() << endl;

    return detecciones;
}

GLinkedList<DeteccionVO> BlazeFaceDetector::detectarLst( GImage *imagen )
{
    float x1,x2,y1,y2;
    GLinkedList<DeteccionVO> detecciones;
    DeteccionVO deteccion;
    shared_ptr<AnclaDeteccion> ancla;
    GColor rojo(255,0,0);

    float *inputTensor = (float *)interpreter->typed_input_tensor<float>(0);

    if ( puntajeMinimo == 0 )
    {
        puntajeMinimo = log(confianza/(1.0-confianza));
        numBytesBuffer = imagen->imagenOpencv.total()*imagen->imagenOpencv.elemSize();
    }
    
    // cv::Mat img = imagen->imagenOpencv.clone();
    cv::Mat img = imagen->imagenOpencv.clone();
    img.convertTo(img, CV_32FC3,1.0/255.0);
    img = (img-0.5)/0.5;
    img.convertTo(img,CV_32F);
    memcpy(inputTensor, img.data, numBytesBuffer);
    
    interpreter->Invoke();      // run your model

    // for(int i=0; i < interpreter->outputs().size(); i++)
    // {
    //     TfLiteTensor *tensorOuput = interpreter->tensor(interpreter->outputs()[i]);
    //     cout << "Output tensor " << i <<  " " << tensorOuput->name << endl;

    //     TfLiteIntArray *tensorOutputDims = tensorOuput->dims;
    //     for(int j=0; j<tensorOutputDims->size; j++)
    //     {
    //         cout << "    Dim[" << j << "] " << tensorOutputDims->data[j] << endl; 
    //     }
    // }

    TfLiteTensor *tensorPosiciones = interpreter->tensor(interpreter->outputs()[0]);
    TfLiteTensor *tensorPuntajes = interpreter->tensor(interpreter->outputs()[1]);

    const float* detection_locations = tensorPosiciones->data.f;
    const float* detection_puntajes = tensorPuntajes->data.f;
            
    float puntajeReal;
    int nDet,posDet;

    // cout << "Puntaje Minimo " << puntajeMinimo << endl; 

    nDet = 0;
    posDet = 0;
    
    for(int i = 0; i < 896; i++)
    {
        // cout << "Puntaje " << i << " : " << detection_puntajes[i] << endl; 
        if ( detection_puntajes[i] < puntajeMinimo )
        {
            continue;
        }

        // cout << "Puntaje Valido " << i << " : " << detection_puntajes[i] << endl; 
        puntajeReal = 1.0 / (1.0 + exp(-detection_puntajes[i]));
        
        if ( puntajeReal > confianza )
        {
    
            ancla = dynamic_pointer_cast<AnclaDeteccion>(lstAnclas->get(i));

            float sx = detection_locations[i*16 + 0];
            float sy = detection_locations[i*16 + 1];
            float w = detection_locations[i*16 + 2];
            float h = detection_locations[i*16 + 3];

            float cx = sx + ancla->xcenter * (float)anchoImgModelo;
            float cy = sy + ancla->ycenter * (float)alturaImgModelo;

            cx/= (float)anchoImgModelo;
            cy/= (float)alturaImgModelo;
            w/= (float)anchoImgModelo;
            h/= (float)alturaImgModelo;

            x1 = (cx-w*0.5);
            y1 = (cy-h*0.5);
            x2 = (cx+w*0.5);
            y2 = (cy-h*0.5);

            deteccion.indiceClase = 0;
            deteccion.region.x1 = (int)(x1);
            deteccion.region.y1 = (int)(y1);
            deteccion.region.x2 = (int)(x2);
            deteccion.region.y2 = (int)(y2);
            deteccion.confianza = puntajeReal;

            // GDibujo::drawRect(*imagen, deteccion->region, rojo, 2);
            // GDibujo::show(*imagen, "dete");

            if ( anchoMinDeteccion > 0 ) 
            {
                if ( anchoMaxDeteccion > 0 )
                {
                    if (( deteccion.region.getWidth() <= anchoMaxDeteccion) && ( deteccion.region.getHeight() <= alturaMaxDeteccion))
                    {
                        detecciones.add(deteccion);
                    }
                }
                else
                if (( deteccion.region.getWidth() >= anchoMinDeteccion) && ( deteccion.region.getHeight() >= alturaMinDeteccion))
                {
                    detecciones.add(deteccion);
                }
            }
            else
            if ( anchoMaxDeteccion > 0 )
            {
                if (( deteccion.region.getWidth() <= anchoMaxDeteccion) && ( deteccion.region.getHeight() <= alturaMaxDeteccion))
                {
                    detecciones.add(deteccion);
                }
            }
            else
            {
                detecciones.add(deteccion);
            }
        
        }
    }
          

    return detecciones;
}