

#include <iostream>
#include "app/GeneradorEventos.h"
#include "lib/web/GHttpClient.h"

/**
 * Trama que se solicita enviar
 */
void GeneradorEventos::agregarTrama( string trama )
{
    mtxBloquea();
    lstTramasPend.add(trama);
    mtxLibera();
}

/**
 * Valida si hay eventos pendientes de ser procesado
 **/
bool GeneradorEventos::hayEventosPend()
{
    bool rpta;

    mtxBloquea();
    if ( lstTramasPend.size() > 0 ) rpta=true;
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
    int i;

    cout << "Thread Generador Eventos iniciado" << endl;
    while( isFinalizado() == false )
    {
        if ( hayEventosPend() == false )
        {
            sleepMS(5);
            continue;
        }

        trama = lstTramasPend.get(0);

        GHttpClient httpClient;
        httpClient.setHeader("Content-Type","application/json");
        // cout << "Trama JSON:" << trama << endl;
        i = httpClient.doHttp(urlServidor, "POST", (char *)trama.c_str(), trama.length());
        if ( i == 0 )
        {
            // exito en la transferencia se saca de la cola
            lstTramasPend.remove(0);
        }
    }

    cout << "Thread generador de eventos finalizado" << endl;
}
