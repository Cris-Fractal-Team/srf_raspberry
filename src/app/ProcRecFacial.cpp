#include "app/ProcRecFacial.h"
#include "app/GeneradorPingsMonitoreo.h"
#include "app/GeneradorPingsAppWeb.h"
#include "app/LectorConfig.h"

#include <stdint.h>
#include <stdio.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>

#include <opencv2/opencv.hpp>

#include "lib/graphics/GDibujo.h"
#include "lib/graphics/ImageSource.h"
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/Logger.h"
#include "lib/utils/base64.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/systemUtils.h"
#include "lib/utils/timedate.h"
#include "lib/web/GHttpClient.h"

static constexpr const char* LOG_COMPONENT = "ProcRecFacial";

/**
 * Lista con las ultimas caras detectadas
 */
vector<TrackedDetectionHailo*> ProcesoRecFacial::lstUltCarasDet;

/**
 * Constructor
 */
ProcesoRecFacial::ProcesoRecFacial() {
    invertirVertical = false;
    usarDistEuclideana = true;
    tiempoMaxNoReconocido = 10;

    numIndentificacionesMin = 3;
    generadorPingsMonitoreo = new GeneradorPingsMonitoreo();
    generadorPingsMonitoreo->procRecFacial = this;
}

/**
 * Establece el creador de imagenes
 */
void ProcesoRecFacial::setImageFactory(shared_ptr<ImageSourceFactory> factory) {
    imageSource = factory->getInstance();
}

void ProcesoRecFacial::setFlagProcesarImagenes(bool nuevoValor) {
    std::lock_guard<std::mutex> lock(mtx);
    flagProcesarImagenes = nuevoValor;
}

bool ProcesoRecFacial::getFlagProcesarImagenes() {
    std::lock_guard<std::mutex> lock(mtx);
    return flagProcesarImagenes;
}

void ProcesoRecFacial::setIdTareaProgramada(int nuevoIdTareaProgramada) {
    std::lock_guard<std::mutex> lock(mtx);
    idTareaProgramada = nuevoIdTareaProgramada;
}

int ProcesoRecFacial::getIdTareaProgramada() {
    std::lock_guard<std::mutex> lock(mtx);
    return idTareaProgramada;
}

bool ProcesoRecFacial::getUsandoNPU() {
    std::lock_guard<std::mutex> lock(mtx);
    return usandoNpu;
}

void ProcesoRecFacial::setUsandoNPU(bool nuevoValor) {
    std::lock_guard<std::mutex> lock(mtx);
    usandoNpu = nuevoValor;
}

/**
 * Bucle que realiza todo el flujo
 */
void ProcesoRecFacial::iniciar() {
    GImage frameCapturado, frameParaDeteccion;
    GImage frameVisor, frameVisorPrevio;

    cv::Mat matrizInvertida;

    vector<DeteccionCaraHailo> deteccionesBrutas;
    vector<DeteccionCaraHailo> deteccionesFiltradas;

    vector<TrackedDetectionHailo*> carasTrackTemporales;
    vector<TrackedDetectionHailo*> carasTrackFinal;

    hailort::Expected<std::unique_ptr<hailort::VDevice>>* vdevicePtr = nullptr;

    float factorEscalaModeloX = 0.0f;
    float factorEscalaModeloY = 0.0f;

    DetectorCarasHailoSCRFD detectorRostros;
    FaceRecHailo generadorDescriptores;

    int modoCargaEnDuro = 1;

    errorInicial = false;
    numIndentificacionesMin = lstParamsApp->getStringLong("numIndentificacionesMin", 3);

    // Crea el dispositivo virtual
    LOG_INFO(LOG_COMPONENT, "Creando VDevice");
    hailort::Expected<std::unique_ptr<hailort::VDevice>> vdevice = hailort::VDevice::create();
    if (!vdevice) {
        LOG_ERROR(LOG_COMPONENT, "Error: No se pudo inicializar el dispositivo Hailo.");
        errorInicial = true;
        return;
    }
    vdevicePtr = &vdevice;

    procDescargaDescFaciales.extractor.vdevice = &vdevice;

    // crea el detector
    detectorRostros.setDimImagenes(640, 640);
    detectorRostros.previsualizaImgParaDeteccion =
        lstParamsApp->getStringBool("previsualizaImgParaDeteccion", false);
    detectorRostros.esperarPrevImgParaDeteccion =
        lstParamsApp->getStringBool("esperarPrevImgParaDeteccion", false);

    generadorPingsMonitoreo->serieEquipo = GStringUtils::trim(serieEquipo);
    procDescargaDescFaciales.serieEquipo = GStringUtils::trim(serieEquipo);

    detectorRostros.runner.device = &vdevice;
    if (detectorRostros.cargarModelo("./models/scrfd_2.5ga.hef") != 0) {
        LOG_ERROR(LOG_COMPONENT, "Error al cargar el modelo detector");
        errorInicial = true;
        return;
    }
    LOG_INFO(LOG_COMPONENT, "Detector de rostros iniciado");

    // crea el generador de descriptores faciales
    generadorDescriptores.runner.device = &vdevice;
    generadorDescriptores.paramContraste = paramContraste;
    generadorDescriptores.nombreUltimaCapaModelo = nombreCapaSalidaRedFacial;

    generadorDescriptores.previsualizaImgReconocimiento =
        lstParamsApp->getStringBool("previsualizaImgReconocimiento", false);
    generadorDescriptores.esperarPrevImgReconocimiento =
        lstParamsApp->getStringBool("esperarPrevImgReconocimiento", false);
    generadorDescriptores.guardarPrevImgReconocimiento =
        lstParamsApp->getStringBool("guardarPrevImgReconocimiento", false);
    generadorDescriptores.guardarCarasFrontalesAlineadas =
        lstParamsApp->getStringBool("guardarCarasFrontalesAlineadas", false);

    generadorDescriptores.prefijoImgReconocimiento =
        lstParamsApp->getString("prefijoImgReconocimiento");
    generadorDescriptores.prefijoImgReconocimiento +=
        "_" + to_string(TimeDateUtils::getDateTimeMs()) + "_";

    generadorDescriptores.similaridadFrontal =
        lstParamsApp->getStringDouble("similaridadFrontal", 5.0);

    if (generadorDescriptores.cargarModelo(pathModeloDescFacial) != 0) {
        LOG_ERROR(LOG_COMPONENT, "Error al cargar modelo generador de descriptores");
        errorInicial = true;
        return;
    }
    LOG_INFO(LOG_COMPONENT, "Generador de descriptores iniciado");

    // Inicia el servidor web
    servidorWeb = make_shared<ServidorHttpImagenes>();
    servidorWeb->lstParamsApp = this->lstParamsApp;
    servidorWeb->procRecFacial = this;
    servidorWeb->iniciar();

    // Factor de escala para dimensiones de visualizacion
    factorEscalaVisualizaX = ((double)anchoVisualiza) / ((double)anchoCamara);
    factorEscalaVisualizaY = ((double)alturaVisualiza) / ((double)alturaCamara);

    // ejecuta el procesador de eventos
    generadorEventos.start();
    generadorPingsMonitoreo->start();
    generadorPingsAppWeb.start();

    factorEscalaModeloX = (float)anchoCamara / 640.0f;
    factorEscalaModeloY = (float)alturaCamara / 640.0f;

    // bucle principal
    finalizar = false;
    idDesconocidoSgte = 0;

    while (!finalizar) {
        const auto tInicioCiclo = std::chrono::high_resolution_clock::now();

        if (isSuspendido())
        {
            setUsandoNPU(false);

            std::vector<std::string> msg = {
                "=== MODO SUSPENDIDO ===",
                "Solo AppWebPings activo.",
                "Presiona 'y' para volver a ENCENDIDO",
                "Presiona 'q' para salir"
            };

            GImage frameAviso = construirFrameAviso(anchoVisualiza, alturaVisualiza, msg);

            servidorWeb->setImage(frameAviso);
            GDibujo::show(frameAviso, "Visor");

            const int tecla = GDibujo::waitForKey(50);
            if (tecla == 'q')
            {
                break;
            }
            if (tecla == 'y')
            {
                setEstadoEncendido();
            }

            usleep(200000);
            continue;
        }

        if (isApagado())
        {
            setUsandoNPU(false);

            std::vector<std::string> msg = {
                "=== MODO APAGADO ===",
                "Monitoreo + AppWebPings activos.",
                "Eventos apagados.",
                "Presiona 'q' para salir"
            };

            GImage frameAviso = construirFrameAviso(anchoVisualiza, alturaVisualiza, msg);

            servidorWeb->setImage(frameAviso);
            GDibujo::show(frameAviso, "Visor");

            const int tecla = GDibujo::waitForKey(50);
            if (tecla == 'q')
            {
                break;
            }

            usleep(200000);
            continue;
        }
        // validar si se debe o no procesar las imagenes
        if (getFlagProcesarImagenes()) {
            if (!getUsandoNPU()) {
                LOG_INFO(LOG_COMPONENT, "Procesamiento solicitado: descarga + generación + recarga de universo");

                procDescargaDescFaciales.idTareaProgramada = getIdTareaProgramada();
                procDescargaDescFaciales.ejecutarDescargaYGeneracion();

                const std::string parametroUnificarDescriptores = lstParamsApp->getString("unificarDescriptores");
                const bool unificarDescriptores = (parametroUnificarDescriptores == "S");

                universoPersonas.cargarpPerConocidas(
                    lstParamsApp->getString("pathBDPersonas"),
                    unificarDescriptores);

                setFlagProcesarImagenes(false);
                LOG_INFO(LOG_COMPONENT, "Procesamiento solicitado finalizado");
            }
            usleep(20000);
            continue;
        }

        frameCapturado = imageSource->getImage();
        if ((frameCapturado.ancho == 0) || (frameCapturado.altura == 0)) {
            continue;
        }

        // marcar el flag usandoNPU = true
        setUsandoNPU(true);

        // invierte verticalmente
        if (invertirVertical) {
            cv::flip(frameCapturado.imagenOpencv, matrizInvertida, -1);
            frameCapturado.imagenOpencv = matrizInvertida;
        }

        if ((regionInteresInicioX > 0) || (regionInteresFinX < anchoCamara)) {
            frameParaDeteccion = frameCapturado.getRect(
                regionInteresInicioX, 0, regionInteresFinX, alturaCamara);
        } else {
            frameParaDeteccion = frameCapturado;
        }

        const auto tFinCaptura = std::chrono::high_resolution_clock::now();

        // detecta rostros
        deteccionesBrutas = detectorRostros.detectar_2_5g(frameParaDeteccion, toleranciaDetec);
        if (detectorRostros.errorDeteccion) {
            LOG_ERROR(LOG_COMPONENT, "Error al detectar rostros");
            break;
        }

        // elimina rostros pequeños
        deteccionesFiltradas.clear();
        for (DeteccionCaraHailo deteccionCara : deteccionesBrutas) {
            const bool superaMinimo =
                (deteccionCara.getAncho() > anchoRostroMinimo) &&
                (deteccionCara.getAltura() > alturaRostroMinima);

            const bool noEsPerfil = (deteccionCara.caraDePerfil() == false);

            if (superaMinimo && noEsPerfil) {
                deteccionCara.desplazaPtosCara(regionInteresInicioX, 0);
                deteccionCara.desplazaRegion(regionInteresInicioX, 0);

                if (encuadrarRostros == "S")
                    deteccionCara.ajustarCuadrado(anchoCamara, alturaCamara);
                else if (encuadrarRostros == "M")
                    deteccionCara.ajustarMiniCuadrado(anchoCamara, alturaCamara);

                deteccionesFiltradas.push_back(deteccionCara);
            }
        }

        // Analiza tracking temporal
        carasTrackTemporales = detTracker.generaListaTrackTemporal(&deteccionesFiltradas);
        const auto tFinDeteccion = std::chrono::high_resolution_clock::now();

        // genera descriptores faciales
        calculaDescriptores(&generadorDescriptores, frameCapturado, &carasTrackTemporales);
        const auto tFinDescriptores = std::chrono::high_resolution_clock::now();

        // identifica personas
        identificarPersonas(&carasTrackTemporales);
        const auto tFinIdentificacion = std::chrono::high_resolution_clock::now();

        // tracking final (identificación + tracker)
        frameVisor.ancho = detectorRostros.imgModeloAncho;
        frameVisor.altura = detectorRostros.imgModeloAltura;
        frameVisor.imagenOpencv = detectorRostros.imagenRedim;

        carasTrackFinal = detTracker.analizaPorIdentificacionYTracker(
            &carasTrackTemporales,
            &frameVisor,
            &frameVisorPrevio,
            factorEscalaModeloX,
            factorEscalaModeloY);

        const auto tFinTracking = std::chrono::high_resolution_clock::now();

        // Notifica al servidor web las incidencias o detecciones
        notificaDetecciones(frameCapturado, &carasTrackFinal);
        const auto tFinNotifica = std::chrono::high_resolution_clock::now();

        frameVisorPrevio = frameVisor.clone();

        frameVisor = dibujaCaras(frameCapturado, &carasTrackFinal);
        const auto tFinDibuja = std::chrono::high_resolution_clock::now();

        universoPersonas.eliminaDesAntiguos(tiempoMaxNoReconocido);

        // calcula los tiempos transcurridos (sin imprimir)
        const auto durCaptura = std::chrono::duration_cast<std::chrono::microseconds>(tFinCaptura - tInicioCiclo);
        const auto durDetecta = std::chrono::duration_cast<std::chrono::microseconds>(tFinDeteccion - tFinCaptura);
        const auto durDescriptor = std::chrono::duration_cast<std::chrono::microseconds>(tFinDescriptores - tFinDeteccion);
        const auto durIdentificacion = std::chrono::duration_cast<std::chrono::microseconds>(tFinIdentificacion - tFinDescriptores);
        const auto durTracking = std::chrono::duration_cast<std::chrono::microseconds>(tFinTracking - tFinIdentificacion);
        const auto durNotifica = std::chrono::duration_cast<std::chrono::microseconds>(tFinNotifica - tFinTracking);
        const auto durDibuja = std::chrono::duration_cast<std::chrono::microseconds>(tFinDibuja - tFinNotifica);
        const auto durTotal = std::chrono::duration_cast<std::chrono::microseconds>(tFinDibuja - tInicioCiclo);

        (void)durCaptura;
        (void)durDetecta;
        (void)durDescriptor;
        (void)durIdentificacion;
        (void)durTracking;
        (void)durNotifica;
        (void)durDibuja;
        (void)durTotal;

        // Envia la imagen al servidor web de configuracion
        ProcesoRecFacial::lstUltCarasDet = carasTrackFinal;
        servidorWeb->setImage(frameVisor);

        // Envia la imagen a la pantalla local
        GDibujo::show(frameVisor, "Visor");

        // configura el procesar de eventos del mouse
        cv::setMouseCallback("Visor", onMouse, this);

        // Lee el teclado para finalizar si presiona 'q'
        const int tecla = GDibujo::waitForKey(10);
        if (tecla == 'q')
        {
            LOG_INFO(LOG_COMPONENT, "Salida solicitada por teclado (q)");
            break;
        }

        // marcar el flag usandoNPU = false
        setUsandoNPU(false);
    }

    LOG_INFO(LOG_COMPONENT, "Finaliza bucle de reconocimiento");
    GDibujo::closeAllWindows();
    imageSource->stop();

    generadorPingsAppWeb.finalizar();
    generadorPingsMonitoreo->finalizar();
    generadorEventos.finalizar();

    universoPersonas.lstPerIdentificadas.reset();
    universoPersonas.lstPerNoIdent.reset();

    detTracker.reset();
    detTracker.lstUniverso.reset();

    exit(0);
}

/**
 * Calcula los descriptores faciales y califica los objetos
 */
void ProcesoRecFacial::calculaDescriptores(
    FaceRecHailo* generador, GImage foto,
    vector<TrackedDetectionHailo*>* lstRostros) {

    GLinkedList<TrackedDetectionHailo*> listaCarasDeteccion;
    GLinkedList<GImage> listaCaras;                 // (no usado actualmente)
    GLinkedList<DeteccionCaraHailo> listaDetecciones; // (no usado actualmente)
    GLinkedList<SIMD_TYPE*> listaDescriptores;      // (no usado actualmente)

    TrackedDetectionHailo* trackFace = nullptr;
    const int totalRostros = (int)lstRostros->size();

    (void)listaCaras;
    (void)listaDetecciones;
    (void)listaDescriptores;

    for (int indiceRostro = 0; indiceRostro < totalRostros; indiceRostro++) {
        trackFace = lstRostros->at(indiceRostro);

        // calcula la deteccion relativa al rectangulo en el que se detecto
        trackFace->cara.detRelCara = trackFace->cara.deteccion;
        trackFace->cara.detRelCara.desplazaPtosCara(
            -trackFace->cara.deteccion.ptoSupIzq.x,
            -trackFace->cara.deteccion.ptoSupIzq.y);

        trackFace->cara.detRelCara.ptoSupIzq.x = 0;
        trackFace->cara.detRelCara.ptoSupIzq.y = 0;

        trackFace->cara.detRelCara.ptoInfDer.x -= trackFace->cara.deteccion.ptoSupIzq.x;
        trackFace->cara.detRelCara.ptoInfDer.y -= trackFace->cara.deteccion.ptoSupIzq.y;

        trackFace->cara.calculaFotoCara(foto);
        listaCarasDeteccion.add(trackFace);
    }

    if (totalRostros > 0) {
        generador->calculaDescriptor(&listaCarasDeteccion);
    }

    listaCarasDeteccion.reset();
}

/**
 * Analiza todas las caras de la lista de rostros detectados
 * para ver si alguno coincide con una persona conocida
 */
void ProcesoRecFacial::identificarPersonas(
    vector<TrackedDetectionHailo*>* lstRostros) {

    TrackedDetectionHailo* trackFace = nullptr;
    DescPersonaExterno* personaCoincidente = nullptr;

    IdentificacionPersona ultimaIdentificacionValida;

    long long timestampDeteccionMs = TimeDateUtils::getDateTimeMs();
    float distanciaEuclidiana = 0.0f;

    const int ultimoIndice = (int)lstRostros->size() - 1;

    for (int indice = ultimoIndice; indice >= 0; indice--) {
        trackFace = lstRostros->at(indice);

        if (trackFace->ciclosNoDetectados > 0) {
            continue;
        }

        // solo se identifica a la persona si es nueva (no viene de tracker)
        if (trackFace->tracker.empty()) {
            personaCoincidente = universoPersonas.buscaPersonaCon(
                trackFace->cara.descriptor,
                toleranciaIden,
                &distanciaEuclidiana,
                usarDistEuclideana);
        } else {
            continue;
        }

        if (personaCoincidente != NULL) {
            IdentificacionPersona identificacion;

            identificacion.fecDet = timestampDeteccionMs;
            std::memcpy(
                identificacion.vecDescripcion,
                trackFace->cara.descriptor,
                NUM_ELEMS_DESC_FACIAL * sizeof(SIMD_TYPE));

            identificacion.comparacion = distanciaEuclidiana;

            trackFace->cara.personaIdent = true;
            personaCoincidente->anonimo = false;

            trackFace->cara.identificador.agregaIdentif(
                personaCoincidente,
                identificacion,
                timestampDeteccionMs);

            ultimaIdentificacionValida =
                trackFace->cara.identificador.getUltimaIdentificacion();

        } else if (reportarDesconocidos) {

            personaCoincidente = universoPersonas.buscaPersonaDesc(
                trackFace->cara.descriptor,
                toleranciaIden,
                &distanciaEuclidiana,
                usarDistEuclideana);

            if (personaCoincidente != NULL) {
                IdentificacionPersona identificacion;

                identificacion.fecDet = timestampDeteccionMs;
                std::memcpy(
                    identificacion.vecDescripcion,
                    trackFace->cara.descriptor,
                    NUM_ELEMS_DESC_FACIAL * sizeof(SIMD_TYPE));

                identificacion.comparacion = distanciaEuclidiana;

                trackFace->cara.personaIdent = true;

                trackFace->cara.identificador.agregaIdentif(
                    personaCoincidente,
                    identificacion,
                    timestampDeteccionMs);

                personaCoincidente->ultimaFechaDetectada = timestampDeteccionMs;

                ultimaIdentificacionValida =
                    trackFace->cara.identificador.getUltimaIdentificacion();

            } else {
                IdentificacionPersona identificacion;

                identificacion.fecDet = timestampDeteccionMs;
                std::memcpy(
                    identificacion.vecDescripcion,
                    trackFace->cara.descriptor,
                    NUM_ELEMS_DESC_FACIAL * sizeof(SIMD_TYPE));

                identificacion.comparacion = 100;

                trackFace->cara.personaIdent = true;

                DescPersonaExterno personaExterna;

                personaExterna.anonimo = true;
                std::memcpy(
                    personaExterna.vecDescripcion,
                    identificacion.vecDescripcion,
                    NUM_ELEMS_DESC_FACIAL * sizeof(SIMD_TYPE));

                personaExterna.id = std::to_string(idDesconocidoSgte);
                personaExterna.nombre = "Desc." + personaExterna.id;
                personaExterna.ultimaFechaDetectada = timestampDeteccionMs;

                idDesconocidoSgte++;
                universoPersonas.lstPerNoIdent.add(personaExterna);

                trackFace->cara.identificador.agregaIdentif(
                    universoPersonas.lstPerNoIdent.getAddrUltimo(),
                    identificacion,
                    timestampDeteccionMs);

                ultimaIdentificacionValida =
                    trackFace->cara.identificador.getUltimaIdentificacion();
            }

        } else {
            trackFace->cara.personaIdent = false;
        }
    }

    (void)ultimaIdentificacionValida;
}

/**
 * Dibuja las caras encontradas
 * Retorna la imagen para el visor
 */
GImage ProcesoRecFacial::dibujaCaras(GImage imagen,
                                     vector<TrackedDetectionHailo*>* lstCaras) {
    GImage frameVisor;

    CaraDescrita* cara = nullptr;
    TrackedDetectionHailo* trackFace = nullptr;

    GPoint punto;
    GRect cajaRostro;
    GRect cajaAux;

    GColor colorAmarillo(0, 255, 255);
    GColor colorRojo(255, 0, 0);
    GColor colorVerde(0, 255, 0);
    GColor colorAzul(0, 0, 255);
    GColor colorBlanco(255, 255, 255);

    DescPersonaExterno* datosPersona = nullptr;

    string etiquetaId;

    int yTextoCaja = 0;
    const int radioPuntos = 2;

    const float escalaX = ((float)anchoVisualiza) / ((float)anchoCamara);
    const float escalaY = ((float)alturaVisualiza) / ((float)alturaCamara);

    const float escalaXTrack = ((float)anchoVisualiza) / ((float)NUM_ELEMS_DESC_FACIAL);
    const float escalaYTrack = ((float)alturaVisualiza) / ((float)NUM_ELEMS_DESC_FACIAL);

    frameVisor = imagen.cloneResize(anchoVisualiza, alturaVisualiza);

    cajaAux.x1 = (int)((float)regionInteresInicioX) * escalaX;
    cajaAux.x2 = (int)((float)regionInteresFinX) * escalaX;
    cajaAux.y1 = 0;
    cajaAux.y2 = alturaVisualiza;
    GDibujo::drawRect(frameVisor, cajaAux, colorAzul, 1);

    const int totalCaras = (int)lstCaras->size();
    for (int indiceCara = 0; indiceCara < totalCaras; indiceCara++) {
        trackFace = lstCaras->at(indiceCara);

        etiquetaId = (trackFace->id > 0) ? to_string(trackFace->id) : "";

        cara = &trackFace->cara;

        cajaRostro.x1 = cara->deteccion.ptoSupIzq.x * escalaX;
        cajaRostro.y1 = cara->deteccion.ptoSupIzq.y * escalaY;
        cajaRostro.x2 = cara->deteccion.ptoInfDer.x * escalaX;
        cajaRostro.y2 = cara->deteccion.ptoInfDer.y * escalaY;

        datosPersona = trackFace->cara.identificador.getDatosPerIden();

        if (!trackFace->tracker.empty()) {
            cajaAux.x1 = (int)(((float)trackFace->trackerBox.x) * escalaXTrack);
            cajaAux.y1 = (int)(((float)trackFace->trackerBox.y) * escalaYTrack);
            cajaAux.x2 = cajaAux.x1 + (int)(((float)trackFace->trackerBox.width) * escalaXTrack);
            cajaAux.y2 = cajaAux.y1 + (int)(((float)trackFace->trackerBox.height) * escalaYTrack);

            GDibujo::drawRect(frameVisor, cajaAux, colorAmarillo, 1);
        }

        if (!cara->descCalculado) {
            GDibujo::drawRect(frameVisor, cajaRostro, colorAmarillo, 2);
        } else if ((!cara->personaIdent) || (datosPersona == NULL)) {
            GDibujo::drawRect(frameVisor, cajaRostro, colorRojo, 2);
        } else {
            if (datosPersona != NULL) {
                etiquetaId.append(" - ");
                etiquetaId.append(GStringUtils::to_string_fixed(
                    trackFace->cara.identificador.getPromedioComparacion(), 2));
                etiquetaId.append(" : ");
                etiquetaId.append(datosPersona->nombre);
            }

            if (datosPersona->anonimo)
                GDibujo::drawRect(frameVisor, cajaRostro, colorRojo, 2);
            else
                GDibujo::drawRect(frameVisor, cajaRostro, colorVerde, 2);
        }

        if (trackFace->tracker.empty()) {
            punto.x = (int)(((float)cara->deteccion.ojoIzq.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.ojoIzq.y) * escalaY);
            GDibujo::drawElipse(frameVisor, punto, radioPuntos, radioPuntos, colorRojo, 1);

            punto.x = (int)(((float)cara->deteccion.ojoDer.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.ojoDer.y) * escalaY);
            GDibujo::drawElipse(frameVisor, punto, radioPuntos, radioPuntos, colorRojo, 1);

            punto.x = (int)(((float)cara->deteccion.nariz.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.nariz.y) * escalaY);
            GDibujo::drawElipse(frameVisor, punto, radioPuntos, radioPuntos, colorRojo, 1);

            punto.x = (int)(((float)cara->deteccion.bocaIzq.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.bocaIzq.y) * escalaY);
            GDibujo::drawElipse(frameVisor, punto, radioPuntos, radioPuntos, colorRojo, 1);

            punto.x = (int)(((float)cara->deteccion.bocaDer.x) * escalaX);
            punto.y = (int)(((float)cara->deteccion.bocaDer.y) * escalaY);
            GDibujo::drawElipse(frameVisor, punto, radioPuntos, radioPuntos, colorRojo, 1);
        }

        // dibuja la caja con el ID / texto
        yTextoCaja = cajaRostro.y1 - 30;
        if (yTextoCaja < 0) {
            // se dibuja la caja en la parte inferior
            cajaRostro.y1 = cajaRostro.y2;
            cajaRostro.y2 += 40;
        } else {
            cajaRostro.y2 = cajaRostro.y1;
            cajaRostro.y1 -= 40;
        }

        cajaRostro.x2 += 100;
        cajaRostro.x1 -= 200;
        if (cajaRostro.x1 < 0) cajaRostro.x1 = 0;

        if (datosPersona != NULL) {
            if (trackFace->cara.identificador.fechaUltEvento == 0) {
                GDibujo::drawRect(frameVisor, cajaRostro, colorBlanco, -1);
                GDibujo::drawText(frameVisor, cajaRostro.x1, cajaRostro.y1 + 30,
                                  etiquetaId, GDIBUJO_FONT_HELVETICA, colorAzul, 0.8, 2);
            } else {
                GDibujo::drawRect(frameVisor, cajaRostro, colorAzul, -1);
                GDibujo::drawText(frameVisor, cajaRostro.x1, cajaRostro.y1 + 30,
                                  etiquetaId, GDIBUJO_FONT_HELVETICA, colorBlanco, 0.8, 2);
            }
        }
    }

    return frameVisor;
}

/**
 * Procesa el evento click sobre la pantalla
 */
void ProcesoRecFacial::onMouse(int event, int x, int y, int flags,
                               void* userdata) {
    const float escalaX = ((float)1920) / ((float)1280);
    const float escalaY = ((float)1080) / ((float)720);

    float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
    TrackedDetectionHailo* trackFace = nullptr;

    if (event != cv::EVENT_LBUTTONDOWN) return;

    const int totalCaras = (int)ProcesoRecFacial::lstUltCarasDet.size();
    for (int indiceCara = 0; indiceCara < totalCaras; indiceCara++) {
        trackFace = ProcesoRecFacial::lstUltCarasDet.at(indiceCara);

        x1 = ((float)trackFace->cara.deteccion.ptoSupIzq.x) / escalaX;
        y1 = ((float)trackFace->cara.deteccion.ptoSupIzq.y) / escalaY;
        x2 = ((float)trackFace->cara.deteccion.ptoInfDer.x) / escalaX;
        y2 = ((float)trackFace->cara.deteccion.ptoInfDer.y) / escalaY;

        const bool dentroX = (x1 <= x) && (x2 >= x);
        const bool dentroY = (y1 <= y) && (y2 >= y);

        if (dentroX && dentroY) {
            std::ostringstream oss;
            for (int indiceElem = 0; indiceElem < NUM_ELEMS_DESC_FACIAL; indiceElem++) {
                oss << trackFace->cara.descriptor[indiceElem];
                if (indiceElem < (NUM_ELEMS_DESC_FACIAL - 1)) oss << ",";
            }
            LOG_INFO(LOG_COMPONENT, "Descriptor seleccionado: " << oss.str());
            break;
        }
    }
}

/**
 * Guarda los parametros de la aplicacion
 */
void ProcesoRecFacial::guardaParametros() {
    guardaArchivoConfig(pathParametros, lstParamsApp);
}

/**
 * Notifica al servidor web sobre las detecciones ocurridas
 */
void ProcesoRecFacial::notificaDetecciones(
    GImage frameOriginal, vector<TrackedDetectionHailo*>* carasTrackFinal) {

    int iteracionesTipoPersona = 0;

    int jpegLenBytes = 0;
    size_t base64LenBytes = 0;

    double valorEscalado = 0.0;

    string jsonRostros;
    string nombresReconocidosCsv;

    TrackedDetectionHailo* trackFace = nullptr;

    char* jpegBuffer = nullptr;
    char* jpegBase64 = nullptr;

    const long long timestampEventoMs = TimeDateUtils::getDateTimeMs();
    const string fechaEventoStr = TimeDateUtils::getFechaDesdeMs(timestampEventoMs);

    DescPersonaExterno* personaDetectada = nullptr;

    iteracionesTipoPersona = (generadorEventos.usarEndpointUnificado ? 1 : 2);

    const int totalCaras = (int)carasTrackFinal->size();

    // bucle para generar datos de personas conocidas (tipoPersona=0) y para
    // personas no identificadas (tipoPersona=1) cuando NO hay endpoint unificado.
    for (int tipoPersona = 0; tipoPersona < iteracionesTipoPersona; tipoPersona++) {
        jsonRostros.clear();

        for (int indiceCara = 0; indiceCara < totalCaras; indiceCara++) {
            trackFace = carasTrackFinal->at(indiceCara);

            personaDetectada = trackFace->cara.identificador.getDatosPerIden();
            if (personaDetectada == NULL) continue;

            if (!generadorEventos.usarEndpointUnificado) {
                const bool esAnonimo = personaDetectada->anonimo;

                if ((tipoPersona == 0) && esAnonimo) continue;
                if ((tipoPersona == 1) && !esAnonimo) continue;
            }

            const long long deltaMs = timestampEventoMs - trackFace->cara.identificador.fechaUltEvento;

            const bool cumpleMinimoIdentificaciones =
                (trackFace->cara.identificador.getNumIdentificaciones() >= numIndentificacionesMin);

            const bool debeReemitirEvento =
                trackFace->cara.identificador.cambioIdentificacion ||
                (trackFace->cara.personaIdent && (deltaMs >= tiempoReEvento));

            if (cumpleMinimoIdentificaciones && debeReemitirEvento) {
                if (personaDetectada->anonimo)
                {
                    if (!shouldEmitAnonimo(personaDetectada->id, timestampEventoMs))
                    {
                        continue;
                    }
                }

                nombresReconocidosCsv.append(personaDetectada->nombre);
                nombresReconocidosCsv.append(",");

                jpegBuffer = GDibujo::encode(
                    trackFace->cara.fotoCara, GDIBUJO_ENCODE_JPEG, 80, jpegLenBytes);

                jpegBase64 = base64_encode(
                    (const unsigned char*)jpegBuffer, jpegLenBytes, &base64LenBytes);

                free(jpegBuffer);

                if (!jsonRostros.empty()) jsonRostros.append(",");

                jsonRostros.append("\n{");

                GStringUtils::addJsonAtt(&jsonRostros, "foto", jpegBase64, false);
                free(jpegBase64);

                jsonRostros.append("\"coordenadas\":{");

                valorEscalado = ((double)trackFace->cara.deteccion.ptoSupIzq.x) * factorEscalaVisualizaX;
                GStringUtils::addJsonAtt(&jsonRostros, "x1", to_string(valorEscalado), true);

                valorEscalado = ((double)trackFace->cara.deteccion.ptoSupIzq.y) * factorEscalaVisualizaY;
                GStringUtils::addJsonAtt(&jsonRostros, "y1", to_string(valorEscalado), true);

                valorEscalado = ((double)trackFace->cara.deteccion.ptoInfDer.x) * factorEscalaVisualizaX;
                GStringUtils::addJsonAtt(&jsonRostros, "x2", to_string(valorEscalado), true);

                valorEscalado = ((double)trackFace->cara.deteccion.ptoInfDer.y) * factorEscalaVisualizaY;
                GStringUtils::addJsonAtt(&jsonRostros, "y2", to_string(valorEscalado), true, false);

                jsonRostros.append("},\n");
                jsonRostros.append("\"descripcionFacial\":[\n");

                for (int indiceElem = 0; indiceElem < NUM_ELEMS_DESC_FACIAL; indiceElem++) {
                    jsonRostros.append(to_string(trackFace->cara.descriptor[indiceElem]));
                    if (indiceElem < (NUM_ELEMS_DESC_FACIAL - 1)) jsonRostros.append(",");
                }

                jsonRostros.append("\n],\n");

                if (!personaDetectada->anonimo) {
                    GStringUtils::addJsonAtt(&jsonRostros, "idReconocido", personaDetectada->id, true);
                    jsonRostros.append("\"idTipoRostro\":2,\n");
                } else {
                    GStringUtils::addJsonAtt(&jsonRostros, "idReconocido", "0", true);
                    jsonRostros.append("\"idTipoRostro\":1,\n");
                }

                GStringUtils::addJsonAtt(
                    &jsonRostros,
                    "probabilidad",
                    GStringUtils::to_string_fixed(trackFace->cara.identificador.getPromedioComparacion(), 2),
                    true,
                    false);

                jsonRostros.append("}\n");

                trackFace->cara.identificador.fechaUltEvento = timestampEventoMs;

                LOG_DEBUG(LOG_COMPONENT,
                    "ROSTRO ADD tipoPersona=" << tipoPersona
                    << " anon=" << (personaDetectada->anonimo ? "S" : "N")
                    << " id=" << personaDetectada->id
                    << " deltaMs=" << deltaMs
                    << " numId=" << trackFace->cara.identificador.getNumIdentificaciones());
            }
        }

        if (!jsonRostros.empty()) {
            string payloadEvento;

            GImage frameEscalado = frameOriginal.cloneResize(anchoVisualiza, alturaVisualiza);

            jpegBuffer = GDibujo::encode(frameEscalado, GDIBUJO_ENCODE_JPEG, compresionJpeg, jpegLenBytes);
            jpegBase64 = base64_encode((const unsigned char*)jpegBuffer, jpegLenBytes, &base64LenBytes);
            free(jpegBuffer);

            payloadEvento.append("{\n");
            GStringUtils::addJsonAtt(&payloadEvento, "cuadro", jpegBase64, false);
            free(jpegBase64);

            GStringUtils::addJsonAtt(&payloadEvento, "serieEquipo", GStringUtils::trim(serieEquipo), false);
            GStringUtils::addJsonAtt(&payloadEvento, "fecEvento", fechaEventoStr, false);

            payloadEvento.append("\"lstRostros\":[");
            payloadEvento.append(jsonRostros);
            payloadEvento.append("]\n");
            payloadEvento.append("}\n");

            LOG_DEBUG(LOG_COMPONENT,
                     "Evento generado. tipo="
                         << (tipoPersona == 0 ? "identificados" : "no_identificados")
                         << " total_rostros=" << totalCaras);

            if (tipoPersona == 0)
                generadorEventos.agregarTramaIden(payloadEvento);
            else
                generadorEventos.agregarTramaNoIden(payloadEvento);
        }
    }
}

void ProcesoRecFacial::configure() {
    const auto cfg = LectorConfig::getInstance().getParams();

    lstParamsApp = cfg;

    generadorPingsAppWeb.configure();
    if (generadorPingsMonitoreo) {
        generadorPingsMonitoreo->serieEquipo = serieEquipo;
        generadorPingsMonitoreo->configure();
    }
    procDescargaDescFaciales.configure();

    throttleNoIdentMs = lstParamsApp->getStringLong("throttleNoIdentMs", 1000);
    if (throttleNoIdentMs < 0) throttleNoIdentMs = 0;

    LOG_INFO(LOG_COMPONENT,"CONFIG Backpressure throttleNoIdentMs=" << throttleNoIdentMs);
    generadorEventos.configure();
}

void ProcesoRecFacial::setEstadoSolo(uint8_t flag)
{
    std::lock_guard<std::mutex> lock(mtx);
    flagsEstado = flag;
}

void ProcesoRecFacial::setEstadoEncendido()
{
    generadorPingsMonitoreo->setEnabled(true);
    generadorEventos.setEnabled(true);
    setEstadoSolo(EST_ENCENDIDO);
}

void ProcesoRecFacial::setEstadoApagado()
{
    generadorPingsMonitoreo->setEnabled(true);
    generadorEventos.setEnabled(false);
    setEstadoSolo(EST_APAGADO);
}

void ProcesoRecFacial::setEstadoSuspendido()
{
    generadorPingsMonitoreo->setEnabled(false);
    generadorEventos.setEnabled(false);
    setEstadoSolo(EST_SUSPENDIDO);
}

bool ProcesoRecFacial::isEncendido()
{
    std::lock_guard<std::mutex> lock(mtx);
    return (flagsEstado & EST_ENCENDIDO) != 0;
}

bool ProcesoRecFacial::isApagado()
{
    std::lock_guard<std::mutex> lock(mtx);
    return (flagsEstado & EST_APAGADO) != 0;
}

bool ProcesoRecFacial::isSuspendido()
{
    std::lock_guard<std::mutex> lock(mtx);
    return (flagsEstado & EST_SUSPENDIDO) != 0;
}

GImage ProcesoRecFacial::construirFrameAviso(int ancho, int alto, const std::vector<std::string>& lineas)
{
    // Crea un frame negro RGB
    GImage img(ancho, alto, GDIBUJO_IMAGEN_MODO_RGB);

    // "Tarjeta" central
    GRect caja;
    caja.x1 = 40;
    caja.y1 = 40;
    caja.x2 = ancho - 40;
    caja.y2 = alto - 40;

    GColor colorFondo(20, 20, 20);
    GColor colorBorde(80, 80, 80);
    GColor colorTexto(255, 255, 255);

    GDibujo::drawRect(img, caja, colorFondo, -1);
    GDibujo::drawRect(img, caja, colorBorde, 2);

    int x = caja.x1 + 30;
    int y = caja.y1 + 60;

    const double escala = 1.0;
    const int grosor = 2;
    const int salto = 45;

    for (const auto& s : lineas)
    {
        GDibujo::drawText(img, x, y, s, GDIBUJO_FONT_HELVETICA, colorTexto, escala, grosor);
        y += salto;
    }

    return img;
}

bool ProcesoRecFacial::shouldEmitAnonimo(const std::string& anonId, long long nowMs)
{
    std::lock_guard<std::mutex> lock(mtxDebounceAnon);

    auto it = lastSentAnon.find(anonId);
    if (it == lastSentAnon.end())
    {
        lastSentAnon.emplace(anonId, nowMs);
        LOG_DEBUG(LOG_COMPONENT, "THROTTLE PASS anonId=" << anonId << " (first)");
        return true;
    }

    const long long last = it->second;
    const long long delta = nowMs - last;

    if (delta < throttleNoIdentMs)
    {
        LOG_DEBUG(LOG_COMPONENT, "THROTTLE BLOCK anonId=" << anonId << " delta=" << delta << "ms < " << throttleNoIdentMs << "ms");
        return false;
    }

    it->second = nowMs;
    LOG_DEBUG(LOG_COMPONENT, "THROTTLE PASS anonId=" << anonId << " delta=" << delta << "ms");
    return true;
}
