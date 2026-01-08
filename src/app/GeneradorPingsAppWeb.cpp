#include <chrono>

#include "app/GeneradorPingsAppWeb.h"
#include "lib/web/GHttpClient.h"
#include "lib/utils/Logger.h"

static constexpr const char* LOG_COMPONENT = "GeneradorPingsAppWeb";

int GeneradorPingsAppWeb::hacerPing()
{
    GHttpClient httpClientPing;
    const int rc = httpClientPing.doHttp(appWebUrl, "GET", nullptr, 0);

    if (rc == 0) 
    { 
        LOG_INFO(LOG_COMPONENT, "[APP_WEB PING] OK url=" << appWebUrl << " code=" << rc); 
    }
    else 
    { 
        LOG_ERROR(LOG_COMPONENT, "[APP_WEB PING] ERROR url=" << appWebUrl << " code=" << rc); 
    }

    return rc;
}

void GeneradorPingsAppWeb::runThread()
{
    if (appWebUrl.empty()) 
    { 
        LOG_WARN(LOG_COMPONENT, "appWebUrl vacío. No se iniciará el envío de pings."); 
        return; 
    }

    LOG_INFO(LOG_COMPONENT, "Thread iniciado. url=" << appWebUrl << " intervalo=" << pingIntervalMs << " ms");

    hacerPing();
    ultimoPingTp = std::chrono::steady_clock::now();

    while (!isFinalizado())
    {
        const auto ahora = std::chrono::steady_clock::now();
        const auto diffMs = std::chrono::duration_cast<std::chrono::milliseconds>(ahora - ultimoPingTp).count();

        if (diffMs >= pingIntervalMs) 
        { 
            hacerPing(); 
            ultimoPingTp = ahora; 
        }

        GThread::sleepMS(10);
    }

    LOG_INFO(LOG_COMPONENT, "Thread finalizado");
}
