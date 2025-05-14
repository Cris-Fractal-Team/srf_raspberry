

#include <iostream>
#include "app/GeneradorEventos.h"
#include "lib/web/GHttpClient.h"

/**
 * Trama que se solicita enviar perteneciente a una persona identificada
 */
void GeneradorEventos::agregarTramaIden( string trama )
{
    mtxBloquea();
    lstTramasPendIden.add(trama);
    mtxLibera();
}

/**
 * Trama que se solicita enviar perteneciente a una persona NO identificada
 */
void GeneradorEventos::agregarTramaNoIden( string trama )
{
    mtxBloquea();
    lstTramasPendNoIden.add(trama);
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

    cout << "Thread Generador Eventos iniciado" << endl;
    while( isFinalizado() == false )
    {
        if ( hayEventosPend() == false )
        {
            sleepMS(5);
            continue;
        }

        if ( lstTramasPendIden.size() > 0 ) 
        {
            trama = lstTramasPendIden.get(0);
            urlServidor = urlServidorIden;
        }            
        else 
        {
            trama = lstTramasPendNoIden.get(0);
            urlServidor = urlServidorNoIden;
        }

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type","application/json");
        // cout << "Trama JSON:" << trama << endl;
        i = httpClient.doHttp(urlServidor, "POST", (char *)trama.c_str(), trama.length());
        if ( i == 0 )
        {
            // exito en la transferencia se saca de la cola
            if ( lstTramasPendIden.size() > 0 )  lstTramasPendIden.remove(0);
            else lstTramasPendNoIden.remove(0);            
        }
    }

    cout << "Thread generador de eventos finalizado" << endl;
}
