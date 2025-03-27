

#include "lib/general/GVector.h"
#include <time.h>
#include <string>

#ifndef GDATE_H
#define GDATE_H

using namespace std;

/**
 * Representa una fecha
 */ 
class GDate : public GObject
{
    public:

        /**
         * Constructor.
         * Toma la hora local del momento de la creacion
         */
        GDate();

        /**
         * Destructor
         */
        ~GDate();

        /**
         * Retorna el nombre del tipo de dato almacenado
         */
        virtual string getType();

        /**
         * Retorna la fecha en formato de timestamp
         */
        time_t toTimeStamp();

        /**
         * Retorna el anio
         */
        int getYear();

        /**
         * Retorna el mes, valor entre 1 y 12
         */
        int getMonth();

        /**
         * Retorna el dia, valor entre 1 y 31
         */
        int getDay();

        /**
         * Retorna la hora del dia, valor entre 0 y 23
         */
        int getHour();

        /**
         * Retorna los minutos de la fecha
         */
        int getMinute();

        /**
         * Retorna los segundos
         */
        int getSecond();

        /**
         * Establece los datos de la fecha sin cambiar los de la hora
         * Si un valor es -1, no se cambia ese valor
         */
        void setDateValue( int year, int month, int day );

        /**
         * Establece el valor de la hora.
         * Si un valor es -1, no se cambia ese valor
         */
        void setTimeValue( int hour, int minut, int secod );

        /**
         * Asigna el valor desde un formato de hora
         */
        void setTime( time_t time);

        /**
         * Pone la hora a 0:00:00
         */
        void resetTime();

        /**
         * Agrega valores a la fecha.
         * Puede ser un valor negativo     
         */
        void addDate( int year, int month, int day );

        /**
         * Agrega valores a la hora
         * Puede ser un valor negativo
         */
        void addTime( int hour, int minute, int second );

        /**
         * Retorna la diferencia en segundos de la fecha actual en segundos
         */
        double diff( GDate date );

        /**
         * Hace que la fecha ponga sus datos a la hora actual
         */
        void toCurrentTime();

        /**
         * Retorna una fecha copia
         */
        GDate getClone();

        /**
         * Retorna el dia de la semana, valor entre 0 y 6
         */
        int getWeekDay();

        /**
         * Copara una fecha con otra.
         * Retorna valor < 0 en caso la fecha sea menor al parametro, 0 en caso sean iguales, > 0 en caso la fecha sea mayor al parametro
         */
        int compareTo( GDate fecha );

        /**
         * Retorna la fecha en formato: yyyy-mm-dd h24:mm:ss
         */
        string toString();

        /**
         * Convierte la fecha a formato dd-mm-yyyy
         *  separator: separador entre los elementos de la fecha
         */
        string toStringDMY( string separator );

        /**
         * Convierte la fecha a formato dd-mm-yyyy hh:mm:ss
         *  separator: separador entre los elementos de la fecha
         */
        string toStringDMYHmS( string separator );

        /**
         * Convierte la fecha a formato mm-dd-yyyy
         *  separator: separador entre los elementos de la fecha
         */
        string toStringMDY( string separator );

        /**
         * Convierte la fecha a formato mm-dd-yyyy hh:mm:ss
         *  separator: separador entre los elementos de la fecha
         */
        string toStringMDYHmS( string separator );

    private:

        /**
         * Estructura formateada de la fecha
         */
        tm *valorInterno;
};

#endif

