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
            dotnetUrl + dotnetEndpointMonitoreo + idEquipo;

        if (idTareaProgramada != 0) {
            urlConsultaMonitoreo +=
                "?idTareaProgramada=" + std::to_string(idTareaProgramada);
        }

        GHttpClient httpClientConsulta;

        int resultadoSolicitudMonitoreo =
            httpClientConsulta.doHttp(urlConsultaMonitoreo, "GET", nullptr, 0);

        bool okTemperatura = false;
        double tempC = extractorTemperatura.getTempCpuC(&okTemperatura);

        if (okTemperatura)
        {
            LOG_INFO(LOG_COMPONENT, "Temperatura CPU: " << tempC << " °C");
        }
        else
        {
            LOG_ERROR(LOG_COMPONENT, "No se pudo obtener la temperatura CPU");
        }

        if (resultadoSolicitudMonitoreo != 0) {
            LOG_ERROR(
                LOG_COMPONENT,
                "ERROR al hacer GET pings dotnet. code="
                    << resultadoSolicitudMonitoreo
                    << " url=" << urlConsultaMonitoreo);
            return;
        }

        std::string cuerpoRespuesta = httpClientConsulta.responseToStr();

        LOG_DEBUG(LOG_COMPONENT,
                  "URL consulta monitoreo: " << urlConsultaMonitoreo);

        json jsonRespuesta = json::parse(cuerpoRespuesta);
        bool success = jsonRespuesta.value("success", false);

        LOG_INFO(
            LOG_COMPONENT,
            "DOTNET PING -> url=" << urlConsultaMonitoreo
                                  << " code=" << resultadoSolicitudMonitoreo
                                  << " success=" << (success ? "true" : "false"));

        if (!success) {
            return;
        }

        if (!jsonRespuesta.contains("pending") ||
            !jsonRespuesta["pending"].is_array()) {
            LOG_WARN(LOG_COMPONENT, "Respuesta sin array 'pending'");
            return;
        }

        const json& listaTareasPendientes = jsonRespuesta["pending"];

        if (listaTareasPendientes.empty()) {
            return;
        }

        for (const auto& jsonTarea : listaTareasPendientes) {
            if (!jsonTarea.is_object()) {
                continue;
            }

            int idTareaProgramadaDesdeJson =
                jsonTarea.value("idTareaProgramada", 0);
            int tipoDeTarea = jsonTarea.value("tipoDeTarea", 0);

            LOG_INFO(
                LOG_COMPONENT,
                "Tarea pendiente recibida -> idTareaProgramada="
                    << idTareaProgramadaDesdeJson
                    << " tipoDeTarea=" << tipoDeTarea);

            procesarTareaProgramada(tipoDeTarea, idTareaProgramadaDesdeJson);
        }

    } catch (const std::exception& ex) {
        LOG_ERROR(LOG_COMPONENT,
                  "Excepción en consultarTareasProgramadas: " << ex.what());
    } catch (...) {
        LOG_ERROR(LOG_COMPONENT,
                  "Excepción desconocida en consultarTareasProgramadas");
    }
}

void GeneradorPingsMonitoreo::procesarTareaProgramada(
    int tipoDeTarea, int idTareaProgramadaDesdeJson) {
    switch (tipoDeTarea) {
        case 0:
            break;
        case 1:
            idTareaProgramada = idTareaProgramadaDesdeJson;
            LOG_DEBUG(LOG_COMPONENT,
                      "Actualizando idTareaProgramada a "
                          << idTareaProgramadaDesdeJson);
            break;
        case 2:
            LOG_INFO(LOG_COMPONENT,
                     "Activando procesamiento de imágenes para tarea "
                         << idTareaProgramadaDesdeJson);
            procRecFacial->setIdTareaProgramada(idTareaProgramadaDesdeJson);
            procRecFacial->setFlagProcesarImagenes(true);
            break;
        default:
            LOG_WARN(LOG_COMPONENT,
                     "Tipo de tarea desconocido: " << tipoDeTarea);
            break;
    }
}

void GeneradorPingsMonitoreo::runThread() {
    try {
        if (dotnetUrl.empty()) {
            LOG_WARN(LOG_COMPONENT,
                     "dotnetUrl vacío. No se iniciará el envío de pings dotnet.");
            return;
        }

        LOG_INFO(LOG_COMPONENT,
                 "Thread iniciado. url=" << dotnetUrl
                                         << " intervalo=" << pingIntervalMs
                                         << " ms");

        while (!isFinalizado()) {
            try {
                auto instanteActual = std::chrono::steady_clock::now();
                auto diferenciaMilisegundos =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        instanteActual - ultimoPingTp)
                        .count();

                if (diferenciaMilisegundos >= pingIntervalMs) {
                    consultarTareasProgramadas();
                    ultimoPingTp = instanteActual;
                }

                sleepMS(5);
            } catch (const std::exception& ex) {
                LOG_ERROR(
                    LOG_COMPONENT,
                    "Excepción en bucle del thread: " << ex.what());
            } catch (...) {
                LOG_ERROR(LOG_COMPONENT,
                          "Excepción desconocida en bucle del thread");
            }
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
