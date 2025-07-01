

#ifndef _MAKVAL_TIME_DATE_
#define _MAKVAL_TIME_DATE_

#include <string>


class TimeDateUtils
{
    public:

        /**
         * Retorna la fecha y hora en milisegundos
         */
        static long long getDateTimeMs();


        /**
         * Retorna un string que formato yyyy-mm-dd HH:ii:ss 
         * de una hora en milisegundos
         */
        static std::string getFechaDesdeMs(long long millis);
};

#endif