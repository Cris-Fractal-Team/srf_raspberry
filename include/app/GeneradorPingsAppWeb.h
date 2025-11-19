#ifndef _GENERADOR_PINGS_APP_WEB_
#define _GENERADOR_PINGS_APP_WEB_

#include <chrono>
#include <string>

using std::string;

#include "lib/general/GThread.h"

/**
 * Clase que genera pings periódicos (GET) contra la app web
 */
class GeneradorPingsAppWeb : public GThread
{
public:

    /**
     * URL de la app web a la que se hará GET
     * Ej: http://localhost:3000/health
     */
    string appWebUrl;

    /**
     * Path del log de pings de la app web
     */
    string pathLogPing;

    /**
     * Indica si se debe o no generar log de pings
     */
    bool generarLogPing = false;

    /**
     * Intervalo entre pings en milisegundos
     */
    long pingIntervalMs = 5000; // default 5s

    /**
     * Bucle del thread
     */
    void runThread() override;

private:
    std::chrono::steady_clock::time_point ultimoPingTp = std::chrono::steady_clock::now();
};

#endif
