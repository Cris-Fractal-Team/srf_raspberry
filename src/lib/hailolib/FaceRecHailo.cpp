

#include "lib/hailolib/FaceRecHailo.h"

#include <math.h>
#include <arm_neon.h>
#include <cmath>

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
    cv::Rect region;
    cv::Mat imgCara,imgCaraAlineada,imagenRgb;
    int ancho, altura;
       
    ancho = caraDet.getAncho();
    altura = caraDet.getAltura();
        
    if (( imagen.cols > ancho ) || ( imagen.rows > altura ))
    {
        // obtiene la cara oringal desde la imagen
        region.x = caraDet.ptoSupIzq.x;
        region.y = caraDet.ptoSupIzq.y;
        region.width = ancho;
        region.height = altura;
        caraDet.desplazaPtosCara(-region.x, -region.y);
    
        imgCara = imagen(region).clone();
        
        
        // alinea el rostro y la convierte a RGB
        imgCaraAlineada = aliearRostro(imgCara, caraDet);
    }
    else
    {
        // alinea el rostro y la convierte a RGB
        imgCaraAlineada = aliearRostro(imagen, caraDet);
    }

    // cv::imshow("cara", imgCaraAlineada);
    // cv::waitKey(0);

    cv::cvtColor(imgCaraAlineada, imagenRgb, cv::COLOR_BGR2RGB);
    imgCaraAlineada/= 255.0;
    
    // crea el buffer para ingresar la imagen
    if ( vectorIniciado == false )
    {
        input_buffer.resize(imagenRgb.total() * imagenRgb.elemSize(),0);
        vectorIniciado = true;
    }   
    std::memcpy(input_buffer.data(), imagenRgb.data, input_buffer.size());

    if ( runner.ejecutarInferencia(input_buffer.data(), input_buffer.size()) == false ) 
        return false;    

    return true;
}


/**
 * Dada una cara detectada , la alinea para ejecutar el reconocimiento
 * facial, usando como dato los puntos de una deteccion previa
 */
cv::Mat FaceRecHailo::aliearRostro( cv::Mat fotoCara, DeteccionCaraHailo deteccion )
{
    std::vector<cv::Point2f> lstPuntos;
    std::vector<cv::Point2f> lstPuntosRef;
    

    // puntos de referencia de facenet 112x112
    lstPuntosRef.push_back( cv::Point2f(38.2946, 51.6963) );
    lstPuntosRef.push_back( cv::Point2f(73.5318, 51.5014) );
    lstPuntosRef.push_back( cv::Point2f(56.0252, 71.7366) );
    lstPuntosRef.push_back( cv::Point2f(41.5493, 92.3655) );
    lstPuntosRef.push_back( cv::Point2f(70.7299, 92.2041) );

    lstPuntos.push_back(cv::Point2f(deteccion.ojoIzq.x, deteccion.ojoIzq.y));
    lstPuntos.push_back(cv::Point2f(deteccion.ojoDer.x, deteccion.ojoDer.y));
    lstPuntos.push_back(cv::Point2f(deteccion.nariz.x, deteccion.nariz.y));
    lstPuntos.push_back(cv::Point2f(deteccion.bocaIzq.x, deteccion.bocaIzq.y));
    lstPuntos.push_back(cv::Point2f(deteccion.bocaDer.x, deteccion.bocaDer.y));

    cv::Mat transformacion;
    cv::Mat imgAlineada;
    try
    {
        transformacion = cv::estimateAffine2D(lstPuntos, lstPuntosRef);
        cv::warpAffine(fotoCara, imgAlineada, transformacion, cv::Size(imgModeloAncho, imgModeloAltura));

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
void FaceRecHailo::extraeDescriptor( SIMD_TYPE *descriptor )
{
    hailo_vstream_info_t info;
    std::vector<SIMD_TYPE> rpta;
    const uint8_t *lstDescriptor;
    vector<uint8_t>vecDescriptor;
    SIMD_TYPE valor;
    string capaSalida = "arcface_mobilefacenet/fc1";

    info = runner.getOutVsTramInfo(capaSalida);

    vecDescriptor = runner.getOutputStream(capaSalida);
    lstDescriptor = vecDescriptor.data();

    for(int i=0; i<512; i++)
    {
        #ifdef USE_INT16
            valor = lstDescriptor[i];
        #else 
            valor = runner.dequantize(lstDescriptor[i], &info.quant_info);
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
    cv::Mat rpta(1,512, CV_32F);
    const uint8_t *lstDescriptor;
    vector<uint8_t>vecDescriptor;
    float* fila = rpta.ptr<float>(0);
    SIMD_TYPE valor;
    string capaSalida = "arcface_mobilefacenet/fc1";

    info = runner.getOutVsTramInfo(capaSalida);

    vecDescriptor = runner.getOutputStream(capaSalida);
    lstDescriptor = vecDescriptor.data();

    for(int i=0; i<512; i++)
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
    if ( ejecutar(imagen.imagenOpencv, deteccion)  == false )
    {
        errorCalculo = true;
        return false;
    }
    errorCalculo = false;
    extraeDescriptor(descriptor);

    return true;
}
