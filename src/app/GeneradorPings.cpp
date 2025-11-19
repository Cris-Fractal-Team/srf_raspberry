#include <iostream>
#include <chrono>
#include "app/GeneradorPings.h"
#include "lib/web/GHttpClient.h"
#include "lib/utils/GLog.h"

/**
 * Encola un ping para “identificados”
 */
void GeneradorPings::encolarPingIdentificado(string trama)
{
    mtxBloquea();
    lstTramasPendIden.add(trama);
    // cout << "Num tramas identificadas pendientes " << lstTramasPendIden.size() << endl;
    mtxLibera();
}

/**
 * Encola un ping para “no identificados”
 */
void GeneradorPings::encolarPingNoIdentificado(string trama)
{
    mtxBloquea();
    lstTramasPendNoIden.add(trama);
    // cout << "Num tramas NO identificadas pendientes " << lstTramasPendNoIden.size() << endl;
    mtxLibera();
}

/**
 * Valida si hay eventos pendientes de ser procesado
 **/
bool GeneradorPings::hayEventosPend()
{
    bool rpta;

    mtxBloquea();
    if ( lstTramasPendIden.size() > 0 ) rpta=true;
    else
    if ( lstTramasPendNoIden.size() > 0 ) rpta=true;
    else rpta = false;
    mtxLibera();

    return rpta;
}

/**
 * Bucle del thread
 */
void GeneradorPings::runThread()
{
    string trama;    
    string urlServidor;
    int i;

    if ( generarLogPing == true )
    {
        std::cout << "Creando archivo de logs de pings" << std::endl;
        GLog::open(pathLogPing);
    }
    
    std::cout << "Thread Generador Pings iniciado" << std::endl;
    while( isFinalizado() == false )
    {
        auto inicioBucle = std::chrono::high_resolution_clock::now();

        {
            auto ahora = std::chrono::steady_clock::now();
            auto diffMs = std::chrono::duration_cast<std::chrono::milliseconds>(ahora - ultimoPingTp).count();
            if (diffMs >= pingIntervalMs) {
                const std::string pingPayload = "{\"tipo\":\"ping\",\"intervalo_ms\":" + std::to_string(pingIntervalMs) + "}";
        
                GHttpClient httpClientPing;
                httpClientPing.setHeader("Content-Type","application/json");
                int rc = httpClientPing.doHttp(urlPing, "POST", (char*)pingPayload.c_str(), (int)pingPayload.size());
        
                if (generarLogPing) {
                    if (rc == 0) GLog::writeSimple("[PING 5s] OK -> " + urlPing, true);
                    else         GLog::writeSimple("[PING 5s] ERROR code=" + std::to_string(rc) + " -> " + urlPing, true);
                } else {
                    std::cout << ((rc==0) ? "[PING 5s] OK " : "[PING 5s] ERROR ") << "url=" << urlPing << std::endl;
                }
        
                ultimoPingTp = ahora; // resetea el reloj del ping forzado
            }
        }
        
        if ( hayEventosPend() == false )
        {
            sleepMS(5);
            continue;
        }

        auto obtieneTrama = std::chrono::high_resolution_clock::now();

        mtxBloquea();
        if ( lstTramasPendIden.size() > 0 ) 
        {
            trama = lstTramasPendIden.get(0);
            urlServidor = urlPing; // único endpoint de pings
        }            
        else 
        {
            trama = lstTramasPendNoIden.get(0);
            urlServidor = urlPing; // único endpoint de pings
        }
        mtxLibera();

        auto inicioHttp = std::chrono::high_resolution_clock::now();
        GHttpClient httpClient;
        httpClient.setHeader("Content-Type","application/json");
        // cout << "Trama JSON:" << trama << endl;        
        i = httpClient.doHttp(urlServidor, "POST", (char *)trama.c_str(), (int)trama.length());

        auto finHttp = std::chrono::high_resolution_clock::now();

        auto durInicio = std::chrono::duration_cast<std::chrono::microseconds>(obtieneTrama - inicioBucle);
        auto durTrama  = std::chrono::duration_cast<std::chrono::microseconds>(inicioHttp - obtieneTrama);
        auto durHttp   = std::chrono::duration_cast<std::chrono::milliseconds>(finHttp - inicioHttp);
        
        if ( i == 0 )
        {
            // exito en la transferencia se saca de la cola
            mtxBloquea();
            if ( lstTramasPendIden.size() > 0 )  
            {
                lstTramasPendIden.remove(0);
                // cout << "Pings Identificados pendientes " <<  lstTramasPendIden.size() << " T.Inicio " << durInicio.count()  <<  " mcs T.Trama " << durTrama.count() << " mcs T.Http " << durHttp.count() << " ms " << endl;
            }
            else 
            {
                lstTramasPendNoIden.remove(0);    
                // cout << "Pings No Identificados pendientes " <<  lstTramasPendNoIden.size() << " T.Inicio " << durInicio.count()  <<  " mcs T.Trama " << durTrama.count() << " mcs T.Http " << durHttp.count() << " ms " << endl;     
            }            
            mtxLibera();
        }
        else
        {
            std::cout << "Error al enviar ping" << std::endl;
        }

        if ( generarLogPing )
        {
            GLog::writeSimple(trama, false);
        }
    }

    if ( generarLogPing )
    {
        std::cout << "Cerrando archivo de Logs de pings" << std::endl;
        GLog::close();
    }

    std::cout << "Thread generador de pings finalizado" << std::endl;
}
