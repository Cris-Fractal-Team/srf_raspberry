#include <chrono>
#include "app/GeneradorEventos.h"
#include "lib/web/GHttpClient.h"
#include "lib/utils/Logger.h"

static constexpr const char* LOG_COMPONENT = "GeneradorEventos";

/**
 * Trama que se solicita enviar perteneciente a una persona identificada
 */
void GeneradorEventos::agregarTramaIden(string trama)
{
    mtxBloquea();
    lstTramasPendIden.add(trama);
    mtxLibera();

    LOG_DEBUG(LOG_COMPONENT,
              "Encolando trama IDENT. Pendientes="
                  << lstTramasPendIden.size());
}

/**
 * Trama que se solicita enviar perteneciente a una persona NO identificada
 */
void GeneradorEventos::agregarTramaNoIden(string trama)
{
    mtxBloquea();
    lstTramasPendNoIden.add(trama);
    mtxLibera();

    LOG_DEBUG(LOG_COMPONENT,
              "Encolando trama NO IDENT. Pendientes="
                  << lstTramasPendNoIden.size());
}

/**
 * Valida si hay eventos pendientes de ser procesado
 **/
bool GeneradorEventos::hayEventosPend()
{
    bool rpta;

    mtxBloquea();
    if (lstTramasPendIden.size() > 0)
        rpta = true;
    else if (lstTramasPendNoIden.size() > 0)
        rpta = true;
    else
        rpta = false;
    mtxLibera();

    return rpta;
}

/**
 * Bucle del thread
 */
void GeneradorEventos::runThread()
{
    string trama;
    string urlServidor;
    int i;

    LOG_INFO(LOG_COMPONENT, "Thread Generador Eventos iniciado");

    while (isFinalizado() == false)
    {
        auto inicioBucle = std::chrono::high_resolution_clock::now();

        if (hayEventosPend() == false)
        {
            sleepMS(5);
            continue;
        }

        auto obtieneTrama = std::chrono::high_resolution_clock::now();

        mtxBloquea();
        bool esIdent = false;
        if (lstTramasPendIden.size() > 0)
        {
            trama = lstTramasPendIden.get(0);
            esIdent = true;
            if (usarEndpointUnificado == false)
                urlServidor = urlServidorIden;
            else
                urlServidor = urlServidorUnificado;
        }
        else
        {
            trama = lstTramasPendNoIden.get(0);
            esIdent = false;
            if (usarEndpointUnificado == false)
                urlServidor = urlServidorNoIden;
            else
                urlServidor = urlServidorUnificado;
        }
        mtxLibera();

        auto inicioHttp = std::chrono::high_resolution_clock::now();

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type", "application/json");
        i = httpClient.doHttp(urlServidor,
                              "POST",
                              (char*)trama.c_str(),
                              trama.length());

        auto finHttp = std::chrono::high_resolution_clock::now();

        auto durInicio = std::chrono::duration_cast<std::chrono::microseconds>(obtieneTrama - inicioBucle);
        auto durTrama  = std::chrono::duration_cast<std::chrono::microseconds>(inicioHttp - obtieneTrama);
        auto durHttp   = std::chrono::duration_cast<std::chrono::milliseconds>(finHttp - inicioHttp);

        if (i == 0)
        {
            mtxBloquea();
            if (esIdent && lstTramasPendIden.size() > 0)
            {
                lstTramasPendIden.remove(0);
                LOG_INFO(
                    LOG_COMPONENT,
                    "Evento IDENT enviado OK. Pendientes="
                        << lstTramasPendIden.size()
                        << " T.Inicio=" << durInicio.count() << "us"
                        << " T.Trama=" << durTrama.count() << "us"
                        << " T.Http=" << durHttp.count() << "ms");
            }
            else if (!esIdent && lstTramasPendNoIden.size() > 0)
            {
                lstTramasPendNoIden.remove(0);
                LOG_INFO(
                    LOG_COMPONENT,
                    "Evento NO IDENT enviado OK. Pendientes="
                        << lstTramasPendNoIden.size()
                        << " T.Inicio=" << durInicio.count() << "us"
                        << " T.Trama=" << durTrama.count() << "us"
                        << " T.Http=" << durHttp.count() << "ms");
            }
            mtxLibera();

            if (generarLogEventos)
            {
                LOG_DEBUG(LOG_COMPONENT,
                          "Trama enviada (resumen): " << trama);
            }
        }
        else
        {
            LOG_ERROR(LOG_COMPONENT,
                      "Error al enviar deteccion. code="
                          << i << " url=" << urlServidor);
        }
    }

    LOG_INFO(LOG_COMPONENT, "Thread Generador Eventos finalizado");
}
