#include "app/EmisorCorreosAlerta.h"

#include <algorithm>
#include <exception>

#include "app/LectorConfig.h"
#include "lib/json/json.hpp"
#include "lib/utils/GStringUtils.h"
#include "lib/utils/Logger.h"
#include "lib/web/GHttpClient.h"

using json = nlohmann::json;

void EmisorCorreosAlerta::configure()
{
    const auto cfg = LectorConfig::getInstance().getParams();

    appWebUrl           = cfg->getString("appWebUrl");
    endpointAuth        = cfg->getString("authEndpoint");
    endpointEmailAlerta = cfg->getString("emailAlertaEndpoint"); // ej: /srf/sensor/email-alerta
    username            = cfg->getString("username");
    password            = cfg->getString("password");
}

void EmisorCorreosAlerta::setSerieEquipo(const std::string& serie)
{
    serieEquipo = serie;
}

void EmisorCorreosAlerta::postEmailAlerta(double temperaturaC)
{
    try
    {
        if (serieEquipo.empty())
        {
            LOG_ERROR(LOG_COMPONENT, "serieEquipo vacío. No se puede enviar correo.");
            return;
        }

        std::string token = iniciarSesion();
        if (token.empty())
        {
            LOG_ERROR(LOG_COMPONENT, "No se obtuvo token. No se enviará correo.");
            return;
        }

        std::string url = construirUrl(endpointEmailAlerta);
        if (url.empty())
        {
            LOG_ERROR(LOG_COMPONENT, "URL de email alerta vacía. No se enviará correo.");
            return;
        }

        json bodyJson;
        bodyJson["temperatura"] = temperaturaC;
        bodyJson["serieEquipo"] = serieEquipo;

        std::string body = bodyJson.dump();

        LOG_INFO(LOG_COMPONENT, "POST email alerta -> url=" << url << " serie=" << serieEquipo << " tempC=" << temperaturaC);

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type", "application/json");
        httpClient.setHeader("Authorization", "Bearer " + token);

        char* bodyPtr = nullptr;
        if (!body.empty())
        {
            bodyPtr = &body[0];
        }

        int rc = httpClient.doHttp(url, "POST", bodyPtr, static_cast<int>(body.size()));
        if (rc != 0)
        {
            LOG_ERROR(LOG_COMPONENT, "ERROR POST email alerta. rc=" << rc << " url=" << url);
            return;
        }

        std::string responseBody = httpClient.responseToStr();
        LOG_INFO(LOG_COMPONENT, "POST email alerta OK. response=" << responseBody);

        std::fill(token.begin(), token.end(), '\0');
        token.clear();
        token.shrink_to_fit();
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción en postEmailAlerta: " << ex.what());
    }
    catch (...)
    {
        LOG_ERROR(LOG_COMPONENT, "Excepción desconocida en postEmailAlerta");
    }
}

std::string EmisorCorreosAlerta::iniciarSesion()
{
    try
    {
        std::string authUrl = construirUrl(endpointAuth);
        if (authUrl.empty())
        {
            LOG_WARN(LOG_COMPONENT, "URL de autenticación vacía");
            return "";
        }

        json loginJson;
        loginJson["username"] = username;
        loginJson["password"] = password;

        std::string body = loginJson.dump();

        LOG_INFO(LOG_COMPONENT, "Iniciando sesión -> url=" << authUrl);

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type", "application/json");

        char* bodyPtr = nullptr;
        if (!body.empty())
        {
            bodyPtr = &body[0];
        }

        int rc = httpClient.doHttp(authUrl, "POST", bodyPtr, static_cast<int>(body.size()));
        if (rc != 0)
        {
            LOG_ERROR(LOG_COMPONENT, "ERROR login. rc=" << rc << " url=" << authUrl);
            return "";
        }

        std::string responseBody = httpClient.responseToStr();
        if (responseBody.empty())
        {
            LOG_WARN(LOG_COMPONENT, "Respuesta de login vacía. url=" << authUrl);
            return "";
        }

        json respJson = json::parse(responseBody);
        if (!respJson.contains("token") || !respJson["token"].is_string())
        {
            LOG_WARN(LOG_COMPONENT, "Respuesta de login sin token válido. url=" << authUrl);
            return "";
        }

        LOG_INFO(LOG_COMPONENT, "Login OK");

        return respJson["token"].get<std::string>();
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR(LOG_COMPONENT, "Error en iniciarSesion: " << ex.what());
        return "";
    }
    catch (...)
    {
        LOG_ERROR(LOG_COMPONENT, "Error desconocido en iniciarSesion");
        return "";
    }
}

std::string EmisorCorreosAlerta::construirUrl(const std::string& endpoint) const
{
    std::string baseTrim = GStringUtils::trim(appWebUrl);
    std::string endpointTrim = GStringUtils::trim(endpoint);

    if (!endpointTrim.empty() &&
        (endpointTrim.rfind("http://", 0) == 0 || endpointTrim.rfind("https://", 0) == 0))
    {
        return endpointTrim;
    }

    if (endpointTrim.empty())
    {
        return baseTrim;
    }

    if (!baseTrim.empty())
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

    return endpointTrim;
}
