

#include <thread>
#include <mutex>
#include <chrono>

#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

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


/**
 * Retorna un string que formato yyyy-mm-dd HH:ii:ss 
 * de una hora en milisegundos
 */
std::string  TimeDateUtils::getFechaDesdeMs(long long millis) 
{
    std::time_t segundos = millis / 1000;
    std::tm* tiempo = std::localtime(&segundos);  // Usa std::gmtime para UTC

    std::ostringstream oss;
    oss << std::put_time(tiempo, "%Y-%m-%d %H:%M:%S");

    // Si deseas incluir milisegundos:
    int ms = millis % 1000;
    oss << "." << std::setw(3) << std::setfill('0') << ms;

    return oss.str();
}