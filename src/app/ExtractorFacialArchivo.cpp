#include "app/ExtractorFacialArchivo.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/GStringUtils.h"
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/utils/Logger.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

static constexpr const char* LOG_COMPONENT = "ExtractorFacialArchivo";

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
    vector<DeteccionCaraHailo> deteccionesRostro;
    string lineaCsv;
    string idPersona;
    string pathFoto;
    string nombre;

    string datosArchivo = leeArchivoTexto(pathArchivoDatos);
    GVector lineasArchivo = GStringUtils::split(datosArchivo, "\n");

    DetectorCarasHailoSCRFD detector;
    FaceRecHailo generadorDesc;

    std::ofstream archivoBd(pathArchivoBD, std::ios::out);

    long long totalLineasProcesadas = 0;
    long long lineasInvalidas = 0;
    long long erroresLecturaImagen = 0;
    long long erroresDeteccion = 0;
    long long sinRostroDetectado = 0;
    long long erroresDescriptor = 0;
    long long aciertos = 0;

    if ( !archivoBd.is_open() )
    {
        LOG_ERROR(LOG_COMPONENT, "Error: no se pudo abrir/crear el archivo de BD. path=" << pathArchivoBD);
        return;
    }

    if (vdevice == nullptr || !(*vdevice))
    {
        LOG_ERROR(LOG_COMPONENT, "Error: VDevice no inicializado en ExtractorFacialArchivo.");
        return;
    }

    try
    {
        // configura el detector
        detector.setDimImagenes(640,640);
        detector.runner.device = vdevice;

        if ( detector.cargarModelo("./models/scrfd_2.5ga.hef") != 0 )
        {
            LOG_ERROR(LOG_COMPONENT, "Error al cargar el modelo detector. path=./models/scrfd_2.5ga.hef");
            return;
        }

        LOG_INFO(LOG_COMPONENT, "Detector de rostros iniciado");

        // crea el generador de descriptores faciales
        generadorDesc.runner.device = vdevice;
        generadorDesc.paramContraste = paramContraste;
        generadorDesc.nombreUltimaCapaModelo = nombreCapaSalidaRedFacial;
        generadorDesc.guardarCarasFrontalesAlineadas = guardarCarasFrontalesAlineadas;
        generadorDesc.previsualizaImgReconocimiento = previsualizaImgReconocimiento;
        generadorDesc.esperarPrevImgReconocimiento = esperarPrevImgReconocimiento;

        if ( generadorDesc.cargarModelo(pathModeloDescFacial) != 0 )
        {
            LOG_ERROR(LOG_COMPONENT, "Error al cargar modelo generador de descriptores. path=" << pathModeloDescFacial);
            return;
        }

        LOG_INFO(LOG_COMPONENT, "Generador de descriptores iniciado");

        int cantidadLineas = lineasArchivo.size();
        for(int indiceLinea = 0; indiceLinea < cantidadLineas; indiceLinea++)
        {
            lineaCsv = lineasArchivo.getString(indiceLinea);
            lineaCsv = GStringUtils::trim(lineaCsv);

            if ( lineaCsv.empty() )
            {
                continue;
            }

            totalLineasProcesadas++;

            try
            {
                GVector columnasCsv = GStringUtils::split(lineaCsv, ",");
                if ( columnasCsv.size() != 3 )
                {
                    lineasInvalidas++;
                    LOG_WARN(LOG_COMPONENT, "Fila inválida (se esperan 3 columnas). fila=" << lineaCsv);
                    continue;
                }

                idPersona = columnasCsv.getString(0);
                nombre = columnasCsv.getString(1);
                pathFoto = columnasCsv.getString(2);

                string pathCompletoFoto = pathFotos + pathFoto;

                LOG_INFO(LOG_COMPONENT,
                         "Procesando -> ID=" << idPersona
                                             << " Nombre=" << nombre
                                             << " PATH=" << pathCompletoFoto);

                GImage imagenRostro;
                try
                {
                    imagenRostro = GDibujo::read(pathCompletoFoto);
                }
                catch (const std::exception& ex)
                {
                    erroresLecturaImagen++;
                    LOG_ERROR(LOG_COMPONENT,
                              "Error leyendo imagen -> idPersona=" << idPersona
                                                                   << " path=" << pathCompletoFoto
                                                                   << " ex=" << ex.what());
                    continue;
                }
                catch (...)
                {
                    erroresLecturaImagen++;
                    LOG_ERROR(LOG_COMPONENT,
                              "Error desconocido leyendo imagen -> idPersona=" << idPersona
                                                                               << " path=" << pathCompletoFoto);
                    continue;
                }

                if ( alturaImagenBase > 0 )
                {
                    long long anchoEscalado;
                    float factorEscala = ((float)alturaImagenBase) / (float)imagenRostro.altura;

                    anchoEscalado = (int)(((float)imagenRostro.ancho) * factorEscala);
                    imagenRostro = imagenRostro.cloneResize(anchoEscalado, alturaImagenBase);
                }

                LOG_DEBUG(LOG_COMPONENT,
                          "Imagen -> ancho=" << imagenRostro.ancho
                                             << " alto=" << imagenRostro.altura
                                             << " suavizado=" << pixelSuavizado
                                             << " resizeSuavizado=" << resizeSuavizado
                                             << " jpegSuavizado=" << jpegSuavizado);

                deteccionesRostro = detector.detectar_2_5g(imagenRostro, toleranciaDetec);
                if ( detector.errorDeteccion == true )
                {
                    erroresDeteccion++;
                    LOG_ERROR(LOG_COMPONENT,
                              "Error al detectar rostros -> idPersona=" << idPersona
                                                                        << " path=" << pathCompletoFoto);
                    continue;
                }

                if ( deteccionesRostro.size() <= 0 )
                {
                    sinRostroDetectado++;
                    LOG_ERROR(LOG_COMPONENT,
                              "Falta rostro detectado -> idPersona=" << idPersona
                                                                     << " path=" << pathCompletoFoto);
                    continue;
                }

                DeteccionCaraHailo deteccionPrincipal = deteccionesRostro.at(0);

                if ( encuadrarRostros.compare("S") == 0 )
                {
                    deteccionPrincipal.ajustarCuadrado(imagenRostro.ancho, imagenRostro.altura);
                }
                else
                if ( encuadrarRostros.compare("M") == 0 )
                {
                    deteccionPrincipal.ajustarMiniCuadrado(imagenRostro.ancho, imagenRostro.altura);
                }

                cv::Mat imagenBlur;
                cv::Mat imagenTmpResize;

                if ( pixelSuavizado > 0 )
                {
                    cv::GaussianBlur(imagenRostro.imagenOpencv,
                                     imagenBlur,
                                     cv::Size(pixelSuavizado,pixelSuavizado),
                                     0);

                    imagenRostro.imagenOpencv = imagenBlur;
                }

                if ( resizeSuavizado > 1 )
                {
                    cv::resize(imagenRostro.imagenOpencv,
                               imagenTmpResize,
                               cv::Size(resizeSuavizado,resizeSuavizado),
                               0, 0, cv::INTER_AREA);

                    cv::resize(imagenTmpResize,
                               imagenRostro.imagenOpencv,
                               imagenRostro.imagenOpencv.size(),
                               0, 0, cv::INTER_LINEAR);
                }
                else
                if ( ( resizeSuavizado > 0.0 ) && ( resizeSuavizado < 1 ) )
                {
                    cv::resize(imagenRostro.imagenOpencv,
                               imagenTmpResize,
                               cv::Size(),
                               resizeSuavizado, resizeSuavizado,
                               cv::INTER_AREA);

                    cv::resize(imagenTmpResize,
                               imagenRostro.imagenOpencv,
                               imagenRostro.imagenOpencv.size(),
                               0, 0, cv::INTER_LINEAR);
                }

                if ( jpegSuavizado < 100 )
                {
                    std::vector<uchar> jpegBuffer;
                    std::vector<int> jpegParams = { cv::IMWRITE_JPEG_QUALITY, 40 };

                    cv::imencode(".jpg", imagenRostro.imagenOpencv, jpegBuffer, jpegParams);
                    imagenRostro.imagenOpencv = cv::imdecode(jpegBuffer, cv::IMREAD_COLOR);
                }

                GImage rostroExtraido = imagenRostro.getRect(deteccionPrincipal.ptoSupIzq.x,
                                                            deteccionPrincipal.ptoSupIzq.y,
                                                            deteccionPrincipal.ptoInfDer.x,
                                                            deteccionPrincipal.ptoInfDer.y);

                GImage rostroInvertido = GDibujo::flip(rostroExtraido, GDIBUJO_FLIP_VER);

                deteccionPrincipal.desplazaPtosCara(-deteccionPrincipal.ptoSupIzq.x,
                                                   -deteccionPrincipal.ptoSupIzq.y);

                deteccionPrincipal.desplazaRegion(-deteccionPrincipal.ptoSupIzq.x,
                                                  -deteccionPrincipal.ptoSupIzq.y);

                SIMD_TYPE descriptorOriginal[NUM_ELEMS_DESC_FACIAL];
                SIMD_TYPE descriptorFlip[NUM_ELEMS_DESC_FACIAL];
                SIMD_TYPE descriptorUnificado[NUM_ELEMS_DESC_FACIAL];

                try
                {
                    generadorDesc.calculaDescriptor(rostroExtraido, deteccionPrincipal, descriptorOriginal);
                    generadorDesc.calculaDescriptor(rostroInvertido, deteccionPrincipal, descriptorFlip);
                    fuse_flip_embeddings(descriptorOriginal, descriptorFlip, descriptorUnificado);
                }
                catch (const std::exception& ex)
                {
                    erroresDescriptor++;
                    LOG_ERROR(LOG_COMPONENT,
                              "Error calculando descriptor -> idPersona=" << idPersona
                                                                          << " path=" << pathCompletoFoto
                                                                          << " ex=" << ex.what());
                    continue;
                }
                catch (...)
                {
                    erroresDescriptor++;
                    LOG_ERROR(LOG_COMPONENT,
                              "Error desconocido calculando descriptor -> idPersona=" << idPersona
                                                                                      << " path=" << pathCompletoFoto);
                    continue;
                }

                string filaSalida = nombre;
                filaSalida.append(",");
                filaSalida.append(idPersona);
                filaSalida.append(",");

                for(int indiceElem = 0; indiceElem < NUM_ELEMS_DESC_FACIAL; indiceElem++)
                {
                    filaSalida.append(to_string(descriptorUnificado[indiceElem]));
                    filaSalida.append(",");
                }

                filaSalida.append("\n");
                archivoBd << filaSalida;

                aciertos++;
            }
            catch (const std::exception& ex)
            {
                erroresDescriptor++;
                LOG_ERROR(LOG_COMPONENT, "Excepción por fila. fila=" << lineaCsv << " ex=" << ex.what());
                continue;
            }
            catch (...)
            {
                erroresDescriptor++;
                LOG_ERROR(LOG_COMPONENT, "Excepción desconocida por fila. fila=" << lineaCsv);
                continue;
            }
        }

        LOG_INFO(LOG_COMPONENT,
                 "Resumen -> totalFilas=" << totalLineasProcesadas
                                          << " aciertos=" << aciertos
                                          << " sinRostro=" << sinRostroDetectado
                                          << " errLecturaImg=" << erroresLecturaImagen
                                          << " errDeteccion=" << erroresDeteccion
                                          << " errDescriptor=" << erroresDescriptor
                                          << " filasInvalidas=" << lineasInvalidas);
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción en ejecutar: " << ex.what());
    }
    catch (...)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción desconocida en ejecutar");
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