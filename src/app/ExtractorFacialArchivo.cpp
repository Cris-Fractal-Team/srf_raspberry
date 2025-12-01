
#include "app/ExtractorFacialArchivo.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/GStringUtils.h"
#include "lib/hailolib/DetectionTrackerHailo.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>


/**
 * Constructor
 */
ExtractorFacialArchivo::ExtractorFacialArchivo()
{
    pixelSuavizado = 5;
    resizeSuavizado = 1;
    jpegSuavizado = 100;
    guardarCarasFrontalesAlineadas = false;
}

/**
 * Ejecuta el proceso
 */
void ExtractorFacialArchivo::ejecutar()
{
    int i,n;

    vector<DeteccionCaraHailo> lstDet;
    string fila,idPersona,pathFoto,nombre;
    string datosArchivo = leeArchivoTexto(pathArchivoDatos);
    GVector lstFilas = GStringUtils::split(datosArchivo, "\n");
    hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr;
    DetectorCarasHailoSCRFD detector;        
    FaceRecHailo generadorDesc;
    std::ofstream archivoBd(pathArchivoBD, std::ios::out);
    
    if ( !archivoBd.is_open() )
    {
        cerr << "Error: no se pudo abrir/crear el archivo de BD" << endl;
        return;
    }

    // inicializa el dispositivo
    // hailort::Expected<std::unique_ptr<hailort::VDevice>> device = hailort::VDevice::create();    
    if (vdevice == nullptr || !(*vdevice)) {
        cerr << "Error: VDevice no inicializado en ExtractorFacialArchivo." << endl;
        return;
    }

    // configura el detector
    detector.setDimImagenes(640,640);
    detector.runner.device = vdevice;
    if ( detector.cargarModelo("./models/scrfd_2.5ga.hef") != 0 )
    {
        cout << "Error al cargar el modelo detector" << endl;        
        return;
    }           
    cout << "Detector de rostros iniciado" << endl;

    // crea el generador de descriptores faciales
    generadorDesc.runner.device = vdevice;
    generadorDesc.paramContraste = paramContraste;
    // if ( generadorDesc.cargarModelo("./models/arcface_mobilefacenet.hef") != 0 )
    generadorDesc.nombreUltimaCapaModelo = nombreCapaSalidaRedFacial;
    generadorDesc.guardarCarasFrontalesAlineadas = guardarCarasFrontalesAlineadas;
    generadorDesc.previsualizaImgReconocimiento = previsualizaImgReconocimiento;
    generadorDesc.esperarPrevImgReconocimiento = esperarPrevImgReconocimiento;


    if ( generadorDesc.cargarModelo(pathModeloDescFacial) != 0 )
    {
        cout << "Error al cargar modelo generador de descriptores" << endl;
        return;
    }
    cout << "Generador de descriptores iniciado" << endl;
     
    n = lstFilas.size();
    for(i=0;i<n;i++)
    {
        fila = lstFilas.getString(i);
        fila = GStringUtils::trim(fila);
        GVector lstValores = GStringUtils::split(fila,",");
        if ( lstValores.size() != 3 )
            continue;

        idPersona = lstValores.getString(0);
        nombre = lstValores.getString(1);
        pathFoto = lstValores.getString(2);

        cout << "Suavizado " << pixelSuavizado << endl; 
        cout << "ID:" << idPersona << " Nombre:" << nombre << " PATH:" << pathFoto << endl;
        
        GImage rostro = GDibujo::read(pathFotos + pathFoto);   
        if ( alturaImagenBase > 0 )
        {
            long long ancho;
            float factor = ((float)alturaImagenBase) / (float)rostro.altura;

            ancho = (int)(((float)rostro.ancho)*factor);
            rostro = rostro.cloneResize(ancho,alturaImagenBase);
        }
        GImage rostroOrig = rostro.clone();
        
        // cv::imshow("Original Escalada", rostro.imagenOpencv);

        // Simular compresión JPEG en memoria
        // std::vector<uchar> buf;
        // std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 30}; // Ajusta el nivel de calidad (ej. 40-60)
        // cv::imencode(".jpg", rostro.imagenOpencv, buf, params);
        // rostro.imagenOpencv = cv::imdecode(buf, cv::IMREAD_COLOR);
        
        cout << "Ancho del rostro " << rostro.ancho << " Altura " << rostro.altura << endl;
        lstDet = detector.detectar_2_5g(rostro, toleranciaDetec);        
        if ( detector.errorDeteccion == true )
        {
            cout << "Error al detectar rostros" << endl;
            break;
        }

        if ( lstDet.size() > 0 )
        {            
            DeteccionCaraHailo det = lstDet.at(0);
            DeteccionCaraHailo det1 = lstDet.at(0);
            cv::Mat blurred, tmp;  

            // cout << "Cara detectada" << endl;
                        
            if ( encuadrarRostros.compare("S") == 0 ) 
                det.ajustarCuadrado(rostro.ancho, rostro.altura);
            else
            if ( encuadrarRostros.compare("M") == 0 ) 
                det.ajustarMiniCuadrado(rostro.ancho, rostro.altura);
            

            rostroOrig = rostro.clone();

            // GDibujo::drawRect(rostroOrig, GRect(det.ptoSupIzq.x, det.ptoSupIzq.y, det.ptoInfDer.x, det.ptoInfDer.y), GColor(255,0,0), 2);
            // GDibujo::drawRect(rostroOrig, GRect(det1.ptoSupIzq.x, det1.ptoSupIzq.y, det1.ptoInfDer.x, det1.ptoInfDer.y), GColor(0,0,255), 2);
            
            // cv::imshow("Deteccion", rostro.imagenOpencv); 
            
            if ( pixelSuavizado > 0 )
            {
                          
                cv::GaussianBlur(rostro.imagenOpencv, blurred, cv::Size(pixelSuavizado,pixelSuavizado), 0);
                                
                // cv::imshow("Blured", rostro.imagenOpencv);
                // cv::waitKey(0);

                rostro.imagenOpencv = blurred;          
            }

            
            
            if ( resizeSuavizado > 1 )
            {   
                cv::resize(rostro.imagenOpencv , tmp, cv::Size(resizeSuavizado,resizeSuavizado), 0, 0, cv::INTER_AREA);
                cv::resize(tmp, rostro.imagenOpencv, rostro.imagenOpencv.size(), 0, 0, cv::INTER_LINEAR);

                // cv::imshow("Suavizado", rostro.imagenOpencv);
                // cv::waitKey(0);
            }
            else
            if ( ( resizeSuavizado > 0.0 ) && ( resizeSuavizado < 1 ))
            {   
                cv::resize(rostro.imagenOpencv , tmp, cv::Size(), resizeSuavizado, resizeSuavizado, cv::INTER_AREA);
                cv::resize(tmp, rostro.imagenOpencv, rostro.imagenOpencv.size(), 0, 0, cv::INTER_LINEAR);

                // cv::imshow("Suavizado", rostro.imagenOpencv);
                // cv::waitKey(0);
            }
                        
            if ( jpegSuavizado < 100 )
            {
                std::vector<uchar> buf;
                std::vector<int> p = { cv::IMWRITE_JPEG_QUALITY, 40 };
                cv::imencode(".jpg", rostro.imagenOpencv, buf, p);
                rostro.imagenOpencv = cv::imdecode(buf, cv::IMREAD_COLOR);

                // cv::imshow("JPEG", rostro.imagenOpencv);
                // cv::waitKey(0);
            }            

            GImage rostroExtraido = rostro.getRect(det.ptoSupIzq.x, det.ptoSupIzq.y, det.ptoInfDer.x, det.ptoInfDer.y);
            GImage rostroInvertido = GDibujo::flip(rostroExtraido, GDIBUJO_FLIP_VER);

            // cv::imshow("Extraido", rostroExtraido.imagenOpencv);
            // cv::imshow("Invertido", rostroInvertido.imagenOpencv);
            // cv::waitKey(0);

            det.desplazaPtosCara(-det.ptoSupIzq.x, -det.ptoSupIzq.y);
            det.desplazaRegion(-det.ptoSupIzq.x, -det.ptoSupIzq.y);
            
            SIMD_TYPE descriptorOrig[NUM_ELEMS_DESC_FACIAL];
            SIMD_TYPE descriptorFlip[NUM_ELEMS_DESC_FACIAL];
            SIMD_TYPE descriptorUnif[NUM_ELEMS_DESC_FACIAL];       
            // cout << "Calculando descriptor: " << rostroOrig.ancho << " x " << rostroOrig.altura << " Det: " << 
            //     det.ptoSupIzq.x << " , " << det.ptoSupIzq.y << " , " <<  
            //     det.ptoInfDer.x << " , " << det.ptoInfDer.y << endl;     
                        
            // cv::waitKey(0);
            generadorDesc.calculaDescriptor(rostroExtraido, det, descriptorOrig);     
            generadorDesc.calculaDescriptor(rostroInvertido, det, descriptorFlip);
            fuse_flip_embeddings(descriptorOrig, descriptorFlip, descriptorUnif);
            
            string fila = nombre;
            fila.append(",");
            fila.append(idPersona);
            fila.append(",");

            for(int j=0; j<NUM_ELEMS_DESC_FACIAL; j++)
            {
                fila.append(to_string(descriptorUnif[j]));
                fila.append(",");
            }
            fila.append("\n");
            archivoBd << fila;

            // cout << fila << endl;
        }
        else
        {
            cout << "!!! NO se detecto un rostro" << endl;
        }
    }

    archivoBd.close();

}


// L2-normaliza in-place (si el vector es todo ceros, lo deja tal cual)
void ExtractorFacialArchivo::l2_normalize(SIMD_TYPE *v) 
{
    float x,s = 0.0; 
    int i;
    for (i=0; i<NUM_ELEMS_DESC_FACIAL; i++) 
    {
        x = v[i];
        s += x*x;
    }        
    
    if (s <= 0.0) return;
    
    float inv = float(1.0 / std::sqrt(s));

    for (i=0; i<NUM_ELEMS_DESC_FACIAL; i++) 
    {
        x = v[i] * inv;
        v[i] = x;
    }
}

// coseno entre dos vectores (si ya están L2-normalizados, es el dot product)
float ExtractorFacialArchivo::cosine_sim(SIMD_TYPE *a, SIMD_TYPE *b) {
    
    float d = 0.0;
    for (size_t i=0;i<NUM_ELEMS_DESC_FACIAL;++i) d += float(a[i]) * float(b[i]);
    return float(d);
}

/**
 * Fusiona dos embeddings (e_orig y e_flip) en uno solo:
 * 1) L2-normaliza ambos
 * 2) Promedia con pesos (w_orig, w_flip)
 * 3) L2-normaliza el resultado
 * Si la similitud coseno entre ambos < min_cos_to_merge, devuelve solo e_orig normalizado.
 *
 * @param e_orig  embedding del rostro original
 * @param e_flip  embedding del mismo rostro con imagen espejada
 * e_unificado embedding unificado resultante
 * @param w_orig  peso del embedding original (default 0.5)
 * @param w_flip  peso del embedding flip (default 0.5)
 * @param min_cos_to_merge  umbral de similitud para fusionar (p.ej., 0.3–0.4)
 */
void ExtractorFacialArchivo::fuse_flip_embeddings(SIMD_TYPE *e_orig,
                                        SIMD_TYPE *e_flip,
                                        SIMD_TYPE *e_unificado,
                                        float w_orig ,
                                        float w_flip ,
                                        float min_cos_to_merge )
{
    // Copias para normalizar
    SIMD_TYPE e_orig1[NUM_ELEMS_DESC_FACIAL];
    SIMD_TYPE e_flip1[NUM_ELEMS_DESC_FACIAL];
    int i;

    for(i=0;i<NUM_ELEMS_DESC_FACIAL;i++)
    {
        e_orig1[i] = e_orig[i];
        e_flip1[i] = e_flip[i];
    }       

    l2_normalize(e_orig1);
    l2_normalize(e_flip1);

    // Si son muy distintos, no fusionar (evita “arrastrar” ruido)
    float cosab = cosine_sim(e_orig1, e_flip1);
    if (cosab < min_cos_to_merge) 
    {
        // devolvemos el original normalizado
        for(i=0;i<NUM_ELEMS_DESC_FACIAL;i++)
        {
            e_unificado[i] = e_orig[i];         
        }    
    }
    else
    {
        // se calcula la media ponderada
        for(i=0;i<NUM_ELEMS_DESC_FACIAL;i++)
        {
            e_unificado[i] =  w_orig*e_orig[i] + w_flip*e_flip[i];
        }        
    }
    l2_normalize(e_unificado);
}