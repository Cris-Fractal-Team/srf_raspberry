#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

#include "app/ComparadorFacial.h"
#include "app/ExtractorFacialArchivo.h"
#include "app/LectorConfig.h"
#include "app/PreProcesadorDataSet.h"
#include "app/ProcRecFacial.h"
#include "lib/graphics/ImageSource.h"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/Logger.h"
#include "lib/utils/systemUtils.h"

#include "lib/hailolib/hailo8l.h"

namespace
{
    constexpr const char* LOG_COMP = "MAIN";

    LogLevel parsearNivelLog(const std::string& valor)
    {
        std::string upper;
        upper.reserve(valor.size());
        for (char caracter : valor)
        {
            upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(caracter))));
        }

        if (upper == "DEBUG") return LogLevel::DEBUG;
        if (upper == "INFO") return LogLevel::INFO;
        if (upper == "WARN" || upper == "WARNING") return LogLevel::WARNING;
        if (upper == "ERROR") return LogLevel::ERROR;
        if (upper == "CRITICAL") return LogLevel::CRITICAL;

        return LogLevel::INFO;
    }

    void configurarLoggerDesdeConfig(const std::shared_ptr<GHashMap>& config)
    {
        std::string nivelLogStr = config->getString("logLevel");
        if (nivelLogStr.empty())
        {
            nivelLogStr = "INFO";
        }

        const LogLevel nivelMinimo = parsearNivelLog(nivelLogStr);

        // Si quieres log a archivo, léelo de config:
        // std::string rutaLog = config->getString("logFile");
        // Si no usas archivo, deja vacío.
        const std::string rutaLog = "";

        Logger::init(rutaLog, nivelMinimo, true);
        LOG_INFO(LOG_COMP, "Logger inicializado. Nivel: " << nivelLogStr);
    }

    void imprimirAyuda()
    {
        std::cout
            << "\n"
            << "Ejecución sin parámetros: modo detector (tiempo real)\n\n"
            << "Parámetros disponibles:\n\n"
            << "  -procesa [archivoDatos] [directorioFotos] [archivoSalida]\n"
            << "     Genera un [archivoSalida] usando los datos del [archivoDatos].\n"
            << "     [archivoDatos] es un CSV con: idPersona,nombre,foto\n"
            << "     'foto' es el nombre del archivo dentro de [directorioFotos].\n\n"
            << "  -compara-simple [path_cara1] [path_cara2]\n"
            << "     Compara dos caras centradas (misma resolución del modelo).\n\n"
            << "  -compara-busca [path_foto1] [path_foto2]\n"
            << "     Busca una cara en cada foto y luego las compara.\n\n"
            << "  -preprocesa-dataset [path_dirfotos] [path_dircaras] [similaridad]\n"
            << "     Borra fotos sin caras y agrupa las restantes por similaridad,\n"
            << "     creando subdirectorios en [path_dircaras].\n\n"
            << "  --help\n"
            << "     Muestra esta ayuda.\n\n";
    }

    int ejecutarModoPreprocesaDataset(
        const char* directorioFotos,
        const char* directorioSalidaCaras,
        const char* similaridadStr,
        const std::shared_ptr<GHashMap>& config)
    {
        PreProcesadorDataSet preprocesador;
        float similaridad = 0.0f;

        try
        {
            similaridad = std::stof(similaridadStr);
        }
        catch (...)
        {
            LOG_ERROR(LOG_COMP, "Parámetro 'similaridad' no es numérico: " << similaridadStr);
            return -1;
        }

        LOG_INFO(LOG_COMP, "Inicializando dispositivo Hailo (preprocesa-dataset)...");
        hailort::Expected<std::unique_ptr<hailort::VDevice>> dispositivo = hailort::VDevice::create();
        if (!dispositivo)
        {
            LOG_CRITICAL(LOG_COMP, "No se pudo inicializar el dispositivo Hailo.");
            return -1;
        }

        auto* punteroDispositivo = &dispositivo;

        LOG_INFO(LOG_COMP, "Eliminando fotos sin caras. Dir: " << directorioFotos);
        preprocesador.eliminaNoCaras(punteroDispositivo, directorioFotos);

        LOG_INFO(LOG_COMP, "Agrupando caras. Entrada: " << directorioFotos
                           << " | Salida: " << directorioSalidaCaras
                           << " | Similaridad: " << similaridad);
        preprocesador.agrupaCaras(punteroDispositivo, directorioFotos, directorioSalidaCaras, similaridad, config);

        LOG_INFO(LOG_COMP, "Preprocesamiento finalizado.");
        return 0;
    }

    int ejecutarModoComparaSimple(
        const char* pathCara1,
        const char* pathCara2,
        const std::shared_ptr<GHashMap>& config)
    {
        LOG_INFO(LOG_COMP, "Comparación simple. Cara1: " << pathCara1 << " | Cara2: " << pathCara2);

        ComparadorFacial comparador;
        const float comparacion = comparador.comparaSimple(pathCara1, pathCara2, config);

        LOG_INFO(LOG_COMP, "Resultado comparación (simple): " << comparacion);
        std::cout << "Resultado de la comparación: " << comparacion << "\n\n";
        return 0;
    }

    int ejecutarModoComparaBusca(
        const char* pathFoto1,
        const char* pathFoto2,
        const std::shared_ptr<GHashMap>& config)
    {
        LOG_INFO(LOG_COMP, "Comparación con búsqueda. Foto1: " << pathFoto1 << " | Foto2: " << pathFoto2);

        ComparadorFacial comparador;
        const float comparacion = comparador.comparaBusca(pathFoto1, pathFoto2, config);

        LOG_INFO(LOG_COMP, "Resultado comparación (busca): " << comparacion);
        std::cout << "Resultado de la comparación: " << comparacion << "\n\n";
        return 0;
    }

    int ejecutarModoProcesaArchivo(
        const char* archivoDatos,
        const char* directorioFotos,
        const char* archivoSalida,
        const std::shared_ptr<GHashMap>& config)
    {
        LOG_INFO(LOG_COMP, "Modo -procesa: generando identificadores faciales.");
        LOG_INFO(LOG_COMP, "Archivo con datos: " << archivoDatos);
        LOG_INFO(LOG_COMP, "Directorio con fotos: " << directorioFotos);
        LOG_INFO(LOG_COMP, "Archivo con descriptores: " << archivoSalida);

        std::cout << "Se deben generar identificadores faciales\n";
        std::cout << "Archivo con datos: " << archivoDatos << "\n";
        std::cout << "Directorio con fotos: " << directorioFotos << "\n";
        std::cout << "Archivo con descriptores: " << archivoSalida << "\n\n";

        ExtractorFacialArchivo extractor;
        extractor.pathArchivoDatos = archivoDatos;
        extractor.pathFotos = directorioFotos;
        extractor.pathArchivoBD = archivoSalida;

        extractor.alturaRostroMinima = config->getStringLong("anchoMinCaraRec", 90);
        extractor.anchoRostroMinimo  = config->getStringLong("alturaMinCaraRec", 90);
        extractor.toleranciaDetec    = config->getStringDouble("presicionDeteccion", 0.40);
        extractor.toleranciaIden     = config->getStringDouble("deltaRostroMax", 0.60);

        extractor.paramContraste     = config->getStringDouble("paramContraste", 0);
        extractor.pixelSuavizado     = config->getStringLong("pixelSuavizado", 5);
        extractor.alturaImagenBase   = config->getStringLong("alturaImagenBase", 0);
        extractor.jpegSuavizado      = config->getStringLong("jpegSuavizado", 100);
        extractor.resizeSuavizado    = config->getStringDouble("resizeSuavizado", 1);

        extractor.guardarCarasFrontalesAlineadas =
            config->getStringBool("guardarCarasFrontalesAlineadas", false);

        extractor.encuadrarRostros = config->getString("encuadrarRostros");

        extractor.pathModeloDescFacial = config->getString("pathModeloDescFacial");
        extractor.nombreCapaSalidaRedFacial = config->getString("nombreUltimaCapaRedNeuronal");

        extractor.previsualizaImgReconocimiento =
            config->getStringBool("previsualizaImgReconocimiento", false);

        extractor.esperarPrevImgReconocimiento =
            config->getStringBool("esperarPrevImgReconocimiento", false);

        LOG_INFO(LOG_COMP, "Ejecutando extractor facial...");
        extractor.ejecutar();
        LOG_INFO(LOG_COMP, "Extractor facial finalizado.");

        return 0;
    }

    int procesarArgumentosLineaComandos(
        int argc,
        char* argv[],
        const std::shared_ptr<GHashMap>& config)
    {
        for (int indiceArgumento = 1; indiceArgumento < argc; indiceArgumento++)
        {
            const char* arg = argv[indiceArgumento];

            if (std::strcmp(arg, "--help") == 0)
            {
                imprimirAyuda();
                return 0;
            }

            if (std::strcmp(arg, "-preprocesa-dataset") == 0)
            {
                if ((indiceArgumento + 3) >= argc)
                {
                    LOG_ERROR(LOG_COMP, "Faltan parámetros para -preprocesa-dataset.");
                    imprimirAyuda();
                    return -1;
                }

                return ejecutarModoPreprocesaDataset(
                    argv[indiceArgumento + 1],
                    argv[indiceArgumento + 2],
                    argv[indiceArgumento + 3],
                    config);
            }

            if (std::strcmp(arg, "-compara-simple") == 0)
            {
                if ((indiceArgumento + 2) >= argc)
                {
                    LOG_ERROR(LOG_COMP, "Faltan parámetros para -compara-simple.");
                    imprimirAyuda();
                    return -1;
                }

                return ejecutarModoComparaSimple(
                    argv[indiceArgumento + 1],
                    argv[indiceArgumento + 2],
                    config);
            }

            if (std::strcmp(arg, "-compara-busca") == 0)
            {
                if ((indiceArgumento + 2) >= argc)
                {
                    LOG_ERROR(LOG_COMP, "Faltan parámetros para -compara-busca.");
                    imprimirAyuda();
                    return -1;
                }

                return ejecutarModoComparaBusca(
                    argv[indiceArgumento + 1],
                    argv[indiceArgumento + 2],
                    config);
            }

            if (std::strcmp(arg, "-procesa") == 0)
            {
                if ((indiceArgumento + 3) >= argc)
                {
                    LOG_ERROR(LOG_COMP, "Faltan parámetros para -procesa.");
                    imprimirAyuda();
                    return -1;
                }

                return ejecutarModoProcesaArchivo(
                    argv[indiceArgumento + 1],
                    argv[indiceArgumento + 2],
                    argv[indiceArgumento + 3],
                    config);
            }
        }

        return 1;
    }

    void cargarParametrosDeProcesoDesdeConfig(ProcesoRecFacial& proceso, const std::shared_ptr<GHashMap>& config)
    {
        proceso.idEquipo = SystemUtils::getRaspberryPiSerial();
        LOG_INFO(LOG_COMP, "Serie del equipo: " << proceso.idEquipo);

        proceso.alturaRostroMinima = config->getStringLong("anchoMinCaraRec", 90);
        proceso.anchoRostroMinimo  = config->getStringLong("alturaMinCaraRec", 90);

        proceso.anchoRostroMinVis  = config->getStringLong("anchoMinCaraVis", 70);
        proceso.alturaRostroMinVis = config->getStringLong("alturaMinCaraVis", 70);

        proceso.anchoCamara  = config->getStringLong("anchoImgFuente", 1920);
        proceso.alturaCamara = config->getStringLong("alturaImgFuente", 1080);

        proceso.anchoRostroDet  = config->getStringLong("anchoCaraRec", 112);
        proceso.alturaRostroDet = config->getStringLong("alturaCaraRec", 112);

        proceso.anchoVisualiza  = config->getStringLong("anchoImgVisualizacion", 1280);
        proceso.alturaVisualiza = config->getStringLong("alturaImgVisualizacion", 720);

        proceso.compresionJpeg = config->getStringLong("compresionJpeg", 70);

        proceso.toleranciaDetec = config->getStringDouble("presicionDeteccion", 0.40);
        proceso.toleranciaIden  = config->getStringDouble("deltaRostroMax", 0.60);

        proceso.regionInteresInicioX = config->getStringDouble("regionInteresInicioX", 0);
        proceso.regionInteresFinX    = config->getStringDouble("regionInteresFinX", proceso.anchoCamara);

        proceso.usarDistEuclideana = config->getStringBool("usarDistEcuclideana", true);

        proceso.universoPersonas.setNumThreadsIdentificacion(
            config->getStringLong("numThreadsIdentificacion", 1));

        proceso.tiempoReEvento =
            config->getStringLong("tiempoReEvento", 30) * 1000;

        proceso.tiempoMaxNoReconocido =
            config->getStringLong("tiempoMaxNoReconocido", 30);

        proceso.reportarDesconocidos =
            config->getStringBool("reportarDesconocidos", false);

        proceso.pathModeloDescFacial =
            config->getString("pathModeloDescFacial");

        proceso.nombreCapaSalidaRedFacial =
            config->getString("nombreUltimaCapaRedNeuronal");

        proceso.paramContraste =
            config->getStringDouble("paramContraste", 2.0);

        proceso.encuadrarRostros =
            config->getString("encuadrarRostros");

        const std::string reflejarVerticalmente = config->getString("reflejarVerticalmente");
        proceso.invertirVertical = (reflejarVerticalmente.compare("S") == 0);

        LOG_DEBUG(LOG_COMP, "Parámetros de proceso cargados desde config.");
    }

    std::shared_ptr<ImageSourceFactory> crearFuenteImagenesDesdeConfig(const std::shared_ptr<GHashMap>& config)
    {
        const std::string fuenteImagenes = config->getString("source");
        LOG_INFO(LOG_COMP, "Fuente de imágenes: " << fuenteImagenes);

        std::shared_ptr<ImageSourceFactory> fabrica;

        if (fuenteImagenes.compare("onvif") == 0)
        {
            auto fabricaOnvif = std::make_shared<OnvifCameraFactory>();
            fabrica = std::dynamic_pointer_cast<ImageSourceFactory>(fabricaOnvif);

            fabrica->lstParams.putString(OnvifCamera::PARAM_IP_SERVIDOR, config->getString(OnvifCamera::PARAM_IP_SERVIDOR));
            fabrica->lstParams.putString(OnvifCamera::PARAM_PUERTO_SERVIDOR, config->getString(OnvifCamera::PARAM_PUERTO_SERVIDOR));
            fabrica->lstParams.putString(OnvifCamera::PARAM_USUARIO_SERVIDOR, config->getString(OnvifCamera::PARAM_USUARIO_SERVIDOR));
            fabrica->lstParams.putString(OnvifCamera::PARAM_PASSWORD_SERVIDOR, config->getString(OnvifCamera::PARAM_PASSWORD_SERVIDOR));
            fabrica->lstParams.putString(OnvifCamera::PARAM_URL_SERVIDOR, config->getString(OnvifCamera::PARAM_URL_SERVIDOR));

            LOG_INFO(LOG_COMP, "Fuente ONVIF configurada.");
        }
        else if (fuenteImagenes.compare("video") == 0)
        {
            auto fabricaVideo = std::make_shared<VideoCameraFactory>();
            fabrica = std::dynamic_pointer_cast<ImageSourceFactory>(fabricaVideo);

            fabrica->lstParams.putString(VideoCamera::PARAM_PATH_VIDEO, config->getString(VideoCamera::PARAM_PATH_VIDEO));
            fabrica->lstParams.putString(VideoCamera::PARAM_VELOCIDAD_VIDEO, config->getString(VideoCamera::PARAM_VELOCIDAD_VIDEO));
            fabrica->lstParams.putString(VideoCamera::PARAM_CUADRO_INICIAL, config->getString(VideoCamera::PARAM_CUADRO_INICIAL));
            fabrica->lstParams.putString(VideoCamera::PARAM_INFINITO, config->getString(VideoCamera::PARAM_INFINITO));

            LOG_INFO(LOG_COMP, "Fuente VIDEO configurada.");
        }
        else
        {
            auto fabricaInterna = std::make_shared<InterImgSourceFactory>();
            fabrica = std::dynamic_pointer_cast<ImageSourceFactory>(fabricaInterna);

            LOG_INFO(LOG_COMP, "Fuente interna configurada.");
        }

        return fabrica;
    }

    void cargarUniversoPersonasYEndpoints(ProcesoRecFacial& proceso, const std::shared_ptr<GHashMap>& config)
    {
        const std::string unificar = config->getString("unificarDescriptores");
        const bool unificarDescriptores = (unificar.compare("S") == 0);

        LOG_INFO(LOG_COMP, "Cargando universo de personas. Unificar: " << (unificarDescriptores ? "S" : "N"));

        proceso.universoPersonas.cargarpPerConocidas(
            config->getString("pathBDPersonas"),
            unificarDescriptores);

        proceso.generadorEventos.urlServidorIden =
            config->getString("urlBaseServidorIden");

        proceso.generadorEventos.urlServidorNoIden =
            config->getString("urlBaseServidorNoIden");

        proceso.generadorEventos.urlServidorUnificado =
            config->getString("urlServidorUnificado");

        proceso.generadorEventos.pathLogEventos =
            config->getString("logEventos");

        proceso.generadorEventos.generarLogEventos =
            config->getStringBool("generarLogEventos", false);

        proceso.generadorEventos.usarEndpointUnificado =
            config->getStringBool("usarEndPointUnificado", false);

        LOG_DEBUG(LOG_COMP, "Endpoints y configuración de eventos cargados.");
    }
}

/**
 * Punto de inicio de ejecución de la aplicación
 */
int main(int argc, char* argv[])
{
    // 1) Cargar configuración desde el singleton LectorConfig
    LectorConfig::getInstance().initFromFile("./config/config.txt");
    const std::shared_ptr<GHashMap> config = LectorConfig::getInstance().getParams();

    // 2) Configurar logger usando config
    configurarLoggerDesdeConfig(config);

    // 3) Si hay modo CLI (procesa / compara / dataset), ejecútalo y termina
    const int resultadoCli = procesarArgumentosLineaComandos(argc, argv, config);
    if (resultadoCli <= 0)
    {
        LOG_INFO(LOG_COMP, "Ejecución finalizada en modo CLI.");
        return resultadoCli;
    }

    // 4) Modo detector: iniciar el proceso principal
    LOG_INFO(LOG_COMP, "Iniciando modo detector (tiempo real).");

    ProcesoRecFacial proceso;

    proceso.lstParamsApp = config;
    proceso.leeParametros("./config/config.txt");

    cargarParametrosDeProcesoDesdeConfig(proceso, config);

    const std::shared_ptr<ImageSourceFactory> fuenteImagenes = crearFuenteImagenesDesdeConfig(config);
    proceso.setImageFactory(fuenteImagenes);

    cargarUniversoPersonasYEndpoints(proceso, config);

    LOG_INFO(LOG_COMP, "Iniciando proceso de reconocimiento facial...");
    proceso.iniciar();

    LOG_INFO(LOG_COMP, "Proceso finalizado.");
    return 0;
}
