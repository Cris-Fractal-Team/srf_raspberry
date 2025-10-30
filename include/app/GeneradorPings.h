#ifndef _GENERADOR_PINGS_
#define _GENERADOR_PINGS_

#include <string>
using std::string;

#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"

/**
 * Clase que encola los pings que se deben enviar al servidor
 * web y los envia en paralelo
 */
class GeneradorPings : public GThread
{
    public:

         /**
         * Path del log de pings
         */
        string pathLogPing;

        /**
         * Indica si se debe o no generar el log de pings
         */
        bool generarLogPing;

        /**
         * Indica si se usa un endpoint unificado
         */
        bool usarEndpointUnificado;

        /** 
         * URL al que se envian las actualizaciones de ping
         */
        string urlPing;

        /**
         * Encola un ping para “identificados”
         */
        void encolarPingIdentificado(string trama);

        /**
         * Encola un ping para “no identificados”
         */
        void encolarPingNoIdentificado(string trama);

        /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Valida si hay eventos pendientes de ser procesado
         **/
        bool hayEventosPend();

    private:

        /**
         * Lista de tramas pendientes de enviar de personas identificadas
         */
        GLinkedList<string> lstTramasPendIden;

        /**
         * Lista de tramas pendientes de enviar de personas no identificadas
         */
        GLinkedList<string> lstTramasPendNoIden;
};

#endif
