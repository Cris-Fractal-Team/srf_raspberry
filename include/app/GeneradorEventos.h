
#ifndef _GENERADOR_EVENTOS_
#define _GENREADOR_EVENTOS_

#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"
#include <atomic>


/**
 * Clase que encola los eventos que se deben enviar al servidor
 * web y los envia en paralelo
 */
class GeneradorEventos : public GThread
{
    public:

         /**
         * Path del log de eventos
         */
        string pathLogEventos;

        /**
         * Indica si se debe o no generar el log de eventos
         */
        bool generarLogEventos;

        /**
         * Indica si se usa un endpoint unificado
         */
        bool usarEndpointUnificado;

        /** 
         * URL al que se envian los datos correspondientes a personas identificadas
         */
        string urlServidorIden;

        /** 
         * URL al que se envian los datos correspondientes a personas NOidentificadas
         */
        string urlServidorNoIden;

        /**
         * URL al que se envian los datos de las personas reconocidas o no identificadas 
         * de forma unificada
         */
        string urlServidorUnificado;

        /**
         * Trama que se solicita enviar perteneciente a una persona identificada
         */
        void agregarTramaIden( string trama );

        /**
         * Trama que se solicita enviar perteneciente a una persona no identificada
         */
        void agregarTramaNoIden( string trama );

        /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Valida si hay eventos pendientes de ser procesado
         **/
        bool hayEventosPend();

        /**
        * Getter y Setter de habilitado
        */
        void setEnabled(bool v) { enabled.store(v); }
        bool isEnabled() const { return enabled.load(); }
    private:

        /**
         * Habilitado para flujo
         */
        std::atomic<bool> enabled{true};
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