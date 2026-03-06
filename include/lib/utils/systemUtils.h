

#ifndef _SYSTEM_UTILS_
#define _SYSTEM_UTILS_

#include <string>

/**
 * Clase con servicios asociados a informacion del sistema
 */
class SystemUtils
{
    public:

        /**
         * Retorna el numero serial de
         */
        static std::string getRaspberryPiSerial();

        /**
         * Imprime en la consola el uso de RAM
         */
        static void printRamUsage();
};

#endif