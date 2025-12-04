#ifndef _PROC_DESCARGA_DESC_FACIALES_
#define _PROC_DESCARGA_DESC_FACIALES_

#include <string>

#include "app/ExtractorFacialArchivo.h"

class ProcDescargaDescFaciales {
   public:
   /**
    * Nro de serie del equipo
    */
    std::string idEquipo;

    /**
     * URL base de la app web, por ejemplo:
     *   http://localhost:8080
     */
    std::string appWebUrl;

    /**
     * URL base del backend dotnet, por ejemplo:
     *   http://localhost:8080
     */
    std::string dotnetUrl;

    /**
     * Endpoint para marcar como completas las tareas:
     */
    std::string endpointCompletado;

    /**
     * Endpoint de descarga del ZIP, por ejemplo:
     *   /srf/sensor/descargarZip?idEmpresa=3
     * Si viene vacío y ya tienes una URL completa, puedes pasarla toda en
     * appWebUrl.
     */
    std::string endpointDescargaZip;

    /**
     * Endpoint de autenticacion, por ejemplo:
     *   /srf/auth/login
     * Si viene vacío y ya tienes una URL completa, puedes pasarla toda en
     * appWebUrl.
     */
    std::string endpointAuth;

    /*
     *Directorio de extracción de archivos
     */
    std::string directorioExtraccion;

    /*
     *Directorio de extracción de backup o versiones
     */
    std::string directorioBackup;

    /*
     *Archivo indice para la generacion de descriptores faciales
     */
    std::string archivoIndice;

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

    /**
     * Extractor facial de los Archivos "datos.txt".
     */
    ExtractorFacialArchivo extractor;

    /**
     *Credenciales para iniciar sesion
     */
    std::string username;
    std::string password;

    /**
     * Id tarea programada para completar
     */
    int idTareaProgramada = -1;

   private:
    /**
     * Construye la URL completa de descarga.
     */
    std::string construirUrlDescarga() const;

    /**
     * Ejecuta el script de shell con la URL como argumento.
     */
    bool ejecutarScriptShell(const std::string& urlDescarga) const;

    /**
     * Ejecutar script de descarga de rostros
     */
    bool descargarZipRostros(const std::string& downloadUrl,
                             const std::string& datasetDirectory);
    /**
     * Ejecutar procedimiento para generar descriptores faciales
     */
    bool generarDescriptoresFaciales(const std::string& datasetDirectory);

    /**
     * Ejecutar procedimiento para guardar una copia de seguridad de los descriptores faciales
     */
    void crearCopiaSeguridadDescriptores(const std::string& datasetDirectory);

    /**
     * Ejecutar script para limpiar archivos utilizados para generar descriptores faciales
     */
    void limpiarArchivosRostros(const std::string& datasetDirectory);

    /**
     * Inicio de sesión para obtener el token
     */
    void iniciarSesion();

    /**
     * Marcar un tarea como completada
     */
    void notificarTareaCompletada();

    std::string token;
};

#endif
