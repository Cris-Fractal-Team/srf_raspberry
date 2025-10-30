

#include "lib/hailolib/FaceRecHailo.h"
#include "lib/hailolib/DetectionTrackerHailo.h"

#include <math.h>
#include <arm_neon.h>
#include <cmath>
#include "lib/utils/GStringUtils.h"

/**
 * Elimina las referencias que los universos de persona
 * tienen de este descriptor externo
 */
void DescPersonaExterno::reset()
{
    UnivIdenPersona *ptrUnivIden;
    int n = lstPersonas.size();

    for(int i=0; i<n; i++)
    {
        ptrUnivIden = lstPersonas.get(i);

    }
}

/**
 * Normaliza el descriptor
 */
void DescPersonaExterno::normaliza()
{
    float norm1 = 0.0;
    
    for(size_t ni=0;ni<512;ni++) norm1+= vecDescripcion[ni] * vecDescripcion[ni];
    norm1 = sqrt(norm1);
    for(size_t ni=0;ni<512;ni++) vecDescripcion[ni] /= norm1;

    norm_coceno_calculado = true;
}


/**
 * Constructor
 */
UnivIdenPersona::UnivIdenPersona()
{
    indiceGrupoPrin = -1;
    numIden = 0;
    cambioIdentificacion = false;
    fechaUltEvento = 0;
}


/**
 * Borra las referencias a descriptor externo
 */
void UnivIdenPersona::borrarDescPersonaExterno( DescPersonaExterno *descExterno )
{
    GrupoIdenPersona *grupo;
    int i,n;

    n = lstGrupoIden.size()-1;
    for(i=n; i >= n; i--)
    {
        grupo = lstGrupoIden.getAddr(i);
        if ( grupo->descExterno == descExterno )
        {
            grupo->lstIdentificaciones.reset();
            lstGrupoIden.remove(i);
        }
    }
}


/**
 * Libera RAM del objeto
 */
void UnivIdenPersona::reset()
{
    int i,n;
    GrupoIdenPersona *grupo;

    n = lstGrupoIden.size();
    for(i=0;i<n;i++)
    {
        grupo = lstGrupoIden.getAddr(i);
        grupo->lstIdentificaciones.reset();
    }

    lstGrupoIden.reset();
}

/**
 * Agrega una identificacion realizada al universo
 * 
 *      descExterno:
 *          Referencia al desriptor externo conocido de la persona.
 *      
 *      iden:
 *          Identificacion de una persona que se proceso y a la que se busco un descriptor externo
 * 
 *      fecDet: 
 *          Fecha en la que se hizo la deteccion
 * 
 */
void UnivIdenPersona::agregaIdentif( DescPersonaExterno *descExterno, IdentificacionPersona iden, long long fecDet )
{
    int i,n,len;
    bool encontro;
    GrupoIdenPersona *grupo;

    encontro = false;
    n = lstGrupoIden.size();
    for(i=0;i<n;i++)
    {
        grupo = lstGrupoIden.getAddr(i);
        if ( grupo->descExterno->id.compare(descExterno->id) == 0 )
        {
            // se encontro que ya existe ese descriptor en el universo
            iden.fecDet = fecDet;
            grupo->lstIdentificaciones.add(iden);
            len = grupo->lstIdentificaciones.size(); 
            if ( len > numIden )
            {
                if ( i != indiceGrupoPrin ) cambioIdentificacion = true;
                else cambioIdentificacion = false;

                indiceGrupoPrin = i;
                numIden = len;                
            }
            encontro = true;
            break;
        }
    }
    if ( encontro == false )
    {
        GrupoIdenPersona grupoNuevo;

        iden.fecDet = fecDet;
        grupoNuevo.descExterno = descExterno;
        grupoNuevo.fecCrea = fecDet;
        grupoNuevo.lstIdentificaciones.add(iden);

        lstGrupoIden.add(grupoNuevo);        
        if ( numIden == 0 )
        {
            // Es el primer grupo que se crea
            indiceGrupoPrin = 0;
            numIden = 1;
            cambioIdentificacion = true;
        }        

        grupoNuevo.lstIdentificaciones.reset();
    }
}

/**
 * Retorna un puntero con los datos de la persona
 * identificada.
 * Si no existe retorna NULL
 */
DescPersonaExterno* UnivIdenPersona::getDatosPerIden()
{
    if ( indiceGrupoPrin == -1 )
        return NULL;

    return lstGrupoIden.getAddr(indiceGrupoPrin)->descExterno;
}

/**
 * Retorna el promedio de las similaridades o distancias de comparacion
 * del grupo actual
 */
float UnivIdenPersona::getPromedioComparacion()
{
    if ( indiceGrupoPrin == -1 )
        return 0.0;

    GrupoIdenPersona grupo = lstGrupoIden.get(indiceGrupoPrin);

    float promedio = 0;
    int i,n = grupo.lstIdentificaciones.size();

    for(i=0;i<n;i++)
    {
        promedio += grupo.lstIdentificaciones.get(i).comparacion;
    }
    promedio/= (float)n;

    return promedio;
}

/**
 * Retorna la cantidad de identificaciones que tiene el grupo actual
 */
int UnivIdenPersona::getNumIdentificaciones()
{
    GrupoIdenPersona grupo = lstGrupoIden.get(indiceGrupoPrin);
    return grupo.lstIdentificaciones.size();
}

/**
 * Retorna la ultima identificacion
 */
IdentificacionPersona UnivIdenPersona::getUltimaIdentificacion()
{
    return lstGrupoIden.getLast().lstIdentificaciones.getLast();
}


/**
 * Constructor
 */
CaraDescrita::CaraDescrita()
{
    descCalculado = false;
    personaIdent = false;
}

/**
 * Calcula la foto de la cara en base a la deteccion 
 * extrallendola desde la foto en la que se realizo la deteccion
 */
void CaraDescrita::calculaFotoCara( GImage foto )
{
    fotoCara = foto.getRect(deteccion.ptoSupIzq.x, deteccion.ptoSupIzq.y, deteccion.ptoInfDer.x, deteccion.ptoInfDer.y);
}

/**
 * Constructor
 */
FaceRecHailo::FaceRecHailo()
{
    imgModeloAltura = 112;
    imgModeloAncho = 112;
    vectorIniciado = false;    
    errorCalculo = false;
    paramContraste = 2.0;
    maxBatchSize = 16;
    similaridadFrontal = 10.0;

    previsualizaImgReconocimiento = false;
    esperarPrevImgReconocimiento = false;
    guardarPrevImgReconocimiento = false;
    secuencialImgReconocimiento = 1;
    prefijoImgReconocimiento = "cara_";

     // puntos de referencia de facenet 112x112
    lstPuntosReferencia.push_back( cv::Point2f(38.2946, 51.6963) );
    lstPuntosReferencia.push_back( cv::Point2f(73.5318, 51.5014) );
    lstPuntosReferencia.push_back( cv::Point2f(56.0252, 71.7366) );
    lstPuntosReferencia.push_back( cv::Point2f(41.5493, 92.3655) );
    lstPuntosReferencia.push_back( cv::Point2f(70.7299, 92.2041) );
}


/**
 * Dimensiones de las imagenes que acepta la red
 */
void FaceRecHailo::setDimImagenes( int ancho, int altura )
{
    imgModeloAltura = altura;
    imgModeloAncho = ancho;
}

/**
 * Carga el modelo
 */
int FaceRecHailo::cargarModelo( string path )
{
    return runner.cargarRed(path);
}

/**
 * Ejecuta la deteccion
 */
bool FaceRecHailo::ejecutar( cv::Mat imagen, DeteccionCaraHailo caraDet )
{   
    auto inicio = std::chrono::high_resolution_clock::now();

    bool caraFrontal;
    cv::Rect region;
    cv::Mat imgCara,imgCaraAlineada,imagenRgb,imgCaraSinBlur;
    int ancho, altura;
    cv::Mat transAlineacion;
       
    ancho = caraDet.getAncho();
    altura = caraDet.getAltura();
        
    if (( imagen.cols >= ancho ) && ( imagen.rows >= altura ))
    {
        // obtiene la cara oringal desde la imagen
        region.x = caraDet.ptoSupIzq.x;
        region.y = caraDet.ptoSupIzq.y;
        region.width = ancho;
        region.height = altura;
        caraDet.desplazaPtosCara(-region.x, -region.y);

        transAlineacion = calculaMatrizAlineacion(caraDet);
        if ( transAlineacion.empty() || transAlineacion.rows != 2 || transAlineacion.cols != 3) return false;
        if (!cv::checkRange(transAlineacion, true)) return false; // sin NaN/Inf

        caraFrontal = caraDet.caraFrontal(&lstPuntosReferencia, transAlineacion, similaridadFrontal);
    
        imgCara = imagen(region).clone();        
        imagen = imgCara;
                
        // alinea el rostro y la convierte a RGB
        imgCaraAlineada = aliearRostro(imgCara, caraDet, transAlineacion);        
    }
    else
    {
        transAlineacion = calculaMatrizAlineacion(caraDet);
        if ( transAlineacion.empty() || transAlineacion.rows != 2 || transAlineacion.cols != 3) return false;
        if (!cv::checkRange(transAlineacion, true)) return false; // sin NaN/Inf

        caraFrontal = caraDet.caraFrontal(&lstPuntosReferencia, transAlineacion, similaridadFrontal) ;

        // alinea el rostro y la convierte a RGB
        imgCaraAlineada = aliearRostro(imagen, caraDet, transAlineacion);
        imgCara = imagen.clone();
    }

    auto fin_alinea = std::chrono::high_resolution_clock::now();

    // quita motion blur 
    GImage imagenSinBlur(imgCaraAlineada);
    imagenSinBlur.sharpenUnsharp();
    imgCaraSinBlur = imagenSinBlur.imagenOpencv;

    auto fin_blur = std::chrono::high_resolution_clock::now();
    
    if ( previsualizaImgReconocimiento == true )
    {   
        cv::imshow("cara-orig", imgCara);
        cv::imshow("cara-alin", imgCaraAlineada);
        cv::imshow("cara-sblur", imgCaraSinBlur);
        if ( esperarPrevImgReconocimiento == true )
        {
            int tecla = cv::waitKey(0);
            if ( guardarPrevImgReconocimiento == true )
            {
                if (( tecla == 'G' ) || ( tecla == 'g' )) 
                {
                    // guarda la cara visualizada
                    std::vector<int> parametros;
                    string pathCara = "./caras/cara_";
                    pathCara.append(GStringUtils::to_fixed_digits(secuencialImgReconocimiento,6));
                    pathCara.append(".png");
                    secuencialImgReconocimiento++;

                    parametros.push_back(cv::IMWRITE_PNG_COMPRESSION);
                    parametros.push_back(3); 

                    cout << "Guarda para alineada 1: " << pathCara << endl;
                    cv::imwrite(pathCara, imgCaraAlineada, parametros);
                }
            }
        }
    }    

    if ( caraFrontal == true ) 
    {
        if ( guardarCarasFrontalesAlineadas == true )
        {
            // guarda el rostro porque es frontal
            std::vector<int> parametros;
            string pathCara = "./caras/" + prefijoImgReconocimiento;
            pathCara.append(GStringUtils::to_fixed_digits(secuencialImgReconocimiento,6));
            pathCara.append(".png");
            secuencialImgReconocimiento++;

            parametros.push_back(cv::IMWRITE_PNG_COMPRESSION);
            parametros.push_back(3); 

            cout << "Guarda para alineada 2 : " << pathCara << endl;

            cv::imwrite(pathCara, imgCaraSinBlur, parametros);
        }
    }
    else
    {
        // no se procesa la cora no es frontal
        // cout << "La cara no es frontal" << endl;
        return false;
    }
    
    cv::cvtColor(imgCaraSinBlur, imagenRgb, cv::COLOR_BGR2RGB);
    
    // crea el buffer para ingresar la imagen
    if ( vectorIniciado == false )
    {
        input_buffer.resize(imagenRgb.total() * imagenRgb.elemSize(),0);
        vectorIniciado = true;
    }   
    std::memcpy(input_buffer.data(), imagenRgb.data, input_buffer.size());

    auto fin_copia = std::chrono::high_resolution_clock::now();

    bool rpta = runner.ejecutarInferencia(input_buffer.data(), input_buffer.size());

    auto fin_ejecuta = std::chrono::high_resolution_clock::now();

    auto durAlinea = std::chrono::duration_cast<std::chrono::microseconds>(fin_alinea - inicio);
    auto durBlur = std::chrono::duration_cast<std::chrono::microseconds>(fin_blur - fin_alinea);
    auto durCopia = std::chrono::duration_cast<std::chrono::microseconds>(fin_copia - fin_blur);
    auto durEjecuta = std::chrono::duration_cast<std::chrono::microseconds>(fin_ejecuta - fin_copia);
    auto durTotal = std::chrono::duration_cast<std::chrono::microseconds>(fin_ejecuta - inicio);

    // cout << "-- Tiempos descriptor microsegundos " << endl << endl;
    // cout << "-- Alinea      : " << durAlinea.count() << endl;
    // cout << "-- Blur      : " << durBlur.count() << endl;
    // cout << "-- Copia : " << durCopia.count() << endl;
    // cout << "-- Descriptor : " << durEjecuta.count() << endl;
    cout << ">> TOTAL FaceRec      : " << durTotal.count() << endl;
    // cout << endl;

    return rpta;
}

/**
 * Ejecuta la deteccion, sobre una lista de imagenes
 */
bool FaceRecHailo::ejecutar( GLinkedList<cv::Mat> *lstImagenes, GLinkedList<DeteccionCaraHailo> *lstDetecciones )
{   
    cv::Rect region;
    DeteccionCaraHailo caraDet;
    cv::Mat imagen,imgCara,imgCaraAlineada,imagenRgb,imgCaraSinBlur;
    int ancho, altura,iImg,nImg, totalBytesGuardados;
    size_t numBytesImagen = 0;
    bool caraFrontal = false;
    cv::Mat matTrans;

    uchar *ptrData = input_buffer.data();

    imagen = lstImagenes->get(0);
    numBytesImagen = 112 * 112 * 3;
    totalBytesGuardados = 0;

    // crea el buffer para ingresar la imagen
    if ( vectorIniciado == false )
    {        
        input_buffer.resize(numBytesImagen * maxBatchSize,0);
        ptrData = input_buffer.data();
        vectorIniciado = true;
    }
    
    nImg = lstImagenes->size();
    lstIndicesRostrosProc.reset();

    for(iImg=0; iImg<nImg; iImg++)
    {
        imagen = lstImagenes->get(iImg);
        caraDet = lstDetecciones->get(iImg);
              
        ancho = caraDet.getAncho();
        altura = caraDet.getAltura();
            
        if (( imagen.cols >= ancho ) && ( imagen.rows >= altura ))
        {
            // obtiene la cara oringal desde la imagen
            region.x = caraDet.ptoSupIzq.x;
            region.y = caraDet.ptoSupIzq.y;
            region.width = ancho;
            region.height = altura;
            caraDet.desplazaPtosCara(-region.x, -region.y);

            // validamos si la cara se debe o no analizar
            matTrans = calculaMatrizAlineacion(caraDet);

            caraFrontal = true;
            if ( matTrans.empty() || matTrans.rows != 2 || matTrans.cols != 3) caraFrontal = false;
            if (!cv::checkRange(matTrans, true)) caraFrontal = false; // sin NaN/Inf

            if ( caraFrontal == true )
                caraFrontal = caraDet.caraFrontal(&lstPuntosReferencia, matTrans, similaridadFrontal);

            if ( caraFrontal == true )
            {
                imgCara = imagen(region).clone();        
                imagen = imgCara;
                        
                // alinea el rostro y la convierte a RGB
                imgCaraAlineada = aliearRostro(imgCara, caraDet, matTrans);  
            }            
        }
        else
        {
            // validamos si la cara se debe o no analizar
            matTrans = calculaMatrizAlineacion(caraDet);

            caraFrontal = true;
            if ( matTrans.empty() || matTrans.rows != 2 || matTrans.cols != 3) caraFrontal = false;
            if (!cv::checkRange(matTrans, true)) caraFrontal = false; // sin NaN/Inf

            if ( caraFrontal == true )        
                caraFrontal = caraDet.caraFrontal(&lstPuntosReferencia, matTrans, similaridadFrontal);
        
            if ( caraFrontal == true )
            {
                // alinea el rostro y la convierte a RGB
                imgCaraAlineada = aliearRostro(imagen, caraDet, matTrans);
                imgCara = imagen.clone();
            }            
        }

        if ( caraFrontal == true )
        {
            // quita motion blur 
            GImage imagenSinBlur(imgCaraAlineada);
            imagenSinBlur.sharpenUnsharp();
            imgCaraSinBlur = imagenSinBlur.imagenOpencv;
        }
        
        if (( previsualizaImgReconocimiento == true ) && ( caraFrontal == true ))
        {
            cv::imshow("cara-orig", imgCara);
            cv::imshow("cara-alin", imgCaraAlineada);
            cv::imshow("cara-sblur", imgCaraSinBlur);
                        
            if ( esperarPrevImgReconocimiento == true )
            {
                int tecla = cv::waitKey(0);
                if ( guardarPrevImgReconocimiento == true )
                {
                    if (( tecla == 'G' ) || ( tecla == 'g' )) 
                    {
                        // guarda la cara visualizada
                        std::vector<int> parametros;
                        string pathCara = "./caras/" + prefijoImgReconocimiento;
                        pathCara.append(GStringUtils::to_fixed_digits(secuencialImgReconocimiento,6));
                        pathCara.append(".png");
                        secuencialImgReconocimiento++;

                        parametros.push_back(cv::IMWRITE_PNG_COMPRESSION);
                        parametros.push_back(3); 
                        cout << "Guardando cara alineada 3:" << pathCara << endl;
                        cv::imwrite(pathCara, imgCaraAlineada, parametros);
                    }
                }
            }
        }    
        else
        if ( caraFrontal == false )
        {
            // cout << "La cara no es frontal" << endl;
        }

        if ( caraFrontal == true ) 
        {                    
            if ( guardarCarasFrontalesAlineadas == true )
            {
                // guarda el rostro porque es frontal
                std::vector<int> parametros;
                string pathCara = "./caras/" + prefijoImgReconocimiento ;
                pathCara.append(GStringUtils::to_fixed_digits(secuencialImgReconocimiento,6));
                pathCara.append(".png");
                secuencialImgReconocimiento++;

                parametros.push_back(cv::IMWRITE_PNG_COMPRESSION);
                parametros.push_back(3); 

                cout << "Guardando cara alineada 3:" << pathCara << endl;
                cv::imwrite(pathCara, imgCaraSinBlur, parametros);
            }

            // se encola la cara para calcular su descriptor
            cv::cvtColor(imgCaraSinBlur, imagenRgb, cv::COLOR_BGR2RGB);            
            std::memcpy(ptrData, imagenRgb.data, numBytesImagen);
            ptrData+= numBytesImagen;
            totalBytesGuardados+= numBytesImagen;
            lstIndicesRostrosProc.add(iImg);
        }                    
    }

    bool rpta;
    
    if  ( lstIndicesRostrosProc.size() > 0 )
        rpta =  runner.ejecutarInferencia(input_buffer.data(), totalBytesGuardados, lstIndicesRostrosProc.size());
    else
        rpta = false;

    return rpta;
}


/**
 * Calcula la matriz de alineacion para un rostro detectado
 */
cv::Mat FaceRecHailo::calculaMatrizAlineacion( DeteccionCaraHailo deteccion )
{
    std::vector<cv::Point2f> lstPuntos;

    lstPuntos.push_back(cv::Point2f(deteccion.ojoIzq.x, deteccion.ojoIzq.y));
    lstPuntos.push_back(cv::Point2f(deteccion.ojoDer.x, deteccion.ojoDer.y));
    lstPuntos.push_back(cv::Point2f(deteccion.nariz.x, deteccion.nariz.y));
    lstPuntos.push_back(cv::Point2f(deteccion.bocaIzq.x, deteccion.bocaIzq.y));
    lstPuntos.push_back(cv::Point2f(deteccion.bocaDer.x, deteccion.bocaDer.y));

    cv::Mat transformacion = cv::estimateAffine2D(lstPuntos, lstPuntosReferencia);

    return transformacion;
}

/**
 * Dada una cara detectada , la alinea para ejecutar el reconocimiento
 * facial, usando como dato los puntos de una deteccion previa
 */
cv::Mat FaceRecHailo::aliearRostro( cv::Mat fotoCara, DeteccionCaraHailo deteccion, cv::Mat transformacion )
{    
    cv::Mat suavizado;
    cv::Mat imgAlineada, fotoCaraContraste;
    try
    {        
        GImage imagen(fotoCara);
        int d = 5;
        double sigmaColor = 35;
        double sigmaSpace = 7;

        // cv::imshow("cara-suavizada", imagen.imagenOpencv);

        // suaiza ruido converva bordes
        cv::bilateralFilter(imagen.imagenOpencv, suavizado, d, sigmaColor, sigmaSpace);
        imagen.imagenOpencv = suavizado;

        // cv::imshow("cara-suavizada", suavizado);

        if ( paramContraste > 0 )
            imagen.aplicarCLAHE(paramContraste);

        // transformacion = calculaMatrizAlineacion(deteccion);
        // transformacion = cv::estimateAffine2D(lstPuntos, lstPuntosReferencia);
        // transformacion = cv::estimateAffinePartial2D(lstPuntos, lstPuntosRef);
            
        // cv::warpAffine(imagen.imagenOpencv, imgAlineada, transformacion, cv::Size(imgModeloAncho, imgModeloAltura));
        // cv::warpAffine(imagen.imagenOpencv, imgAlineada, transformacion, cv::Size(imgModeloAncho, imgModeloAltura), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));
        // cv::warpAffine(imagen.imagenOpencv, imgAlineada, transformacion, cv::Size(imgModeloAncho, imgModeloAltura), cv::INTER_LINEAR, cv::BORDER_REPLICATE, cv::Scalar(128,128,128));

        if ( imagen.ancho > 120 ) cv::warpAffine(imagen.imagenOpencv, imgAlineada, transformacion, cv::Size(imgModeloAncho, imgModeloAltura), cv::INTER_AREA, cv::BORDER_CONSTANT, cv::Scalar(128,128,128));
        else cv::warpAffine(imagen.imagenOpencv, imgAlineada, transformacion, cv::Size(imgModeloAncho, imgModeloAltura), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(128,128,128));

        return imgAlineada;
    }
    catch (const std::exception& e) 
    {
        // Manejo de la excepción
        std::cerr << "Excepción atrapada: " << e.what() << std::endl;
        return fotoCara;
    }    
}

/**
 * Extrae el descriptor facial de la ultima inferencia
 */
void FaceRecHailo::extraeDescriptor( SIMD_TYPE *descriptor, int offset )
{
    hailo_vstream_info_t info;
    std::vector<SIMD_TYPE> rpta;
    const uint8_t *lstDescriptor;
    vector<uint8_t>vecDescriptor;
    SIMD_TYPE valor;
    // string capaSalida = "arcface_mobilefacenet/fc1";

    // info = runner.getOutVsTramInfo(capaSalida);
    info = runner.getOutVsTramInfo(nombreUltimaCapaModelo);

    // vecDescriptor = runner.getOutputStream(capaSalida);
    vecDescriptor = runner.getOutputStream(nombreUltimaCapaModelo);
    lstDescriptor = vecDescriptor.data();

    for(int i=0; i<NUM_ELEMS_DESC_FACIAL; i++)
    {
        #ifdef USE_INT16
            valor = lstDescriptor[offset++];
        #else 
            valor = runner.dequantize(lstDescriptor[offset++], &info.quant_info);
        #endif

        descriptor[i] = valor;
    }
}

/**
 * Extrae el descriptor facial de la ultima inferencia en formato de OpenCV
 */
cv::Mat FaceRecHailo::extraeDescriptorMat()
{
    hailo_vstream_info_t info;
    cv::Mat rpta(1,NUM_ELEMS_DESC_FACIAL, CV_32F);
    const uint8_t *lstDescriptor;
    vector<uint8_t>vecDescriptor;
    float* fila = rpta.ptr<float>(0);
    SIMD_TYPE valor;
    string capaSalida = "arcface_mobilefacenet/fc1";

    info = runner.getOutVsTramInfo(capaSalida);

    vecDescriptor = runner.getOutputStream(capaSalida);
    lstDescriptor = vecDescriptor.data();

    for(int i=0; i<NUM_ELEMS_DESC_FACIAL; i++)
    {
        #ifdef USE_INT16
            valor = lstDescriptor[i];
        #else 
            valor = runner.dequantize(lstDescriptor[i], &info.quant_info);
        #endif

        fila[i] = (float)valor;
    }

    return rpta;
}

// float FaceRecHailo::calculaDiferenciaSIMD( vector<float>desc1, vector<float>desc2 ) 
// {
//     size_t size = desc1.size();
//     float *val1 = desc1.data();
//     float *val2 = desc2.data();

//     size_t simd_size = size / 4;  // Cada registro SIMD maneja 4 floats
//     float32x4_t sum_vec = vdupq_n_f32(0.0f);  // Inicializa a cero el acumulador SIMD
    
//     // Procesamiento en bloques de 4 floats
//     for (size_t i = 0; i < simd_size * 4; i += 4) {
//         float32x4_t v1 = vld1q_f32(&val1[i]);    // Carga 4 elementos de arr1
//         float32x4_t v2 = vld1q_f32(&val2[i]);    // Carga 4 elementos de arr2
//         float32x4_t diff = vsubq_f32(v1, v2);    // Resta los vectores
//         float32x4_t sq = vmulq_f32(diff, diff);  // Eleva al cuadrado
//         sum_vec = vaddq_f32(sum_vec, sq);        // Acumula
//     }

//     // Reduce el vector SIMD a un escalar
//     float result_array[4];
//     vst1q_f32(result_array, sum_vec);
//     float sum = result_array[0] + result_array[1] + result_array[2] + result_array[3];

//     // Procesa los elementos restantes (si el tamaño no es múltiplo de 4)
//     for (size_t i = simd_size * 4; i < size; ++i) {
//         float diff = val1[i] - val2[i];
//         sum += diff * diff;
//     }

//     // return std::sqrt(sum);  // Devuelve la raíz cuadrada del resultado
//     return sum;
// }


/**
 * Calcula el descriptor facial para una cara
 */
bool FaceRecHailo::calculaDescriptor( GImage imagen, DeteccionCaraHailo deteccion, SIMD_TYPE *descriptor )
{    
    try
    {
        if ( ejecutar(imagen.imagenOpencv, deteccion)  == false )
        {
            errorCalculo = true;
            return false;
        }
    }
    catch( Exception e )
    {
        cout << "Error calculando descriptor para una imagen : " << e.msg << endl;
        return false;
    }

    errorCalculo = false;
    extraeDescriptor(descriptor);

    return true;
}


/**
 * Calcula el descriptor facial para una lista de caras detectadas
 */
bool FaceRecHailo::calculaDescriptor( GLinkedList<TrackedDetectionHailo *> *lstDeteccionesTrack )
{    
    int i,n,offset,indiceImg;
    GImage img;
    TrackedDetectionHailo *deteccionTr;
    SIMD_TYPE *descriptor;
    GLinkedList<cv::Mat> lstImgOpenCv;
    GLinkedList<DeteccionCaraHailo> lstDetecciones;
    
    n = lstDeteccionesTrack->size();
    for(i=0; i<n; i++)
    {
        deteccionTr = lstDeteccionesTrack->get(i);
        deteccionTr->cara.descCalculado = false;
        img = deteccionTr->cara.fotoCara;
        lstImgOpenCv.add(img.imagenOpencv);
        lstDetecciones.add(deteccionTr->cara.detRelCara);
    }

    try
    {
        if ( ejecutar(&lstImgOpenCv, &lstDetecciones)  == false )
        {
            errorCalculo = true;
            lstImgOpenCv.reset();
            return false;
        }
    }
    catch( Exception e )
    {
        cout << "Error al ejecutar calculo de descripcion para una lista: " << e.msg << endl;
        errorCalculo = true;
        return false;
    }

    errorCalculo = false;

    offset = 0;
    n = lstIndicesRostrosProc.size();
    for(i=0; i<n; i++)
    {
        indiceImg = lstIndicesRostrosProc.get(i);
        deteccionTr = lstDeteccionesTrack->get(indiceImg);
        deteccionTr->cara.descCalculado = true;
        descriptor = deteccionTr->cara.descriptor;
        extraeDescriptor(descriptor, offset);
        offset+= NUM_ELEMS_DESC_FACIAL;
    }    

    lstDetecciones.reset();
    lstImgOpenCv.reset();

    return true;
}
