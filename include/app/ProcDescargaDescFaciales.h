#ifndef _PROC_DESCARGA_DESC_FACIALES_
#define _PROC_DESCARGA_DESC_FACIALES_

#include <string>

class ProcDescargaDescFaciales {
   public:
    /**
     * URL base de la app web, por ejemplo:
     *   http://localhost:8080
     */
    std::string appWebUrl;

    /**
     * Endpoint de descarga del ZIP, por ejemplo:
     *   /srf/sensor/descargarZip?idEmpresa=3
     * Si viene vacío y ya tienes una URL completa, puedes pasarla toda en
     * appWebUrl.
     */
    std::string endpointDescargaZip;

    /**
     * Ruta al script de shell que hará la descarga y generación.
     * Por defecto: ./descargarRostros.sh
     */
    std::string rutaScriptGenerar = "./descargarRostros.sh";

    /**
     * Indica si se debe o no generar log.
     */
    bool generarLog = false;

    /**
     * Ruta del archivo de log (solo si generarLog = true).
     */
    std::string pathLog;

    /**
     * Ejecuta el flujo:
     *  1) Construye la URL de descarga (appWebUrl + endpointDescargaZip o
     * appWebUrl si ya es completa) 2) Llama al script de shell pasando la URL
     * como parámetro
     *
     * Retorna true si el script termina con código 0.
     */
    bool ejecutarDescargaYGeneracion();

   private:
    /**
     * Construye la URL completa de descarga.
     */
    std::string construirUrlDescarga() const;

    /**
     * Ejecuta el script de shell con la URL como argumento.
     */
    bool ejecutarScriptShell(const std::string& urlDescarga) const;
};

#endif
