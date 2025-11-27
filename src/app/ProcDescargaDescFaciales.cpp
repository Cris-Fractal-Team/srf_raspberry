#include "app/ProcDescargaDescFaciales.h"

#include <cstdlib>
#include <exception>
#include <iostream>

#include "lib/utils/GLog.h"
#include "lib/utils/GStringUtils.h"

bool ProcDescargaDescFaciales::ejecutarDescargaYGeneracion() {
    try {
        std::string urlDescarga = construirUrlDescarga();
        if (urlDescarga.empty()) {
            if (generarLog) {
                GLog::open(pathLog);
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] URL de descarga vacía", true);
                GLog::close();
            } else {
                std::cout << "[ProcDescargaDescFaciales] URL de descarga vacía"
                          << std::endl;
            }
            return false;
        }

        if (generarLog) {
            GLog::open(pathLog);
            GLog::writeSimple(
                "[ProcDescargaDescFaciales] Iniciando script "
                "generarDescFaciales.sh con URL: " +
                    urlDescarga,
                true);
        } else {
            std::cout << "[ProcDescargaDescFaciales] Iniciando script "
                         "generarDescFaciales.sh con URL: "
                      << urlDescarga << std::endl;
        }

        bool ok = ejecutarScriptShell(urlDescarga);

        if (generarLog) {
            if (ok)
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] Script generarDescFaciales.sh "
                    "ejecutado correctamente",
                    true);
            else
                GLog::writeSimple(
                    "[ProcDescargaDescFaciales] ERROR al ejecutar script "
                    "generarDescFaciales.sh",
                    true);
            GLog::close();
        } else {
            if (ok)
                std::cout << "[ProcDescargaDescFaciales] Script "
                             "generarDescFaciales.sh ejecutado correctamente"
                          << std::endl;
            else
                std::cout << "[ProcDescargaDescFaciales] ERROR al ejecutar "
                             "script generarDescFaciales.sh"
                          << std::endl;
        }

        return ok;
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
            std::cout << "[ProcDescargaDescFaciales] Excepción en "
                         "ejecutarDescargaYGeneracion: "
                      << ex.what() << std::endl;
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
            std::cout << "[ProcDescargaDescFaciales] Excepción desconocida en "
                         "ejecutarDescargaYGeneracion"
                      << std::endl;
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
        std::cout << "[ProcDescargaDescFaciales] rutaScriptGenerar vacía. No "
                     "se ejecutará ningún script."
                  << std::endl;
        return false;
    }

    std::string comando = rutaScriptGenerar + " \"" + urlDescarga + "\"";

    if (generarLog) {
        GLog::writeSimple(
            "[ProcDescargaDescFaciales] Ejecutando comando: " + comando, true);
    } else {
        std::cout << "[ProcDescargaDescFaciales] Ejecutando comando: "
                  << comando << std::endl;
    }

    int resultado = std::system(comando.c_str());
    return (resultado == 0);
}
