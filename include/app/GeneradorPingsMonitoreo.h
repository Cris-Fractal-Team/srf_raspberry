#ifndef _GENERADOR_PINGS_MONITOREO_
#define _GENERADOR_PINGS_MONITOREO_

#include <chrono>
#include <string>
using std::string;

#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"
#include "app/LectorConfig.h"

class ProcesoRecFacial;

/**
 * Clase que encola los pings que se deben enviar al servidor
 * web y los envia en paralelo, además de consultar el endpoint
 * de monitoreo para tareas programadas.
 */
class GeneradorPingsMonitoreo : public GThread
{
public:

    /**
     * Path del log de pings
     */
    string pathLogPing;

    /**
     * Indica si se debe o no generar el log de pings
     */
    bool generarLogPing = false;

    /**
     * Indica si se usa un endpoint unificado (por compatibilidad)
     */
    bool usarEndpointUnificado = true;

    /**
     * URL al que se envian las actualizaciones de ping (POST de eventos)
     * Ej: http://localhost:5250/api/eventos/ping
     */
    string urlPing;

    /**
     * URL base de la app web (para GET /api/ping/log/{serie})
     * Ej: http://localhost:5250
     */
    string dotnetUrl;

    /**
     * Serie del equipo que se usa en la URL:
     *   /api/ping/log/{serie}
     */
    string idEquipo;

    string dotnetEndpointMonitoreo;

    /**
     * Intervalo (ms) del ping de monitoreo
     */
    long pingIntervalMs = 5000;

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

    /**
     * Proceso de Reconocimiento facial de los Archivos.
     */
    ProcesoRecFacial* procRecFacial = nullptr;

    /**
     * Método de configuración de parámetros
     */
    void configure() {
        const auto cfg = LectorConfig::getInstance().getParams();
        dotnetUrl               = cfg->getString("dotnetUrl");
        dotnetEndpointMonitoreo = cfg->getString("dotnetEndpointMonitoreo");

        generarLogPing = cfg->getStringBool("generarLogPingMonitoreo", false);
        pathLogPing    = cfg->getString("pathLogPingMonitoreo");
        pingIntervalMs = cfg->getStringLong("pingIntervalMsMonitoreo", 5000);
    }

private:

    /**
     * Lista de tramas pendientes de enviar de personas identificadas
     */
    GLinkedList<string> lstTramasPendIden;

    /**
     * Lista de tramas pendientes de enviar de personas no identificadas
     */
    GLinkedList<string> lstTramasPendNoIden;

    std::chrono::steady_clock::time_point ultimoPingTp = std::chrono::steady_clock::now();

    /**
     * ID de la tarea programada actual (se inicializa en 0)
     * Se actualiza cuando tipoDeTarea == 1 en la respuesta del GET
     */
    int idTareaProgramada = 0;

    /**
     * Consulta el endpoint de monitoreo:
     *   GET /api/ping/log/{serie}?idTareaProgramada=x
     * y procesa el JSON de respuesta.
     */
    void consultarTareasProgramadas();

    /**
     * Procesa el tipo de tarea que llega en el JSON
     * y permite extender comportamientos via switch.
     */
    void procesarTareaProgramada(int tipoDeTarea, int idTareaJson);
};

#endif
