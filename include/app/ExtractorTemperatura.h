#pragma once

#include <string>

/**
 * Entidad para extraer temperatura del dispositivo (CPU) desde sysfs.
 * No crea hilos. Se invoca bajo demanda desde quien necesite el dato.
 */
class ExtractorTemperatura
{
public:
    ExtractorTemperatura();

    /**
     * Permite redefinir el path sysfs si tu dispositivo usa otro.
     * Default: /sys/class/thermal/thermal_zone0/temp
     */
    void setPathTempCpu(const std::string& path);

    /**
     * Obtiene temperatura CPU en °C.
     * Retorna true si pudo leer; false si no.
     */
    bool obtenerTempCpuC(double& outTempC);

    /**
     * Obtiene temperatura CPU en miligrados (tal cual sysfs).
     * Retorna true si pudo leer; false si no.
     */
    bool obtenerTempCpuMilli(long& outTempMilli);

    /**
     * Obtiene el valor de la temperatura
     */
    double getTempCpuC(bool* ok = nullptr);

    /**
     * Método de configuración de parámetros
     */
    void configure();

private:
    std::string pathTempCpu;
};
