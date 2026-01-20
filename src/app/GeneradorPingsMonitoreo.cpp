#include "app/GeneradorPingsMonitoreo.h"

#include <exception>
#include <string>

#include "app/ProcRecFacial.h"
#include "lib/json/json.hpp"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/Logger.h"
#include "lib/web/GHttpClient.h"

using json = nlohmann::json;

static constexpr const char* LOG_COMPONENT = "GeneradorPingsMonitoreo";

void GeneradorPingsMonitoreo::consultarTareasProgramadas() {
    try {
        std::string urlConsultaMonitoreo =
            dotnetUrl + dotnetEndpointMonitoreo + serieEquipo;

        bool okTemperatura = false;
        double tempC = extractorTemperatura.getTempCpuC(&okTemperatura);

        if (okTemperatura)
        {
            LOG_INFO(LOG_COMPONENT, "Temperatura CPU: " << tempC << " °C");
            urlConsultaMonitoreo += "?temperatura=" + std::to_string(tempC);
            if(extractorTemperatura.esTemperaturaCritica(tempC)){
                procRecFacial->setEstadoSuspendido();
                emisorCorreosAlerta.postEmailAlerta(tempC);
            }
        }
        else
        {
            LOG_ERROR(LOG_COMPONENT, "No se pudo obtener la temperatura CPU");
        }

        if (!username.empty()) {
            urlConsultaMonitoreo += "&usuario=" + username;
        }

        if (idTareaProgramada != 0 && procRecFacial->isEncendido()) {
            urlConsultaMonitoreo += "&idTareaProgramada=" + std::to_string(idTareaProgramada);
        }

        LOG_DEBUG(LOG_COMPONENT, "Haciendo GET monitoreo: " << urlConsultaMonitoreo);

        GHttpClient httpClientConsulta;
        httpClientConsulta.setHeader("Content-Type", "application/json");

        int resultadoSolicitudMonitoreo = httpClientConsulta.doHttp(urlConsultaMonitoreo, "GET", nullptr, 0);

        LOG_DEBUG(LOG_COMPONENT, "GET retornó code=" << resultadoSolicitudMonitoreo);

        if (resultadoSolicitudMonitoreo != 0) {
            LOG_ERROR(LOG_COMPONENT, "ERROR al hacer GET pings dotnet. code=" << resultadoSolicitudMonitoreo << " url=" << urlConsultaMonitoreo);
            return;
        }

        std::string cuerpoRespuesta = httpClientConsulta.responseToStr();
        json jsonRespuesta = json::parse(cuerpoRespuesta);

        bool success = jsonRespuesta.value("success", false);

        LOG_INFO(LOG_COMPONENT, "DOTNET PING -> url=" << urlConsultaMonitoreo << " code=" << resultadoSolicitudMonitoreo << " success=" << (success ? "true" : "false"));

        if (!success) {
            if(idTareaProgramada != 0)
            {
                idTareaProgramada = 0;
                consultarTareasProgramadas();
            }
            return;
        }

        if (!jsonRespuesta.contains("pending") || !jsonRespuesta["pending"].is_array()) 
        {
            LOG_WARN(LOG_COMPONENT, "Respuesta sin array 'pending'");
            return;
        }

        const json& listaTareasPendientes = jsonRespuesta["pending"];

        if (listaTareasPendientes.empty()) {
            return;
        }

        for (const auto& jsonTarea : listaTareasPendientes) 
        {
            if (!jsonTarea.is_object()) 
            {
                continue;
            }

            int idTareaProgramadaDesdeJson = jsonTarea.value("idTareaProgramada", 0);
            int tipoDeTarea = jsonTarea.value("tipoDeTarea", 0);

            LOG_INFO(LOG_COMPONENT, "Tarea pendiente recibida -> idTareaProgramada=" << idTareaProgramadaDesdeJson << " tipoDeTarea=" << tipoDeTarea);

            procesarTareaProgramada(tipoDeTarea, idTareaProgramadaDesdeJson);
        }
    } catch (const std::exception& ex) {
        LOG_ERROR(LOG_COMPONENT, "Excepción en consultarTareasProgramadas: " << ex.what());
    } catch (...) {
        LOG_ERROR(LOG_COMPONENT, "Excepción desconocida en consultarTareasProgramadas");
    }
}

void GeneradorPingsMonitoreo::procesarTareaProgramada(int tipoDeTarea, int idTareaProgramadaDesdeJson) {
    if(procRecFacial->isEncendido()){
        switch (tipoDeTarea) {
            case 0:
                break;
            case 1:
                idTareaProgramada = idTareaProgramadaDesdeJson;
                LOG_DEBUG(LOG_COMPONENT, "Actualizando idTareaProgramada a " << idTareaProgramadaDesdeJson);
                break;
            case 2:
                LOG_INFO(LOG_COMPONENT, "Activando procesamiento de imágenes para tarea " << idTareaProgramadaDesdeJson);
                procRecFacial->setIdTareaProgramada(idTareaProgramadaDesdeJson);
                procRecFacial->setFlagProcesarImagenes(true);
                break;
            case 4:
                LOG_INFO(LOG_COMPONENT, "Cambiando el estado a APAGADO PROGRAMADO segun la tarea programada " << idTareaProgramadaDesdeJson);
                procRecFacial->setEstadoApagado();
                break;
            default:
                LOG_WARN(LOG_COMPONENT, "Tipo de tarea desconocido: " << tipoDeTarea);
                break;
        }
    }else if (procRecFacial->isApagado()){
       switch (tipoDeTarea) {
            case 0:
                break;
            case 1:
                idTareaProgramada = idTareaProgramadaDesdeJson;
                LOG_DEBUG(LOG_COMPONENT, "Actualizando idTareaProgramada a " << idTareaProgramadaDesdeJson);
                procRecFacial->setEstadoEncendido();
                break;
            case 2:
                LOG_INFO(LOG_COMPONENT, "APAGADO: ignorando tarea tipo 2 (procesamiento de imágenes) id=" << idTareaProgramada);
                break;
            case 4:
                LOG_INFO(LOG_COMPONENT, "Manteniendo el estado a APAGADO PROGRAMADO segun la tarea programada " << idTareaProgramadaDesdeJson);
                break;
            default:
                LOG_WARN(LOG_COMPONENT, "Tipo de tarea desconocido: " << tipoDeTarea);
                break;
        } 
    }
}

void GeneradorPingsMonitoreo::runThread() {
    try {
        if (dotnetUrl.empty()) {
            LOG_WARN(LOG_COMPONENT, "dotnetUrl vacío. No se iniciará el envío de pings dotnet.");
            return;
        }

        LOG_INFO(LOG_COMPONENT, "Thread iniciado. url=" << dotnetUrl << " intervalo=" << pingIntervalMs << " ms");

        consultarTareasProgramadas();
        ultimoPingTp = std::chrono::steady_clock::now();

        while (!isFinalizado()) {
            if (!enabled.load())
            {
                // modo deshabilitado: duerme y no hace llamadas
                sleepMS(200);
                continue;
            }
            auto ahora = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(ahora - ultimoPingTp).count();

            if (diff >= pingIntervalMs) {
                consultarTareasProgramadas();
                ultimoPingTp = ahora;
            }

            sleepMS(5);
        }

        LOG_INFO(LOG_COMPONENT, "Thread finalizado");
    } catch (const std::exception& ex) {
        LOG_ERROR(LOG_COMPONENT,
                  "Excepción fuera del bucle del thread: " << ex.what());
    } catch (...) {
        LOG_ERROR(LOG_COMPONENT,
                  "Excepción desconocida fuera del bucle del thread");
    }
}

void GeneradorPingsMonitoreo::configure() {
        const auto cfg = LectorConfig::getInstance().getParams();
        dotnetUrl               = cfg->getString("dotnetUrl");
        dotnetEndpointMonitoreo = cfg->getString("dotnetEndpointMonitoreo");

        generarLogPing = cfg->getStringBool("generarLogPingMonitoreo", false);
        pathLogPing    = cfg->getString("pathLogPingMonitoreo");
        pingIntervalMs = cfg->getStringLong("pingIntervalMsMonitoreo", 5000);
        username = cfg->getString("username");
        emisorCorreosAlerta.configure();
        emisorCorreosAlerta.setSerieEquipo(serieEquipo);
        extractorTemperatura.configure();
}