#ifndef _GENERADOR_EVENTOS_
#define _GENERADOR_EVENTOS_

#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"
#include <atomic>
#include <string>

using std::string;

/**
 * Clase que encola los eventos que se deben enviar al servidor web y los envía en un thread.
 *
 * - Mantiene 2 colas principales: IDENT y NO_IDENT.
 * - Mantiene 2 colas DEAD (reintento final): IDENT y NO_IDENT.
 * - Regla: se procesa DEAD solo cuando ambas colas principales están vacías.
 * - Regla: si falla un envío desde MAIN, pasa a DEAD (si hay espacio). Si falla desde DEAD, se descarta.
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
         * URL al que se envían los datos correspondientes a personas identificadas
         */
        string urlServidorIden;

        /**
         * URL al que se envían los datos correspondientes a personas no identificadas
         */
        string urlServidorNoIden;

        /**
         * URL al que se envían los datos de las personas reconocidas o no identificadas de forma unificada
         */
        string urlServidorUnificado;

        /**
         * Encola una trama perteneciente a una persona identificada
         *
         * @param trama Trama en JSON
         */
        void agregarTramaIden(string trama);

        /**
         * Encola una trama perteneciente a una persona no identificada
         *
         * @param trama Trama en JSON
         */
        void agregarTramaNoIden(string trama);

        /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Valida si hay eventos pendientes de ser procesados (incluye DEAD)
         *
         * @return true si hay eventos pendientes
         */
        bool hayEventosPend();

        /**
         * Setter/Getter de habilitado
         */
        void setEnabled(bool v) { enabled.store(v); }
        bool isEnabled() const { return enabled.load(); }

        /**
         * Configura límites de colas desde configuración
         */
        void configure();

        /**
         * Obtiene el tamaño de colas principales (para monitoreo)
         */
        size_t getPendIden() const { return (size_t)lstTramasPendIden.size(); }
        size_t getPendNoIden() const { return (size_t)lstTramasPendNoIden.size(); }

        /**
         * Obtiene el tamaño de colas DEAD (para monitoreo)
         */
        size_t getDeadIden() const { return (size_t)lstDeadIden.size(); }
        size_t getDeadNoIden() const { return (size_t)lstDeadNoIden.size(); }

    private:
        /**
         * Tipos de evento
         */
        enum class TipoEvento { Ident, NoIdent };

        /**
         * Fuente de cola
         */
        enum class FuenteCola { Main, Dead };

        /**
         * Elemento que se envía
         */
        struct EventoPop
        {
            string trama;
            string url;
            TipoEvento tipo;
            FuenteCola fuente;
            size_t pendI = 0, pendN = 0, deadI = 0, deadN = 0;
        };

        /**
         * Resultado de envío
         */
        struct SendResult
        {
            int code = -1;
            int httpStatus = -1;
            long durMs = 0;
        };

        /**
         * Habilitado para flujo
         */
        std::atomic<bool> enabled{true};

        /**
         * Cola principal de identificados
         */
        GLinkedList<string> lstTramasPendIden;

        /**
         * Cola principal de no identificados
         */
        GLinkedList<string> lstTramasPendNoIden;

        /**
         * Cola DEAD de identificados (reintento final)
         */
        GLinkedList<string> lstDeadIden;

        /**
         * Cola DEAD de no identificados (reintento final)
         */
        GLinkedList<string> lstDeadNoIden;

        /**
         * Límite de cola para identificados (aplica a MAIN y DEAD)
         */
        size_t maxQueueIden = 120;

        /**
         * Límite de cola para no identificados (aplica a MAIN y DEAD)
         */
        size_t maxQueueNoIden = 80;

        /**
         * Valida si puede encolar en MAIN (ident)
         */
        bool canEnqueueIden() const { return (size_t)lstTramasPendIden.size() < maxQueueIden; }

        /**
         * Valida si puede encolar en MAIN (no ident)
         */
        bool canEnqueueNoIden() const { return (size_t)lstTramasPendNoIden.size() < maxQueueNoIden; }

        /**
         * Obtiene URL a usar según tipo
         *
         * @param tipo Tipo de evento
         * @return URL destino
         */
        string resolveUrl(TipoEvento tipo) const;

        /**
         * Intenta extraer (POP) el siguiente evento:
         * - Prioriza MAIN.
         * - Si MAIN está vacía, procesa DEAD.
         *
         * @param out Evento extraído
         * @return true si se obtuvo evento
         */
        bool tryPopNext(EventoPop& out);

        /**
         * Envía trama por HTTP
         *
         * @param httpClient Cliente HTTP
         * @param ev Evento a enviar
         * @return Resultado del envío
         */
        SendResult sendTrama(class GHttpClient& httpClient, const EventoPop& ev);

        /**
         * Registra log de dequeue
         *
         * @param ev Evento extraído
         */
        void logDequeue(const EventoPop& ev) const;

        /**
         * Maneja envío exitoso
         *
         * @param ev Evento enviado
         * @param sr Resultado del envío
         */
        void onSendOk(const EventoPop& ev, const SendResult& sr) const;

        /**
         * Intenta encolar en DEAD (reintento final)
         *
         * @param trama Trama a encolar (move)
         * @param tipo Tipo de evento
         * @return true si se encoló
         */
        bool tryEnqueueDead(string&& trama, TipoEvento tipo);

        /**
         * Maneja envío fallido:
         * - Si viene de MAIN, intenta mover a DEAD.
         * - Si viene de DEAD, descarta.
         *
         * @param ev Evento fallido
         * @param sr Resultado del envío
         */
        void onSendFail(EventoPop& ev, const SendResult& sr);
};

#endif
