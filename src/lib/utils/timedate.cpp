

#include <thread>
#include <mutex>
#include <chrono>

#include "lib/utils/timedate.h"


/**
 * Retorna la fecha y hora en milisegundos
 */
long long TimeDateUtils::getDateTimeMs()
{
    auto now = std::chrono::system_clock::now();
    auto ahora = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = ahora.time_since_epoch();

    return (long long)epoch.count();
}