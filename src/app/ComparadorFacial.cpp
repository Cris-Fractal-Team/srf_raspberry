
#include "app/ComparadorFacial.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/GStringUtils.h"
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/hailolib/FaceRecHailo.h"


#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

/**
 * Compara dos archivos que tienen una foto en la que hay una persona
 * se debe detectar el rostro en cada imagen y luego hacer la comparacion
 * 
 *  pathCara1: ruta de la primera imagen
 * 
 *  pathCara2: ruta de la segunda imagen
 * 
 *  params: parametros leidos desde el archivo de configuracion de la aplicacion
 *
 * Retorna la similaridad entre las caras
 */
float ComparadorFacial::comparaBusca( string pathCara1, string pathCara2, shared_ptr<GHashMap> params )
{
	hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr;
    FaceRecHailo generadorDesc;
    DetectorCarasHailoSCRFD detector;

    // inicializa el dispositivo
    hailort::Expected<std::unique_ptr<hailort::VDevice>> device = hailort::VDevice::create();    
    if (!device) 
    {
        cerr << "Error: No se pudo inicializar el dispositivo Hailo." << endl;        
        return 0.0;
    }
    devicePtr = &device;

    // extrae parametros
    string pathModeloDescFacial = params->getString("pathModeloDescFacial");
    string nombreCapaSalidaRedFacial = params->getString("nombreUltimaCapaRedNeuronal");
    double paramContraste = params->getStringDouble("paramContraste", 0);

    // crea el generador de descriptores faciales
    generadorDesc.runner.device = &device;
    generadorDesc.paramContraste = paramContraste;
    generadorDesc.previsualizaImgReconocimiento = params->getStringBool("previsualizaImgReconocimiento", false);
    generadorDesc.esperarPrevImgReconocimiento = params->getStringBool("esperarPrevImgReconocimiento", false);
    generadorDesc.guardarPrevImgReconocimiento = params->getStringBool("guardarPrevImgReconocimiento", false);

    // Cargamos el modelo de red neuronal
    generadorDesc.nombreUltimaCapaModelo = nombreCapaSalidaRedFacial;
    if ( generadorDesc.cargarModelo(pathModeloDescFacial) != 0 )
    {
        cout << "Error al cargar modelo generador de descriptores" << endl;
        return 0.0;
    }
    cout << "Generador de descriptores iniciado" << endl;

    // crea el detector de rostros
    detector.setDimImagenes(640,640);
    detector.runner.device = &device;
    if ( detector.cargarModelo("./models/scrfd_2.5ga.hef") != 0 )
    {
        cout << "Error al cargar el modelo detector" << endl;        
        return 0.0;
    }           
    cout << "Detector de rostros iniciado" << endl;

    SIMD_TYPE descriptor1[NUM_ELEMS_DESC_FACIAL];
    SIMD_TYPE descriptor2[NUM_ELEMS_DESC_FACIAL];
    DeteccionCaraHailo det;

    // detectamos la primera cara y calculamos su descriptor
    GImage rostro1 = GDibujo::read(pathCara1);
    std::vector<DeteccionCaraHailo> detecciones1 = detector.detectar_2_5g(rostro1, 0.40);
    DescPersonaExterno desc1;

    if ( detecciones1.size() == 0 )
    {
        cout << "No se detecto rostro en la primera imagen" << endl;        
        return 0.0;
    }
    det = detecciones1.at(0);
    generadorDesc.calculaDescriptor(rostro1, det, desc1.vecDescripcion);     
            
    // detectamos la segunda cara y calculamos su descriptor
    GImage rostro2 = GDibujo::read(pathCara2);
    std::vector<DeteccionCaraHailo> detecciones2 = detector.detectar_2_5g(rostro2, 0.40);
    DescPersonaExterno desc2;

    if ( detecciones2.size() == 0 )
    {
        cout << "No se detecto rostro en la primera imagen" << endl;        
        return 0.0;
    }
    det = detecciones2.at(0);
    generadorDesc.calculaDescriptor(rostro2, det, desc2.vecDescripcion);     

    // Creamos la lista con el universo de personas conocidas
    GLinkedList<DescPersonaExterno> lstUniv;
    lstUniv.add(desc1);

    // Buscamos la comparacion
    IdentificadorPerHailo identificador;
    // identificador.setNumThreadsIdentificacion(1);

    int indiceIdentif;
    float tolerancia = params->getStringDouble("deltaRostroMax", 0.5);
    float distancia;

    // normalizamos el descriptor 2
    float norm1;
    int size = NUM_ELEMS_DESC_FACIAL;

    norm1 = 0.0;
    for(size_t i=0;i<size;i++)
    {   
        norm1+= desc2.vecDescripcion[i] * desc2.vecDescripcion[i];
    }
    norm1 = sqrt(norm1);
    for(size_t i=0;i<size;i++)
    {   
        desc2.vecDescripcion[i] /= norm1;
    }   

    DescPersonaExterno *descEncontrado = identificador.encuentraPerCerCos(&lstUniv, desc2.vecDescripcion, tolerancia, &indiceIdentif, &distancia );

    cout << "Distancia calculada : " << distancia << endl;

    if ( indiceIdentif < 0 )
        return 0.0;

    return distancia;
}


/**
 * Compara dos archivos que estan alineados frontalmente y tiene la resolucion
 * esperada por la red neuronal.
 *  
 *  pathCara1: ruta de la primera imagen
 * 
 *  pathCara2: ruta de la segunda imagen
 * 
 *  params: parametros leidos desde el archivo de configuracion de la aplicacion
 *
 * Retorna la similaridad entre las caras
 */
float ComparadorFacial::comparaSimple( string pathCara1, string pathCara2, shared_ptr<GHashMap> params )
{
	hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr;
    FaceRecHailo generadorDesc;
    
    // inicializa el dispositivo
    hailort::Expected<std::unique_ptr<hailort::VDevice>> device = hailort::VDevice::create();    
    if (!device) 
    {
        cerr << "Error: No se pudo inicializar el dispositivo Hailo." << endl;        
        return 0.0;
    }
    devicePtr = &device;

    // extrae parametros
    string pathModeloDescFacial = params->getString("pathModeloDescFacial");
    string nombreCapaSalidaRedFacial = params->getString("nombreUltimaCapaRedNeuronal");
    double paramContraste = params->getStringDouble("paramContraste", 0);

    // crea el generador de descriptores faciales
    generadorDesc.runner.device = &device;
    generadorDesc.paramContraste = paramContraste;
    generadorDesc.previsualizaImgReconocimiento = params->getStringBool("previsualizaImgReconocimiento", false);
    generadorDesc.esperarPrevImgReconocimiento = params->getStringBool("esperarPrevImgReconocimiento", false);
    generadorDesc.guardarPrevImgReconocimiento = params->getStringBool("guardarPrevImgReconocimiento", false);

    // Cargamos el modelo de red neuronal
    generadorDesc.nombreUltimaCapaModelo = nombreCapaSalidaRedFacial;
    if ( generadorDesc.cargarModelo(pathModeloDescFacial) != 0 )
    {
        cout << "Error al cargar modelo generador de descriptores" << endl;
        return 0.0;
    }
    cout << "Generador de descriptores iniciado" << endl;

    SIMD_TYPE descriptor1[NUM_ELEMS_DESC_FACIAL];
    SIMD_TYPE descriptor2[NUM_ELEMS_DESC_FACIAL];
    DeteccionCaraHailo det;

    // Ponemos en duro la ubicacion de la deteccion a las dimensiones 
    // esperadas por el modelo, que deben coincidir con las dimensiones de la imagen
    det.ptoSupIzq.x = 0;
    det.ptoSupIzq.y = 0;
    det.ptoInfDer.x = 111;
    det.ptoInfDer.y = 111;

    // Ponemos en duro las coordenadas de los ojos, nariz y boca
    // en base a las coordenadas esandares de arcface
    det.ojoIzq.x = 38.2946;
    det.ojoIzq.y = 51.6963;
    det.ojoDer.x = 73.5318;
    det.ojoDer.y = 51.5014;
    det.nariz.x = 56.0252;
    det.nariz.y = 71.7366;
    det.bocaIzq.x = 41.5493;
    det.bocaIzq.y = 92.3655;
    det.bocaDer.x = 70.7299;
    det.bocaDer.y = 92.2041;

    // calculamos el descriptor de las dos caras
    GImage rostro1 = GDibujo::read(pathCara1);
    DescPersonaExterno desc1;
    generadorDesc.calculaDescriptor(rostro1, det, desc1.vecDescripcion);     
            
    GImage rostro2 = GDibujo::read(pathCara2);
    DescPersonaExterno desc2;
    generadorDesc.calculaDescriptor(rostro2, det, desc2.vecDescripcion);     

    // Creamos la lista con el universo de personas conocidas
    GLinkedList<DescPersonaExterno> lstUniv;
    lstUniv.add(desc1);

    // Buscamos la comparacion
    IdentificadorPerHailo identificador;
    // identificador.setNumThreadsIdentificacion(1);

    // normalizamos el descriptor 2
    float norm1;
    int size = NUM_ELEMS_DESC_FACIAL;

    norm1 = 0.0;
    for(size_t i=0;i<size;i++)
    {   
        norm1+= desc2.vecDescripcion[i] * desc2.vecDescripcion[i];
    }
    norm1 = sqrt(norm1);
    for(size_t i=0;i<size;i++)
    {   
        desc2.vecDescripcion[i] /= norm1;
    }         

    int indiceIdentif;
    float tolerancia = params->getStringDouble("deltaRostroMax", 0.5);
    float distancia;

    DescPersonaExterno *descEncontrado = identificador.encuentraPerCerCos(&lstUniv, desc2.vecDescripcion, tolerancia, &indiceIdentif, &distancia );

    cout << "Distancia calculada : " << distancia << endl;

    if ( indiceIdentif < 0 )
        return 0.0;

    return distancia;
}
