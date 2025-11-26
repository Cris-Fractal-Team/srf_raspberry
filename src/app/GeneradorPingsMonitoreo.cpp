#include "app/GeneradorPingsMonitoreo.h"

#include <iostream>
#include <string>
#include <exception>

#include "lib/json/json.hpp"
#include "lib/utils/GLog.h"
#include "lib/utils/GStringUtils.h"
#include "lib/web/GHttpClient.h"

using json = nlohmann::json;

void GeneradorPingsMonitoreo::consultarTareasProgramadas() {
    try {
        /*  std::string serieEquipo = GStringUtils::trim(idEquipo);
            std::string urlConsultaMonitoreo =
            dotnetUrl + "/api/ping/log/" + serieEquipo +
            "?idTareaProgramada=" + std::to_string(idTareaProgramada); */
        std::string urlConsultaMonitoreo =
            dotnetUrl;

        GHttpClient httpClientConsulta;

        int resultadoSolicitudMonitoreo =
            httpClientConsulta.doHttp(urlConsultaMonitoreo, "GET", nullptr, 0);
            
        if (resultadoSolicitudMonitoreo != 0) {
            if (generarLogPing) {
                GLog::writeSimple(
                    "[GeneradorPingsMonitoreo] ERROR al hacer GET pings dotnet. code=" +
                        std::to_string(resultadoSolicitudMonitoreo) +
                        " url=" + urlConsultaMonitoreo,
                    true);
            } else {
                std::cout << "[GeneradorPingsMonitoreo] ERROR al hacer GET pings dotnet "
                          << "url=" << urlConsultaMonitoreo
                          << " code=" << resultadoSolicitudMonitoreo
                          << std::endl;
            }
            return;
        }

        std::string cuerpoRespuesta = httpClientConsulta.responseToStr();

        json jsonRespuesta = json::parse(cuerpoRespuesta);
        // const string& jsonMensaje = jsonRespuesta["mensaje"];
        std::string jsonMensaje = jsonRespuesta.value("status", std::string{});

        if (generarLogPing) {
            if (resultadoSolicitudMonitoreo == 0)
                GLog::writeSimple("[GeneradorPingsMonitoreo] APP_WEB PING OK -> " + dotnetUrl, true);
            else
                GLog::writeSimple(
                    "[GeneradorPingsMonitoreo] APP_WEB PING ERROR code=" +
                        std::to_string(resultadoSolicitudMonitoreo) + " -> " +
                        dotnetUrl,
                    true);
        } else {
            std::cout << ((resultadoSolicitudMonitoreo == 0)
                              ? "[GeneradorPingsMonitoreo] APP_WEB PING OK "
                              : "[GeneradorPingsMonitoreo] APP_WEB PING ERROR ")
                      << "url=" << dotnetUrl
                      << " code=" << resultadoSolicitudMonitoreo << std::endl;
        }
        std::cout << "[GeneradorPingsMonitoreo] Mensaje de la API: " << jsonMensaje << std::endl;
        /* if (!jsonRespuesta.contains("pending") ||
            !jsonRespuesta["pending"].is_array() ||
            jsonRespuesta["pending"].empty()) {
            return;
        }

        const json& jsonTareaPendiente = jsonRespuesta["pending"][0];

        int idTareaProgramadaDesdeJson =
            jsonTareaPendiente.value("idTareaProgramada", 0);
        int tipoDeTarea = jsonTareaPendiente.value("tipoDeTarea", 0);

        procesarTareaProgramada(tipoDeTarea, idTareaProgramadaDesdeJson); */
    } catch (const std::exception& ex) {
        if (generarLogPing) {
            GLog::writeSimple(
                std::string("[GeneradorPingsMonitoreo] Excepción en consultarTareasProgramadas: ") +
                    ex.what(),
                true);
        } else {
            std::cout << "[GeneradorPingsMonitoreo] Excepción en consultarTareasProgramadas: "
                      << ex.what() << std::endl;
        }
    } catch (...) {
        if (generarLogPing) {
            GLog::writeSimple(
                "[GeneradorPingsMonitoreo] Excepción desconocida en consultarTareasProgramadas",
                true);
        } else {
            std::cout << "[GeneradorPingsMonitoreo] Excepción desconocida en consultarTareasProgramadas"
                      << std::endl;
        }
    }
}

void GeneradorPingsMonitoreo::procesarTareaProgramada(
    int tipoDeTarea, int idTareaProgramadaDesdeJson) {
    switch (tipoDeTarea) {
        case 0:
            break;
        case 1:
            idTareaProgramada = idTareaProgramadaDesdeJson;
            break;
        case 2:
            break;
        default:
            break;
    }
}

void GeneradorPingsMonitoreo::runThread() {
    try {
        if (dotnetUrl.empty()) {
            std::cout << "[GeneradorPingsMonitoreo] dotnetUrl vacío. No se iniciará "
                         "el envío de pings dotnet."
                      << std::endl;
            return;
        }

        if (generarLogPing) {
            std::cout << "[GeneradorPingsMonitoreo] Creando archivo de logs de "
                         "pings dotnet"
                      << std::endl;
            GLog::open(pathLogPing);
        }

        std::cout << "[GeneradorPingsMonitoreo] Thread iniciado. url=" << dotnetUrl
                  << " intervalo=" << pingIntervalMs << " ms" << std::endl;

        while (isFinalizado() == false) {
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
                if (generarLogPing) {
                    GLog::writeSimple(
                        std::string("[GeneradorPingsMonitoreo] Excepción en bucle del thread: ") +
                            ex.what(),
                        true);
                } else {
                    std::cout << "[GeneradorPingsMonitoreo] Excepción en bucle del thread: "
                              << ex.what() << std::endl;
                }
            } catch (...) {
                if (generarLogPing) {
                    GLog::writeSimple(
                        "[GeneradorPingsMonitoreo] Excepción desconocida en bucle del thread",
                        true);
                } else {
                    std::cout << "[GeneradorPingsMonitoreo] Excepción desconocida en bucle del thread"
                              << std::endl;
                }
            }
        }

        if (generarLogPing) {
            std::cout << "[GeneradorPingsMonitoreo] Cerrando archivo de logs de "
                         "pings dotnet"
                      << std::endl;
            GLog::close();
        }
        std::cout << "[GeneradorPingsMonitoreo] Thread finalizado" << std::endl;
    } catch (const std::exception& ex) {
        if (generarLogPing) {
            GLog::writeSimple(
                std::string("[GeneradorPingsMonitoreo] Excepción fuera del bucle del thread: ") +
                    ex.what(),
                true);
        } else {
            std::cout << "[GeneradorPingsMonitoreo] Excepción fuera del bucle del thread: "
                      << ex.what() << std::endl;
        }
    } catch (...) {
        if (generarLogPing) {
            GLog::writeSimple(
                "[GeneradorPingsMonitoreo] Excepción desconocida fuera del bucle del thread",
                true);
        } else {
            std::cout << "[GeneradorPingsMonitoreo] Excepción desconocida fuera del bucle del thread"
                      << std::endl;
        }
    }
}
