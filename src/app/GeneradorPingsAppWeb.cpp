#include <iostream>
#include <chrono>

#include "app/GeneradorPingsAppWeb.h"
#include "lib/web/GHttpClient.h"
#include "lib/utils/GLog.h"

void GeneradorPingsAppWeb::runThread()
{
    if (appWebUrl.empty())
    {
        std::cout << "[GeneradorPingsAppWeb] appWebUrl vacío. No se iniciará el envío de pings." << std::endl;
        return;
    }

    if (generarLogPing)
    {
        std::cout << "[GeneradorPingsAppWeb] Creando archivo de logs de pings app web" << std::endl;
        GLog::open(pathLogPing);
    }

    std::cout << "[GeneradorPingsAppWeb] Thread iniciado. url=" << appWebUrl
              << " intervalo=" << pingIntervalMs << " ms" << std::endl;

    ultimoPingTp = std::chrono::steady_clock::now();

    while (isFinalizado() == false)
    {
        auto ahora = std::chrono::steady_clock::now();
        auto diffMs = std::chrono::duration_cast<std::chrono::milliseconds>(ahora - ultimoPingTp).count();

        if (diffMs >= pingIntervalMs)
        {
            GHttpClient httpClientPing;

            // GET sin cuerpo
            int rc = httpClientPing.doHttp(appWebUrl, "GET", nullptr, 0);

            if (generarLogPing)
            {
                if (rc == 0)
                    GLog::writeSimple("[APP_WEB PING] OK -> " + appWebUrl, true);
                else
                    GLog::writeSimple("[APP_WEB PING] ERROR code=" + std::to_string(rc) + " -> " + appWebUrl, true);
            }
            else
            {
                std::cout << ((rc == 0) ? "[APP_WEB PING] OK " : "[APP_WEB PING] ERROR ")
                          << "url=" << appWebUrl
                          << " code=" << rc << std::endl;
            }

            ultimoPingTp = ahora;
        }

        // Pequeña espera para no ciclar la CPU
        GThread::sleepMS(10);
    }

    if (generarLogPing)
    {
        std::cout << "[GeneradorPingsAppWeb] Cerrando archivo de logs de pings app web" << std::endl;
        GLog::close();
    }

    std::cout << "[GeneradorPingsAppWeb] Thread finalizado" << std::endl;
}
