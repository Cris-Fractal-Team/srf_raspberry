#include "app/ExtractorTemperatura.h"
#include "app/LectorConfig.h"

#include <chrono>
#include <fstream>
#include <sstream>

#include "lib/utils/Logger.h"

static constexpr const char* LOG_COMPONENT = "ExtractorTemperatura";

ExtractorTemperatura::ExtractorTemperatura()
{
    pathTempCpu = "/sys/class/thermal/thermal_zone0/temp";
}

void ExtractorTemperatura::configure()
{
    const auto cfg = LectorConfig::getInstance().getParams();
    string pathTemperatura = cfg->getString("pathTemperatura");
    setPathTempCpu(pathTemperatura);
}

void ExtractorTemperatura::setPathTempCpu(const std::string& path)
{
    pathTempCpu = path;
    LOG_INFO(LOG_COMPONENT, "Path temp CPU configurado: " << pathTempCpu);
}

bool ExtractorTemperatura::obtenerTempCpuMilli(long& outTempMilli)
{
    auto tInicio = std::chrono::high_resolution_clock::now();

    std::ifstream f(pathTempCpu);
    if (!f.is_open())
    {
        LOG_ERROR(LOG_COMPONENT,
                  "No se pudo abrir archivo de temperatura: " << pathTempCpu);
        return false;
    }

    long milli = 0;
    f >> milli;

    if (!f.good())
    {
        LOG_ERROR(LOG_COMPONENT,
                  "No se pudo leer temperatura (contenido invalido). Path="
                      << pathTempCpu);
        return false;
    }

    outTempMilli = milli;

    auto tFin = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(tFin - tInicio);

    LOG_DEBUG(LOG_COMPONENT,
              "Temp leida OK: " << outTempMilli << " (mC) T=" << dur.count() << "us");

    return true;
}

bool ExtractorTemperatura::obtenerTempCpuC(double& outTempC)
{
    auto tInicio = std::chrono::high_resolution_clock::now();

    long milli = 0;
    if (!obtenerTempCpuMilli(milli))
    {
        return false;
    }

    outTempC = static_cast<double>(milli) / 1000.0;

    auto tFin = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(tFin - tInicio);

    LOG_DEBUG(LOG_COMPONENT,
              "Temp CPU: " << outTempC << " C"
                           << " (raw=" << milli << " mC)"
                           << " T=" << dur.count() << "us");

    return true;
}

double ExtractorTemperatura::getTempCpuC(bool* ok)
{
    double tempC = 0.0;
    bool r = obtenerTempCpuC(tempC);

    if (ok) *ok = r;

    if (!r)
        return std::numeric_limits<double>::quiet_NaN(); // ✅ valor “inválido” si falló

    return tempC; // ✅ valor numérico listo para usar
}