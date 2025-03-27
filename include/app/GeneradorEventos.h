
#ifndef _GENERADOR_EVENTOS_
#define _GENREADOR_EVENTOS_

#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h""


/**
 * Clase que encola los eventos que se deben enviar al servidor
 * web y los envia en paralelo
 */
class GeneradorEventos : public GThread
{
    public:

        /** 
         * URL al que se envian los datos
         */
        string urlServidor;

        /**
         * Trama que se solicita enviar
         */
        void agregarTrama( string trama );

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
         * Lista de tramas pendientes de enviar
         */
        GLinkedList<string> lstTramasPend;
};

#endif