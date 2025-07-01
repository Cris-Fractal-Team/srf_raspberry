

#include <iostream>
#include "app/GeneradorEventos.h"
#include "lib/web/GHttpClient.h"
#include "lib/utils/GLog.h"

/**
 * Trama que se solicita enviar perteneciente a una persona identificada
 */
void GeneradorEventos::agregarTramaIden( string trama )
{
    mtxBloquea();
    lstTramasPendIden.add(trama);
    cout << "Num tramas identificadas pendientes " << lstTramasPendIden.size() << endl;
    mtxLibera();
}

/**
 * Trama que se solicita enviar perteneciente a una persona NO identificada
 */
void GeneradorEventos::agregarTramaNoIden( string trama )
{
    mtxBloquea();
    lstTramasPendNoIden.add(trama);
    cout << "Num tramas NO identificadas pendientes " << lstTramasPendNoIden.size() << endl;
    mtxLibera();
}

/**
 * Valida si hay eventos pendientes de ser procesado
 **/
bool GeneradorEventos::hayEventosPend()
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
void GeneradorEventos::runThread()
{
    string trama;    
    string urlServidor;
    int i;

    if ( generarLogEventos == true )
    {
        cout << "Creando archivo de logs " << endl;
        GLog::open(pathLogEventos);
    }
    
    cout << "Thread Generador Eventos iniciado" << endl;
    while( isFinalizado() == false )
    {
        auto inicioBucle = std::chrono::high_resolution_clock::now();
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
            if ( usarEndpointUnificado == false ) urlServidor = urlServidorIden;
            else urlServidor = urlServidorUnificado;
        }            
        else 
        {
            trama = lstTramasPendNoIden.get(0);
            if ( usarEndpointUnificado == false )  urlServidor = urlServidorNoIden;
            else urlServidor = urlServidorUnificado;
        }
        mtxLibera();

        auto inicioHttp = std::chrono::high_resolution_clock::now();
        GHttpClient httpClient;
        httpClient.setHeader("Content-Type","application/json");
        // cout << "Trama JSON:" << trama << endl;        
        i = httpClient.doHttp(urlServidor, "POST", (char *)trama.c_str(), trama.length());

        auto finHttp = std::chrono::high_resolution_clock::now();

        auto durInicio = std::chrono::duration_cast<std::chrono::microseconds>(obtieneTrama - inicioBucle);
        auto durTrama = std::chrono::duration_cast<std::chrono::microseconds>(inicioHttp - obtieneTrama);
        auto durHttp = std::chrono::duration_cast<std::chrono::milliseconds>(finHttp - inicioHttp);
        
        if ( i == 0 )
        {
            // exito en la transferencia se saca de la cola
            mtxBloquea();
            if ( lstTramasPendIden.size() > 0 )  
            {
                lstTramasPendIden.remove(0);
                cout << "Eventos Identificados pendientes " <<  lstTramasPendIden.size() << " T.Inicio " << durInicio.count()  <<  " mcs T.Trama " << durTrama.count() << " mcs T.Http " << durHttp.count() << " ms " << endl;
            }
            else 
            {
                lstTramasPendNoIden.remove(0);    
                cout << "Eventos No Identificados pendientes " <<  lstTramasPendNoIden.size() << " T.Inicio " << durInicio.count()  <<  " mcs T.Trama " << durTrama.count() << " mcs T.Http " << durHttp.count() << " ms " << endl;     
            }            
            mtxLibera();
        }
        else
        {
            cout << "Error al enviar deteccion" << endl;
        }

         GLog::writeSimple(trama, false);
    }

    if ( generarLogEventos )
    {
        cout << "Cerrando archivo de Logs" << endl;
        GLog::close();
    }

    cout << "Thread generador de eventos finalizado" << endl;
}
