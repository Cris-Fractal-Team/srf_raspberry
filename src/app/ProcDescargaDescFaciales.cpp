#include "app/ProcDescargaDescFaciales.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ctime>

#include "lib/utils/GLog.h"
#include "lib/utils/GStringUtils.h"
#include "lib/web/GHttpClient.h"
#include "lib/json/json.hpp"

using json = nlohmann::json;

void ProcDescargaDescFaciales::iniciarSesion() {
    std::string baseTrim = GStringUtils::trim(appWebUrl);
    std::string endpointTrim = GStringUtils::trim(endpointAuth);
    std::string authUrl;

    if (!endpointTrim.empty() &&
        (endpointTrim.rfind("http://", 0) == 0 ||
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
            GLog::writeSimple("[ProcDescargaDescFaciales] URL de autenticacion vacia", true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] URL de autenticacion vacia" << std::endl;
        }
        return;
    }

    json loginJson;
    loginJson["username"] = username;
    loginJson["password"] = password;
    std::string body = loginJson.dump();
    if (generarLog) {
        GLog::writeSimple("[ProcDescargaDescFaciales] Iniciando sesion en: " + authUrl, true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] Iniciando sesion en: " << authUrl << std::endl;
    }

    GHttpClient httpClient;

    char* bodyPtr = body.empty() ? nullptr : &body[0];
    httpClient.setHeader("Content-Type", "application/json");
    int rc = httpClient.doHttp(authUrl, "POST", bodyPtr,
                               static_cast<int>(body.size()));

    if (rc != 0) {
        if (generarLog) {
            GLog::writeSimple("[ProcDescargaDescFaciales] ERROR en iniciarSesion. rc=" + std::to_string(rc), true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] ERROR en iniciarSesion. rc=" << rc << std::endl;
        }
        return;
    }

    std::string responseBody = httpClient.responseToStr();
    if (responseBody.empty()) {
        if (generarLog) {
            GLog::writeSimple("[ProcDescargaDescFaciales] Respuesta de login vacia", true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] Respuesta de login vacia" << std::endl;
        }
        return;
    }

    try {
        json respJson = json::parse(responseBody);

        if (!respJson.contains("token") || !respJson["token"].is_string()) {
            if (generarLog) {
                GLog::writeSimple("[ProcDescargaDescFaciales] Respuesta de login sin campo token valido", true);
            } else {
                std::cout << "[ProcDescargaDescFaciales] Respuesta de login sin campo token valido" << std::endl;
            }
            return;
        }

        token = respJson["token"].get<std::string>();

        if (generarLog) {
            std::string mensaje =
                respJson.contains("mensaje") && respJson["mensaje"].is_string()
                    ? respJson["mensaje"].get<std::string>()
                    : "Login realizado";
            GLog::writeSimple("[ProcDescargaDescFaciales] Login OK. mensaje: " + mensaje, true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] Login OK. Token obtenido." << std::endl;
        }
    } catch (const std::exception& ex) {
        if (generarLog) {
            GLog::writeSimple(std::string("[ProcDescargaDescFaciales] Error parseando JSON de login: ") + ex.what(),
                              true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] Error parseando JSON de login: " << ex.what() << std::endl;
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

    if (generarLog) {
        GLog::writeSimple("[ProcDescargaDescFaciales] Ejecutando descarga: " + downloadCommand, true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] Ejecutando descarga: " << downloadCommand << std::endl;
    }

    int downloadExitCode = std::system(downloadCommand.c_str());

    if (!token.empty()) {
        std::fill(token.begin(), token.end(), '\0');
        token.clear();
        token.shrink_to_fit();
    }

    if (downloadExitCode != 0) {
        if (generarLog) {
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] ERROR al ejecutar descargarZipRostros.sh (codigo " +
                    std::to_string(downloadExitCode) + ")",
                true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] ERROR al ejecutar descargarZipRostros.sh (codigo "
                      << downloadExitCode << ")" << std::endl;
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
            "[ProcDescargaDescFaciales] Iniciando extractor.ejecutar() para generar descriptores faciales",
            true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] Iniciando extractor.ejecutar() para generar descriptores faciales"
                  << std::endl;
    }

    extractor.ejecutar();

    if (generarLog) {
        GLog::writeSimple("[ProcDescargaDescFaciales] extractor.ejecutar() finalizado", true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] extractor.ejecutar() finalizado" << std::endl;
    }

    return true;
}

void ProcDescargaDescFaciales::crearCopiaSeguridadDescriptores(
    const std::string& datasetDirectory) {
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath = scriptsDirectory + "/copiaDeSeguridadDescFaciales.sh";

    std::time_t currentTime = std::time(nullptr);
    std::string backupLabel =
        "rostros_" + std::to_string(static_cast<long long>(currentTime));

    std::string backupCommand =
        scriptPath + " \"" + datasetDirectory + "\" \"" + backupLabel + "\"";

    if (generarLog) {
        GLog::writeSimple("[ProcDescargaDescFaciales] Ejecutando backup: " + backupCommand, true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] Ejecutando backup: " << backupCommand << std::endl;
    }

    int backupExitCode = std::system(backupCommand.c_str());
    if (backupExitCode != 0 && generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] AVISO: error al crear copia de seguridad (codigo " +
                std::to_string(backupExitCode) + ")",
            true);
    } else if (backupExitCode != 0) {
        std::cout << "[ProcDescargaDescFaciales] AVISO: error al crear copia de seguridad (codigo " << backupExitCode
                  << ")" << std::endl;
    }
}

void ProcDescargaDescFaciales::limpiarArchivosRostros(
    const std::string& datasetDirectory) {
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath = scriptsDirectory + "/limpiarRostros.sh";

    std::string cleanupCommand = scriptPath + " \"" + datasetDirectory + "\"";

    if (generarLog) {
        GLog::writeSimple("[ProcDescargaDescFaciales] Ejecutando limpieza: " + cleanupCommand, true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] Ejecutando limpieza: " << cleanupCommand << std::endl;
    }

    int cleanupExitCode = std::system(cleanupCommand.c_str());
    if (cleanupExitCode != 0 && generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] AVISO: error al limpiar rostros (codigo " +
                std::to_string(cleanupExitCode) + ")",
            true);
    } else if (cleanupExitCode != 0) {
        std::cout << "[ProcDescargaDescFaciales] AVISO: error al limpiar rostros (codigo " << cleanupExitCode << ")"
                  << std::endl;
    }
}

bool ProcDescargaDescFaciales::ejecutarDescargaYGeneracion() {
    try {
        iniciarSesion();

        std::string downloadUrl = construirUrlDescarga();
        if (downloadUrl.empty()) {
            if (generarLog) {
                GLog::open(pathLog);
                GLog::writeSimple("[ProcDescargaDescFaciales] URL de descarga vacía", true);
                GLog::close();
            } else {
                std::cout << "[ProcDescargaDescFaciales] URL de descarga vacía" << std::endl;
            }
            return false;
        }

        std::string datasetDirectory = extractor.pathFotos;

        if (generarLog) {
            GLog::open(pathLog);
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] Iniciando flujo descarga + generación + backup + limpieza con URL: " +
                    downloadUrl,
                true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] Iniciando flujo descarga + generación + backup + limpieza con URL: "
                      << downloadUrl << std::endl;
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
                GLog::writeSimple("[ProcDescargaDescFaciales] Flujo completado correctamente", true);
            else
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] Flujo finalizado con errores en la descarga o generacion de descriptores",
                    true);
            GLog::close();
        } else {
            if (isSuccessful)
                std::cout << "[ProcDescargaDescFaciales] Flujo completado correctamente" << std::endl;
            else
                std::cout << "[ProcDescargaDescFaciales] Flujo finalizado con errores en la descarga o generacion de "
                             "descriptores"
                          << std::endl;
        }

        notificarTareaCompletada();

        return isSuccessful;
    } catch (const std::exception& ex) {
        if (generarLog) {
            GLog::open(pathLog);
            GLog::writeSimple(
                std::string("[ProcDescargaDescFaciales] Excepción en ejecutarDescargaYGeneracion: ") + ex.what(),
                true);
            GLog::close();
        } else {
            std::cout << "[ProcDescargaDescFaciales] Excepción en ejecutarDescargaYGeneracion: " << ex.what()
                      << std::endl;
        }
        return false;
    } catch (...) {
        if (generarLog) {
            GLog::open(pathLog);
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] Excepción desconocida en ejecutarDescargaYGeneracion", true);
            GLog::close();
        } else {
            std::cout << "[ProcDescargaDescFaciales] Excepción desconocida en ejecutarDescargaYGeneracion" << std::endl;
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
        std::cout << "[ProcDescargaDescFaciales] rutaScriptGenerar vacía. No se ejecutará ningún script." << std::endl;
        return false;
    }

    std::string comando = rutaScriptGenerar + " \"" + urlDescarga + "\"";

    if (generarLog) {
        GLog::writeSimple("[ProcDescargaDescFaciales] Ejecutando comando: " + comando, true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] Ejecutando comando: " << comando << std::endl;
    }

    int resultado = std::system(comando.c_str());
    return (resultado == 0);
}

void ProcDescargaDescFaciales::notificarTareaCompletada()
{
    try {
        std::string baseTrim     = GStringUtils::trim(dotnetUrl);
        std::string endpointTrim = GStringUtils::trim(endpointCompletado);
        std::string urlCompleta;

        if (!endpointTrim.empty() &&
            (endpointTrim.rfind("http://", 0) == 0 ||
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
            std::cout
                << "[ProcDescargaDescFaciales] URL de completado vacía, no se envía POST"
                << std::endl;
            return;
        }

        if (urlCompleta.back() != '/')
            urlCompleta += "/";
        urlCompleta += std::to_string(idTareaProgramada);

        std::cout << "[ProcDescargaDescFaciales] Notificando tarea completada en: "
                  << urlCompleta << std::endl;

        json bodyJson;
        bodyJson["nroDeSerie"] = idEquipo;
        std::string body = bodyJson.dump();
        char* bodyPtr = body.empty() ? nullptr : &body[0];

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type", "application/json");

        int rc = httpClient.doHttp(
            urlCompleta,
            "POST",
            bodyPtr,
            static_cast<int>(body.size())
        );

        if (rc != 0) {
            std::cout
                << "[ProcDescargaDescFaciales] ERROR en notificarTareaCompletada. rc="
                << rc << std::endl;
            return;
        }

        std::string responseBody = httpClient.responseToStr();
        if (responseBody.empty()) {
            std::cout
                << "[ProcDescargaDescFaciales] Respuesta vacía en notificarTareaCompletada"
                << std::endl;
            return;
        }

        try {
            json respJson = json::parse(responseBody);
            bool success = respJson.value("success", false);
            std::cout
                << "[ProcDescargaDescFaciales] POST /ping/completo "
                << (success ? "OK" : "NO OK")
                << ". Respuesta: " << responseBody << std::endl;
        }
        catch (const std::exception& ex) {
            std::cout
                << "[ProcDescargaDescFaciales] Error parseando JSON en notificarTareaCompletada: "
                << ex.what() << std::endl;
        }
    }
    catch (const std::exception& ex) {
        std::cout
            << "[ProcDescargaDescFaciales] Excepción en notificarTareaCompletada: "
            << ex.what() << std::endl;
    }
}
