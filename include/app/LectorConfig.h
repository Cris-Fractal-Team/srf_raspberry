#ifndef LECTOR_CONFIG_H
#define LECTOR_CONFIG_H

/**
 * @file LectorConfig.h
 * @brief Singleton global para acceder a parámetros de configuración usando GHashMap como backend.
 *
 * ## Objetivo
 * Centralizar el acceso a la configuración del sistema (archivo config.txt, parámetros del App Web, etc.)
 * en **una sola instancia global** accesible desde cualquier clase, sin depender de que `ProcesoRecFacial`
 * sea el dueño de `lstParamsApp`.
 *
 * ## Principio clave
 * - **NO se reimplementa** la lectura del archivo.
 * - Se reutiliza la API ya existente:
 *   - `leeArchivoConfig(path, convierteNombresLowerCase)`  -> retorna `shared_ptr<GHashMap>`
 *   - `guardaArchivoConfig(path, lstParams)`               -> guarda el hashmap a disco
 * - El `GHashMap` ya contiene la lógica de:
 *   - valores por defecto para números/bool (ej. getStringLong/getStringDouble/getStringBool)
 *   - validación de existencia (hasKey)
 *
 * ## Formato del archivo de configuración (según tu API existente)
 * Archivo de texto simple:
 * - Comentarios comienzan con `#` al inicio de la línea.
 * - Parámetros:
 *     nombre = valor
 *
 * ## Concurrencia (multihilo)
 * Este Singleton puede ser usado por varios hilos (pings, eventos, servidor HTTP, etc.).
 * Por eso se protege el acceso al puntero `shared_ptr<GHashMap>` y su modificación con un mutex:
 * - Lecturas: getters
 * - Escrituras: initFromFile/reload/applyOverrides/setParams/save
 *
 * Si tu aplicación nunca modifica la config en runtime, igualmente es seguro dejar el mutex
 * para prevenir errores futuros cuando se añadan overrides dinámicos.
 */

#include <memory>
#include <mutex>
#include <string>

#include "lib/general/GHashMap.h"
#include "lib/utils/fileutils.h"

/**
 * @class LectorConfig
 * @brief Singleton global de configuración.
 *
 * Esta clase actúa como:
 * 1) **Contenedor global** de todos los parámetros (en un `GHashMap`).
 * 2) **Facade** para obtener valores de manera más amigable:
 *    - `getString(key, default)`
 *    - `getStringLong(key, default)`
 *    - `getStringBool(key, default)` ...
 * 3) **Punto único** para aplicar "overrides" (por ejemplo, parámetros que llegan desde el App Web),
 *    sin que una sola clase (como `ProcesoRecFacial`) sea la dueña del mapa.
 *
 * ### Flujo recomendado
 * - En `main()`:
 *   1) `LectorConfig::getInstance().initFromFile("./config/config.txt");`
 * - En cualquier clase:
 *   - `auto& cfg = LectorConfig::getInstance();`
 *   - `auto tol = cfg.getStringDouble("deltaRostroMax", 0.60);`
 *
 * ### Compatibilidad con tu código actual
 * Si aún tienes partes del sistema que usan `shared_ptr<GHashMap> lstParamsApp`,
 * puedes hacer:
 *   - `proc.lstParamsApp = LectorConfig::getInstance().getParams();`
 * y migrar poco a poco al facade.
 */
class LectorConfig
{
public:
    /**
     * @brief Retorna la instancia única del Singleton.
     *
     * Implementación tipo "Meyers Singleton":
     * - Thread-safe desde C++11 para la inicialización estática.
     * - Evita fugas y problemas de orden de destrucción en la mayoría de casos.
     */
    static LectorConfig& getInstance();

    /**
     * @brief Inicializa el mapa de configuración desde un archivo.
     *
     * Internamente llama a `leeArchivoConfig(path, convierteNombresLowerCase)`.
     *
     * @param path Ruta del archivo de configuración (ej: "./config/config.txt")
     * @param convierteNombresLowerCase Si es true, convierte los nombres de los parámetros a lowercase
     *                                 (útil para búsquedas case-insensitive).
     *
     * @note Si el archivo no existe o hay error, la instancia queda con un `GHashMap` vacío.
     * @note Se recomienda llamar a este método una vez al inicio del programa (main).
     */
    void initFromFile(const std::string& path, bool convierteNombresLowerCase = false);

    /**
     * @brief Recarga la configuración desde el mismo archivo con el que se inicializó.
     *
     * Útil si deseas refrescar parámetros sin reiniciar el programa.
     *
     * @note Si no se llamó antes a initFromFile() o si no hay path, no hace nada.
     */
    void reload();

    /**
     * @brief Guarda la configuración actual a disco.
     *
     * Internamente llama a `guardaArchivoConfig(...)`.
     *
     * @param path Si está vacío, guarda usando el path usado en initFromFile().
     *             Si no está vacío, guarda en la ruta indicada.
     *
     * @note Si no hay path (ni inicializado ni pasado), no hace nada.
     */
    void save(const std::string& path = "");

    /**
     * @brief Retorna el `shared_ptr<GHashMap>` actual.
     *
     * Esto sirve para:
     * - Compatibilidad con código existente que espera `lstParamsApp`.
     * - Pasar el mapa a funciones que ya reciben `shared_ptr<GHashMap>`.
     *
     * @return shared_ptr<GHashMap> del estado actual.
     */
    std::shared_ptr<GHashMap> getParams() const;

    /**
     * @brief Reemplaza completamente el mapa de parámetros actual.
     *
     * @param params Nuevo mapa (si es nullptr, se usará un GHashMap vacío).
     *
     * @warning Al reemplazar, todas las referencias anteriores al mapa viejo
     *          seguirán apuntando al mapa anterior (si alguien lo guardó).
     *          Si quieres consistencia global, usa getParams() siempre que sea posible.
     */
    void setParams(const std::shared_ptr<GHashMap>& params);

    /**
     * @brief Aplica "overrides" encima del mapa actual.
     *
     * Típico para parámetros recibidos desde el App Web en runtime.
     * Para cada clave en `overrides`, se sobrescribe el valor en el mapa actual.
     *
     * @param overrides HashMap con las claves/valores a sobrescribir.
     *
     * @note Este método NO modifica el archivo automáticamente.
     *       Si deseas persistir, llama a save().
     */
    void applyOverrides(const std::shared_ptr<GHashMap>& overrides);

    // -------------------------------------------------------------------------
    // Facade: getters amigables
    // -------------------------------------------------------------------------

    /**
     * @brief Verifica si existe la clave.
     * @param key Nombre del parámetro.
     * @return true si existe, false si no.
     */
    bool hasKey(const std::string& key) const;

    /**
     * @brief Obtiene un string con valor por defecto.
     *
     * `GHashMap::getString()` retorna "" si no existe o no es string,
     * por eso este método aplica `def` si:
     * - la clave no existe, o
     * - el valor obtenido es vacío.
     *
     * @param key Nombre del parámetro.
     * @param def Valor por defecto.
     * @return Valor del parámetro o `def`.
     */
    std::string getString(const std::string& key, const std::string& def = "") const;

    /**
     * @brief Obtiene un int almacenado como int dentro del hashmap (putInt/getInt).
     * @param key Nombre del parámetro.
     * @param def Valor por defecto.
     * @return Valor o def.
     */
    int getInt(const std::string& key, int def) const;

    /**
     * @brief Obtiene un long almacenado como long dentro del hashmap (putLong/getLong).
     * @param key Nombre del parámetro.
     * @param def Valor por defecto.
     * @return Valor o def.
     */
    long getLong(const std::string& key, long def) const;

    /**
     * @brief Obtiene un long long almacenado como longlong dentro del hashmap (putLongLong/getLongLong).
     * @param key Nombre del parámetro.
     * @param def Valor por defecto.
     * @return Valor o def.
     */
    long long getLongLong(const std::string& key, long long def) const;

    /**
     * @brief Obtiene un double almacenado como double dentro del hashmap (putDouble/getDouble).
     * @param key Nombre del parámetro.
     * @param def Valor por defecto.
     * @return Valor o def.
     */
    double getDouble(const std::string& key, double def) const;

    /**
     * @brief Obtiene un long desde un parámetro almacenado como string.
     *
     * Internamente usa `GHashMap::getStringLong(key, def)`, que:
     * - verifica existencia
     * - convierte `stol`
     * - retorna def en caso de error
     */
    long getStringLong(const std::string& key, long def) const;

    /**
     * @brief Obtiene un double desde un parámetro almacenado como string.
     *
     * Internamente usa `GHashMap::getStringDouble(key, def)`, que:
     * - verifica existencia
     * - convierte `stod`
     * - retorna def en caso de error
     */
    double getStringDouble(const std::string& key, double def) const;

    /**
     * @brief Obtiene un bool desde un parámetro almacenado como string.
     *
     * Internamente usa `GHashMap::getStringBool(key, def)`.
     *
     * Valores reconocidos (lowercase):
     * - true:  "t", "true", "si", "s"
     * - false: "f", "false", "no", "n"
     *
     * @note Si el string no coincide, `GHashMap` imprime un mensaje y retorna def.
     */
    bool getStringBool(const std::string& key, bool def) const;

    // -------------------------------------------------------------------------
    // Estado / info
    // -------------------------------------------------------------------------

    /**
     * @brief Retorna la ruta del archivo de configuración cargado.
     */
    std::string getPath() const;

    /**
     * @brief Indica si el singleton fue inicializado al menos una vez.
     */
    bool isInitialized() const;

private:
    LectorConfig() = default;
    LectorConfig(const LectorConfig&) = delete;
    LectorConfig& operator=(const LectorConfig&) = delete;

private:
    // Mutex para proteger acceso a params_ / path_ en escenarios multihilo.
    mutable std::mutex mtx_;

    // Backend real: aquí vive todo el config. Es el reemplazo global de proc.lstParamsApp.
    std::shared_ptr<GHashMap> params_{std::make_shared<GHashMap>()};

    // Ruta del archivo (para reload/save)
    std::string path_;

    // Flag de lower-case (para reload consistente)
    bool convierteNombresLowerCase_ = false;

    // Flag de inicialización
    bool initialized_ = false;

private:
    /**
     * @brief Mezcla todas las claves de src dentro de target (override).
     *
     * Copia las claves de `src->getLstClaves()` y hace:
     * - `target->put(k, src->get(k))`
     *
     * Eso preserva los tipos (string/int/double/long...) ya que se copian los GObject.
     */
    static void mergeInto(std::shared_ptr<GHashMap>& target,
                          const std::shared_ptr<GHashMap>& src);
};

#endif