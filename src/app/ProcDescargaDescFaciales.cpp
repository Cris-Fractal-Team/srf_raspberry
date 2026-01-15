#include "app/ProcDescargaDescFaciales.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <string>

#include "lib/json/json.hpp"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/Logger.h"
#include "lib/web/GHttpClient.h"

using json = nlohmann::json;

static constexpr const char* LOG_COMPONENT = "ProcDescargaDescFaciales";

static std::string basenamePortable(const std::string& path)
{
    if (path.empty())
    {
        return "";
    }

    size_t lastSeparatorPosition = path.find_last_of("/\\");
    if (lastSeparatorPosition == std::string::npos)
    {
        return path;
    }

    return path.substr(lastSeparatorPosition + 1);
}

std::string ProcDescargaDescFaciales::obtenerNombreArchivoFinal() const
{
    const std::string defaultName = "rostros_conocidos.txt";
    std::string baseName = basenamePortable(archivoFinal);

    if (baseName.empty())
    {
        return defaultName;
    }

    return baseName;
}

void ProcDescargaDescFaciales::iniciarSesion()
{
    std::string baseTrim = GStringUtils::trim(appWebUrl);
    std::string endpointTrim = GStringUtils::trim(endpointAuth);
    std::string authUrl;

    if (!endpointTrim.empty() && (endpointTrim.rfind("http://", 0) == 0 || endpointTrim.rfind("https://", 0) == 0))
    {
        authUrl = endpointTrim;
    }
    else if (endpointTrim.empty())
    {
        authUrl = baseTrim;
    }
    else if (!baseTrim.empty())
    {
        if (baseTrim.back() == '/' && endpointTrim.front() == '/')
        {
            authUrl = baseTrim + endpointTrim.substr(1);
        }
        else if (baseTrim.back() != '/' && endpointTrim.front() != '/')
        {
            authUrl = baseTrim + "/" + endpointTrim;
        }
        else
        {
            authUrl = baseTrim + endpointTrim;
        }
    }
    else
    {
        authUrl = endpointTrim;
    }

    if (authUrl.empty())
    {
        LOG_WARN(LOG_COMPONENT, "URL de autenticacion vacia");
        return;
    }

    json loginJson;
    loginJson["username"] = username;
    loginJson["password"] = password;

    std::string body = loginJson.dump();

    LOG_INFO(LOG_COMPONENT, "Iniciando sesion en: " << authUrl);

    GHttpClient httpClient;

    char* bodyPtr = nullptr;
    if (!body.empty())
    {
        bodyPtr = &body[0];
    }

    httpClient.setHeader("Content-Type", "application/json");

    int rc = httpClient.doHttp(authUrl, "POST", bodyPtr, static_cast<int>(body.size()));
    if (rc != 0)
    {
        LOG_ERROR(LOG_COMPONENT, "ERROR en iniciarSesion. rc=" << rc << " url=" << authUrl);
        return;
    }

    std::string responseBody = httpClient.responseToStr();
    if (responseBody.empty())
    {
        LOG_WARN(LOG_COMPONENT, "Respuesta de login vacia. url=" << authUrl);
        return;
    }

    try
    {
        json respJson = json::parse(responseBody);

        if (!respJson.contains("token") || !respJson["token"].is_string())
        {
            LOG_WARN(LOG_COMPONENT, "Respuesta de login sin campo token valido. url=" << authUrl);
            return;
        }

        token = respJson["token"].get<std::string>();

        std::string mensaje = "Login realizado";
        if (respJson.contains("mensaje") && respJson["mensaje"].is_string())
        {
            mensaje = respJson["mensaje"].get<std::string>();
        }

        LOG_INFO(LOG_COMPONENT, "Login OK. mensaje=" << mensaje);
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(LOG_COMPONENT, "Error parseando JSON de login: " << ex.what());
    }
}

bool ProcDescargaDescFaciales::descargarZipRostros(const std::string& downloadUrl, const std::string& datasetDirectory)
{
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath = scriptsDirectory + "/descargarZipRostros.sh";

    std::string downloadCommand = scriptPath + " \"" + downloadUrl + "\" \"" + datasetDirectory + "\"";
    downloadCommand += " \"" + token + "\"";

    std::string downloadCommandLog = scriptPath + " \"" + downloadUrl + "\" \"" + datasetDirectory + "\"";
    downloadCommandLog += " \"" + token.substr(0, 8) + "***SECRET***" + "\"";

    LOG_INFO(LOG_COMPONENT, "Ejecutando descarga: " << downloadCommandLog);

    int downloadExitCode = std::system(downloadCommand.c_str());
    if (downloadExitCode != 0)
    {
        LOG_ERROR(LOG_COMPONENT, "ERROR al ejecutar descargarZipRostros.sh (codigo " << downloadExitCode << ")");
        return false;
    }

    return true;
}

bool ProcDescargaDescFaciales::generarDescriptoresFaciales(const std::string& datasetDirectory)
{
    (void)datasetDirectory;

    LOG_INFO(LOG_COMPONENT, "Iniciando extractor.ejecutar() para generar descriptores faciales");

    extractor.ejecutar();

    LOG_INFO(LOG_COMPONENT, "extractor.ejecutar() finalizado");

    return true;
}

void ProcDescargaDescFaciales::crearCopiaSeguridadDescriptores(const std::string& datasetDirectory)
{
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath = scriptsDirectory + "/copiaDeSeguridadDescFaciales.sh";

    std::time_t currentTime = std::time(nullptr);
    std::string backupLabel = "rostros_" + std::to_string(static_cast<long long>(currentTime));

    std::string apiUrl = GStringUtils::trim(appWebUrl);
    std::string outputName = obtenerNombreArchivoFinal();

    // Firma:
    // copiaDeSeguridadDescFaciales.sh <DIRECTORIO_DATASET> <API_URL> <SERIE_EQUIPO> <TOKEN> [ETIQUETA_OPCIONAL] [NOMBRE_ARCHIVO_FINAL]
    std::string backupCommand = scriptPath + " \"" + datasetDirectory + "\" \"" + apiUrl + "\" \"" + serieEquipo + "\" \"" + token + "\" \"" + backupLabel + "\" \"" + outputName + "\"";

    std::string backupCommandLog = scriptPath + " \"" + datasetDirectory + "\" \"" + apiUrl + "\" \"" + serieEquipo + "\" \"" + token.substr(0, 8) + "***SECRET***" + "\" \"" + backupLabel + "\" \"" + outputName + "\"";

    LOG_INFO(LOG_COMPONENT, "Ejecutando backup: " << backupCommandLog);

    int backupExitCode = std::system(backupCommand.c_str());
    if (backupExitCode != 0)
    {
        LOG_WARN(LOG_COMPONENT, "AVISO: error al crear copia de seguridad (codigo " << backupExitCode << ")");
    }
}

void ProcDescargaDescFaciales::limpiarArchivosRostros(const std::string& datasetDirectory)
{
    const std::string scriptsDirectory = "./scripts";
    std::string scriptPath = scriptsDirectory + "/limpiarRostros.sh";

    std::string outputName = obtenerNombreArchivoFinal();

    // Firma:
    // limpiarRostros.sh <DIRECTORIO_DATASET> [NOMBRE_ARCHIVO_FINAL]
    std::string cleanupCommand = scriptPath + " \"" + datasetDirectory + "\" \"" + outputName + "\"";

    LOG_INFO(LOG_COMPONENT, "Ejecutando limpieza: " << cleanupCommand);

    int cleanupExitCode = std::system(cleanupCommand.c_str());
    if (cleanupExitCode != 0)
    {
        LOG_WARN(LOG_COMPONENT, "AVISO: error al limpiar rostros (codigo " << cleanupExitCode << ")");
    }
}

bool ProcDescargaDescFaciales::ejecutarDescargaYGeneracion()
{
    try
    {
        iniciarSesion();

        std::string downloadUrl = construirUrlDescarga();
        if (downloadUrl.empty())
        {
            LOG_WARN(LOG_COMPONENT, "URL de descarga vacía");
            return false;
        }

        std::string datasetDirectory = extractor.pathFotos;

        LOG_INFO(LOG_COMPONENT, "Iniciando flujo descarga + generación + backup + limpieza con URL: " << downloadUrl);

        bool isSuccessful = true;

        if (!descargarZipRostros(downloadUrl, datasetDirectory))
        {
            isSuccessful = false;
        }

        if (isSuccessful)
        {
            if (!generarDescriptoresFaciales(datasetDirectory))
            {
                isSuccessful = false;
            }
        }

        crearCopiaSeguridadDescriptores(datasetDirectory);
        limpiarArchivosRostros(datasetDirectory);

        if (isSuccessful)
        {
            LOG_INFO(LOG_COMPONENT, "Flujo completado correctamente");
        }
        else
        {
            LOG_WARN(LOG_COMPONENT, "Flujo finalizado con errores en la descarga o generacion de descriptores");
        }

        notificarTareaCompletada();

        if (!token.empty())
        {
            std::fill(token.begin(), token.end(), '\0');
            token.clear();
            token.shrink_to_fit();
        }

        return isSuccessful;
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción en ejecutarDescargaYGeneracion: " << ex.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción desconocida en ejecutarDescargaYGeneracion");
        return false;
    }
}

std::string ProcDescargaDescFaciales::construirUrlDescarga() const
{
    std::string baseTrim = GStringUtils::trim(appWebUrl);
    std::string endpointTrim = GStringUtils::trim(endpointDescargaZip);

    if (!endpointTrim.empty())
    {
        if (endpointTrim.rfind("http://", 0) == 0 || endpointTrim.rfind("https://", 0) == 0)
        {
            return endpointTrim;
        }
    }

    if (endpointTrim.empty())
    {
        return baseTrim;
    }

    if (!baseTrim.empty() && !endpointTrim.empty())
    {
        if (baseTrim.back() == '/' && endpointTrim.front() == '/')
        {
            return baseTrim + endpointTrim.substr(1);
        }
        else if (baseTrim.back() != '/' && endpointTrim.front() != '/')
        {
            return baseTrim + "/" + endpointTrim;
        }
        else
        {
            return baseTrim + endpointTrim;
        }
    }

    return baseTrim + endpointTrim;
}

bool ProcDescargaDescFaciales::ejecutarScriptShell(const std::string& urlDescarga) const
{
    if (rutaScriptGenerar.empty())
    {
        LOG_WARN(LOG_COMPONENT, "rutaScriptGenerar vacía. No se ejecutará ningún script.");
        return false;
    }

    std::string comando = rutaScriptGenerar + " \"" + urlDescarga + "\"";

    LOG_INFO(LOG_COMPONENT, "Ejecutando comando: " << comando);

    int resultado = std::system(comando.c_str());
    if (resultado == 0)
    {
        return true;
    }

    return false;
}

void ProcDescargaDescFaciales::notificarTareaCompletada()
{
    try
    {
        std::string baseTrim = GStringUtils::trim(dotnetUrl);
        std::string endpointTrim = GStringUtils::trim(endpointCompletado);
        std::string urlCompleta;

        if (!endpointTrim.empty() && (endpointTrim.rfind("http://", 0) == 0 || endpointTrim.rfind("https://", 0) == 0))
        {
            urlCompleta = endpointTrim;
        }
        else if (endpointTrim.empty())
        {
            urlCompleta = baseTrim;
        }
        else if (!baseTrim.empty())
        {
            if (baseTrim.back() == '/' && endpointTrim.front() == '/')
            {
                urlCompleta = baseTrim + endpointTrim.substr(1);
            }
            else if (baseTrim.back() != '/' && endpointTrim.front() != '/')
            {
                urlCompleta = baseTrim + "/" + endpointTrim;
            }
            else
            {
                urlCompleta = baseTrim + endpointTrim;
            }
        }
        else
        {
            urlCompleta = endpointTrim;
        }

        if (urlCompleta.empty())
        {
            LOG_WARN(LOG_COMPONENT, "URL de completado vacía, no se envía POST");
            return;
        }

        if (urlCompleta.back() != '/')
        {
            urlCompleta += "/";
        }

        urlCompleta += std::to_string(idTareaProgramada);

        LOG_INFO(LOG_COMPONENT, "Notificando tarea completada en: " << urlCompleta);

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

        json bodyJson;
        bodyJson["nroDeSerie"] = serieEquipo;
        bodyJson["temperatura"] = tempC;
        bodyJson["usuario"] = username;

        std::string body = bodyJson.dump();

        char* bodyPtr = nullptr;
        if (!body.empty())
        {
            bodyPtr = &body[0];
        }

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type", "application/json");

        int rc = httpClient.doHttp(urlCompleta, "POST", bodyPtr, static_cast<int>(body.size()));
        if (rc != 0)
        {
            LOG_ERROR(LOG_COMPONENT, "ERROR en notificarTareaCompletada. rc=" << rc << " url=" << urlCompleta);
            return;
        }

        std::string responseBody = httpClient.responseToStr();
        if (responseBody.empty())
        {
            LOG_WARN(LOG_COMPONENT, "Respuesta vacía en notificarTareaCompletada. url=" << urlCompleta);
            return;
        }

        try
        {
            json respJson = json::parse(responseBody);
            bool success = respJson.value("success", false);

            LOG_INFO(
                LOG_COMPONENT,
                "POST completado -> url=" << urlCompleta << " rc=" << rc << " success=" << (success ? "true" : "false") << " response=" << responseBody);
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR(LOG_COMPONENT, "Error parseando JSON en notificarTareaCompletada: " << ex.what());
        }
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción en notificarTareaCompletada: " << ex.what());
    }
    catch (...)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción desconocida en notificarTareaCompletada");
    }
}
