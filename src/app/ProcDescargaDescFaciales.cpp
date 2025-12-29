#include "app/ProcDescargaDescFaciales.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <string>

#include "lib/json/json.hpp"
#include "lib/utils/GLog.h"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/Logger.h"
#include "lib/web/GHttpClient.h"

using json = nlohmann::json;

static constexpr const char* LOG_COMPONENT = "ProcDescargaDescFaciales";

static std::string basenamePortable(const std::string& path) {
    if (path.empty()) return "";
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

std::string ProcDescargaDescFaciales::obtenerNombreArchivoFinal() const {
    const std::string defaultName = "rostros_conocidos.txt";
    std::string b = basenamePortable(archivoFinal);
    if (b.empty()) return defaultName;
    return b;
}

void ProcDescargaDescFaciales::iniciarSesion() {
    std::string baseTrim = GStringUtils::trim(appWebUrl);
    std::string endpointTrim = GStringUtils::trim(endpointAuth);
    std::string authUrl;

    if (!endpointTrim.empty() && (endpointTrim.rfind("http://", 0) == 0 ||
                                  endpointTrim.rfind("https://", 0) == 0)) {
        authUrl = endpointTrim;
    } else if (endpointTrim.empty()) {
        authUrl = baseTrim;
    } else if (!baseTrim.empty()) {
        if (baseTrim.back() == '/' && endpointTrim.front() == '/')
            authUrl = baseTrim + endpointTrim.substr(1);
        else if (baseTrim.back() != '/' && endpointTrim.front() != '/')
            authUrl = baseTrim + "/" + endpointTrim;
        else
            authUrl = baseTrim + endpointTrim;
    } else {
        authUrl = endpointTrim;
    }

    if (authUrl.empty()) {
        if (generarLog) {
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] URL de autenticacion vacia", true);
        } else {
            LOG_WARN(LOG_COMPONENT, "URL de autenticacion vacia");
        }
        return;
    }

    json loginJson;
    loginJson["username"] = username;
    loginJson["password"] = password;
    std::string body = loginJson.dump();

    if (generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] Iniciando sesion en: " + authUrl, true);
    } else {
        LOG_INFO(LOG_COMPONENT, "Iniciando sesion en: " << authUrl);
    }

    GHttpClient httpClient;

    char* bodyPtr = body.empty() ? nullptr : &body[0];
    httpClient.setHeader("Content-Type", "application/json");

    int rc = httpClient.doHttp(authUrl, "POST", bodyPtr,
                               static_cast<int>(body.size()));

    if (rc != 0) {
        if (generarLog) {
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] ERROR en iniciarSesion. rc=" +
                    std::to_string(rc),
                true);
        } else {
            LOG_ERROR(LOG_COMPONENT, "ERROR en iniciarSesion. rc="
                                         << rc << " url=" << authUrl);
        }
        return;
    }

    std::string responseBody = httpClient.responseToStr();
    if (responseBody.empty()) {
        if (generarLog) {
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] Respuesta de login vacia", true);
        } else {
            LOG_WARN(LOG_COMPONENT,
                     "Respuesta de login vacia. url=" << authUrl);
        }
        return;
    }

    try {
        json respJson = json::parse(responseBody);

        if (!respJson.contains("token") || !respJson["token"].is_string()) {
            if (generarLog) {
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] Respuesta de login sin campo "
                    "token valido",
                    true);
            } else {
                LOG_WARN(LOG_COMPONENT,
                         "Respuesta de login sin campo token valido. url="
                             << authUrl);
            }
            return;
        }

        token = respJson["token"].get<std::string>();

        if (generarLog) {
            std::string mensaje =
                respJson.contains("mensaje") && respJson["mensaje"].is_string()
                    ? respJson["mensaje"].get<std::string>()
                    : "Login realizado";
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] Login OK. mensaje: " + mensaje,
                true);
        } else {
            std::string mensaje =
                respJson.contains("mensaje") && respJson["mensaje"].is_string()
                    ? respJson["mensaje"].get<std::string>()
                    : "Login realizado";
            LOG_INFO(LOG_COMPONENT, "Login OK. mensaje=" << mensaje);
        }
    } catch (const std::exception& ex) {
        if (generarLog) {
            GLog::writeSimple(std::string("[ProcDescargaDescFaciales] Error "
                                          "parseando JSON de login: ") +
                                  ex.what(),
                              true);
        } else {
            LOG_ERROR(LOG_COMPONENT,
                      "Error parseando JSON de login: " << ex.what());
        }
    }
}

bool ProcDescargaDescFaciales::descargarZipRostros(
    const std::string& downloadUrl, const std::string& datasetDirectory) {
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath = scriptsDirectory + "/descargarZipRostros.sh";

    std::string downloadCommand =
        scriptPath + " \"" + downloadUrl + "\" \"" + datasetDirectory + "\"";
    downloadCommand += " \"" + token + "\"";

    std::string downloadCommandLog =
        scriptPath + " \"" + downloadUrl + "\" \"" + datasetDirectory + "\"";
    downloadCommand += " \"" + token.substr(0, 8) + "***SECRET***" + "\"";

    if (generarLog) {
        GLog::writeSimple("[ProcDescargaDescFaciales] Ejecutando descarga: " +
                          downloadCommandLog,
                          true);
    } else {
        LOG_INFO(LOG_COMPONENT, "Ejecutando descarga: " << downloadCommandLog);
    }

    int downloadExitCode = std::system(downloadCommand.c_str());

    if (downloadExitCode != 0) {
        if (generarLog) {
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] ERROR al ejecutar "
                "descargarZipRostros.sh (codigo " +
                    std::to_string(downloadExitCode) + ")",
                true);
        } else {
            LOG_ERROR(LOG_COMPONENT,
                      "ERROR al ejecutar descargarZipRostros.sh (codigo "
                          << downloadExitCode << ")");
        }
        return false;
    }

    return true;
}

bool ProcDescargaDescFaciales::generarDescriptoresFaciales(
    const std::string& datasetDirectory) {
    (void)datasetDirectory;

    if (generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] Iniciando extractor.ejecutar() para "
            "generar descriptores faciales",
            true);
    } else {
        LOG_INFO(LOG_COMPONENT,
                 "Iniciando extractor.ejecutar() para generar descriptores "
                 "faciales");
    }

    extractor.ejecutar();

    if (generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] extractor.ejecutar() finalizado", true);
    } else {
        LOG_INFO(LOG_COMPONENT, "extractor.ejecutar() finalizado");
    }

    return true;
}

void ProcDescargaDescFaciales::crearCopiaSeguridadDescriptores(
    const std::string& datasetDirectory) {
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath =
        scriptsDirectory + "/copiaDeSeguridadDescFaciales.sh";

    std::time_t currentTime = std::time(nullptr);
    std::string backupLabel =
        "rostros_" + std::to_string(static_cast<long long>(currentTime));

    std::string apiUrl = GStringUtils::trim(appWebUrl);
    std::string outputName = obtenerNombreArchivoFinal();

    // Firma:
    // copiaDeSeguridadDescFaciales.sh <DIRECTORIO_DATASET> <API_URL>
    // <SERIE_EQUIPO> <TOKEN> [ETIQUETA_OPCIONAL] [NOMBRE_ARCHIVO_FINAL]
    std::string backupCommand = scriptPath + " \"" + datasetDirectory +
                                "\" \"" + apiUrl + "\" \"" + idEquipo +
                                "\" \"" + token + "\" \"" + backupLabel +
                                "\" \"" + outputName + "\"";

    std::string backupCommandLog = scriptPath + " \"" + datasetDirectory +
                                "\" \"" + apiUrl + "\" \"" + idEquipo +
                                "\" \"" + token.substr(0, 8) + "***SECRET***" + "\" \"" + backupLabel +
                                "\" \"" + outputName + "\"";
    if (generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] Ejecutando backup: " + backupCommandLog,
            true);
    } else {
        LOG_INFO(LOG_COMPONENT, "Ejecutando backup: " << backupCommandLog);
    }

    int backupExitCode = std::system(backupCommand.c_str());
    if (backupExitCode != 0 && generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] AVISO: error al crear copia de "
            "seguridad (codigo " +
                std::to_string(backupExitCode) + ")",
            true);
    } else if (backupExitCode != 0) {
        LOG_WARN(LOG_COMPONENT,
                 "AVISO: error al crear copia de seguridad (codigo "
                     << backupExitCode << ")");
    }
}

void ProcDescargaDescFaciales::limpiarArchivosRostros(
    const std::string& datasetDirectory) {
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath = scriptsDirectory + "/limpiarRostros.sh";

    std::string outputName = obtenerNombreArchivoFinal();

    // Firma:
    // limpiarRostros.sh <DIRECTORIO_DATASET> [NOMBRE_ARCHIVO_FINAL]
    std::string cleanupCommand =
        scriptPath + " \"" + datasetDirectory + "\" \"" + outputName + "\"";

    if (generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] Ejecutando limpieza: " + cleanupCommand,
            true);
    } else {
        LOG_INFO(LOG_COMPONENT, "Ejecutando limpieza: " << cleanupCommand);
    }

    int cleanupExitCode = std::system(cleanupCommand.c_str());
    if (cleanupExitCode != 0 && generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] AVISO: error al limpiar rostros "
            "(codigo " +
                std::to_string(cleanupExitCode) + ")",
            true);
    } else if (cleanupExitCode != 0) {
        LOG_WARN(LOG_COMPONENT, "AVISO: error al limpiar rostros (codigo "
                                    << cleanupExitCode << ")");
    }
}

bool ProcDescargaDescFaciales::ejecutarDescargaYGeneracion() {
    try {
        iniciarSesion();

        std::string downloadUrl = construirUrlDescarga();
        if (downloadUrl.empty()) {
            if (generarLog) {
                GLog::open(pathLog);
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] URL de descarga vacía", true);
                GLog::close();
            } else {
                LOG_WARN(LOG_COMPONENT, "URL de descarga vacía");
            }
            return false;
        }

        std::string datasetDirectory = extractor.pathFotos;

        if (generarLog) {
            GLog::open(pathLog);
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] Iniciando flujo descarga + "
                "generación + backup + limpieza con URL: " +
                    downloadUrl,
                true);
        } else {
            LOG_INFO(LOG_COMPONENT,
                     "Iniciando flujo descarga + generación + backup + "
                     "limpieza con URL: "
                         << downloadUrl);
        }

        bool isSuccessful = true;

        if (!descargarZipRostros(downloadUrl, datasetDirectory)) {
            isSuccessful = false;
        }

        if (isSuccessful) {
            if (!generarDescriptoresFaciales(datasetDirectory)) {
                isSuccessful = false;
            }
        }

        crearCopiaSeguridadDescriptores(datasetDirectory);
        limpiarArchivosRostros(datasetDirectory);

        if (generarLog) {
            if (isSuccessful)
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] Flujo completado correctamente",
                    true);
            else
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] Flujo finalizado con errores "
                    "en la descarga o generacion de descriptores",
                    true);
            GLog::close();
        } else {
            if (isSuccessful) {
                LOG_INFO(LOG_COMPONENT, "Flujo completado correctamente");
            } else {
                LOG_WARN(LOG_COMPONENT,
                         "Flujo finalizado con errores en la descarga o "
                         "generacion de descriptores");
            }
        }

        notificarTareaCompletada();

        if (!token.empty()) {
            std::fill(token.begin(), token.end(), '\0');
            token.clear();
            token.shrink_to_fit();
        }

        return isSuccessful;
    } catch (const std::exception& ex) {
        if (generarLog) {
            GLog::open(pathLog);
            GLog::writeSimple(
                std::string("[ProcDescargaDescFaciales] Excepción en "
                            "ejecutarDescargaYGeneracion: ") +
                    ex.what(),
                true);
            GLog::close();
        } else {
            LOG_ERROR(
                LOG_COMPONENT,
                "Excepción en ejecutarDescargaYGeneracion: " << ex.what());
        }
        return false;
    } catch (...) {
        if (generarLog) {
            GLog::open(pathLog);
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] Excepción desconocida en "
                "ejecutarDescargaYGeneracion",
                true);
            GLog::close();
        } else {
            LOG_ERROR(LOG_COMPONENT,
                      "Excepción desconocida en ejecutarDescargaYGeneracion");
        }
        return false;
    }
}

std::string ProcDescargaDescFaciales::construirUrlDescarga() const {
    std::string baseTrim = GStringUtils::trim(appWebUrl);
    std::string endpointTrim = GStringUtils::trim(endpointDescargaZip);

    if (!endpointTrim.empty()) {
        if (endpointTrim.rfind("http://", 0) == 0 ||
            endpointTrim.rfind("https://", 0) == 0) {
            return endpointTrim;
        }
    }

    if (endpointTrim.empty()) {
        return baseTrim;
    }

    if (!baseTrim.empty() && !endpointTrim.empty()) {
        if (baseTrim.back() == '/' && endpointTrim.front() == '/')
            return baseTrim + endpointTrim.substr(1);
        else if (baseTrim.back() != '/' && endpointTrim.front() != '/')
            return baseTrim + "/" + endpointTrim;
        else
            return baseTrim + endpointTrim;
    }

    return baseTrim + endpointTrim;
}

bool ProcDescargaDescFaciales::ejecutarScriptShell(
    const std::string& urlDescarga) const {
    if (rutaScriptGenerar.empty()) {
        LOG_WARN(LOG_COMPONENT,
                 "rutaScriptGenerar vacía. No se ejecutará ningún script.");
        return false;
    }

    std::string comando = rutaScriptGenerar + " \"" + urlDescarga + "\"";

    if (generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] Ejecutando comando: " + comando, true);
    } else {
        LOG_INFO(LOG_COMPONENT, "Ejecutando comando: " << comando);
    }

    int resultado = std::system(comando.c_str());
    return (resultado == 0);
}

void ProcDescargaDescFaciales::notificarTareaCompletada() {
    try {
        std::string baseTrim = GStringUtils::trim(dotnetUrl);
        std::string endpointTrim = GStringUtils::trim(endpointCompletado);
        std::string urlCompleta;

        if (!endpointTrim.empty() && (endpointTrim.rfind("http://", 0) == 0 ||
                                      endpointTrim.rfind("https://", 0) == 0)) {
            urlCompleta = endpointTrim;
        } else if (endpointTrim.empty()) {
            urlCompleta = baseTrim;
        } else if (!baseTrim.empty()) {
            if (baseTrim.back() == '/' && endpointTrim.front() == '/')
                urlCompleta = baseTrim + endpointTrim.substr(1);
            else if (baseTrim.back() != '/' && endpointTrim.front() != '/')
                urlCompleta = baseTrim + "/" + endpointTrim;
            else
                urlCompleta = baseTrim + endpointTrim;
        } else {
            urlCompleta = endpointTrim;
        }

        if (urlCompleta.empty()) {
            LOG_WARN(LOG_COMPONENT,
                     "URL de completado vacía, no se envía POST");
            return;
        }

        if (urlCompleta.back() != '/') urlCompleta += "/";
        urlCompleta += std::to_string(idTareaProgramada);

        LOG_INFO(LOG_COMPONENT,
                 "Notificando tarea completada en: " << urlCompleta);

        json bodyJson;
        bodyJson["nroDeSerie"] = idEquipo;
        std::string body = bodyJson.dump();
        char* bodyPtr = body.empty() ? nullptr : &body[0];

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type", "application/json");

        int rc = httpClient.doHttp(urlCompleta, "POST", bodyPtr,
                                   static_cast<int>(body.size()));

        if (rc != 0) {
            LOG_ERROR(LOG_COMPONENT, "ERROR en notificarTareaCompletada. rc="
                                         << rc << " url=" << urlCompleta);
            return;
        }

        std::string responseBody = httpClient.responseToStr();
        if (responseBody.empty()) {
            LOG_WARN(LOG_COMPONENT,
                     "Respuesta vacía en notificarTareaCompletada. url="
                         << urlCompleta);
            return;
        }

        try {
            json respJson = json::parse(responseBody);
            bool success = respJson.value("success", false);

            LOG_INFO(LOG_COMPONENT,
                     "POST completado -> url=" << urlCompleta << " rc=" << rc
                                               << " success="
                                               << (success ? "true" : "false")
                                               << " response=" << responseBody);
        } catch (const std::exception& ex) {
            LOG_ERROR(LOG_COMPONENT,
                      "Error parseando JSON en notificarTareaCompletada: "
                          << ex.what());
        }
    } catch (const std::exception& ex) {
        LOG_ERROR(LOG_COMPONENT,
                  "Excepción en notificarTareaCompletada: " << ex.what());
    } catch (...) {
        LOG_ERROR(LOG_COMPONENT,
                  "Excepción desconocida en notificarTareaCompletada");
    }
}
