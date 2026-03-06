
#include "lib/general/GVector.h"
#include "lib/general/GDate.h"
#include <time.h>
#include <string>

using namespace std;


/**
 * Constructor.
 * Toma la hora local del momento de la creacion
 */
GDate::GDate()
{
    time_t ahora = time(NULL);
    tm *rpta = localtime(&ahora);
    valorInterno = *rpta;
}

/**
 * Destructor
 */
GDate::~GDate()
{
    // if ( valorInterno != NULL )
    // {
    //     free(valorInterno);
    //     valorInterno = NULL;
    // }
}

/**
 * Retorna el nombre del tipo de dato almacenado
 */
string GDate::getType()
{
    return "GDate";    
}

/**
 * Retorna la fecha en formato de timestamp
 */
time_t GDate::toTimeStamp()
{
    return mktime(&valorInterno);
}

/**
 * Retorna el anio
 */
int GDate::getYear()
{
    return valorInterno.tm_year+1900;
}

/**
 * Retorna el mes, valor entre 1 y 12
 */
int GDate::getMonth()
{
    return valorInterno.tm_mon+1;
}

/**
 * Retorna el dia del mes, valor entre 1 y 31
 */
int GDate::getDay()
{
    return valorInterno.tm_mday;
}

/**
 * Retorna la hora del dia, , valor entre 0 y 23
 */
int GDate::getHour()
{
    return valorInterno.tm_hour;
}

/**
 * Retorna los minutos de la fecha
 */
int GDate::getMinute()
{
    return valorInterno.tm_min;
}

/**
 * Retorna los segundos
 */
int GDate::getSecond()
{
    return valorInterno.tm_sec;
}

/**
    * Establece los datos de la fecha sin cambiar los de la hora
    * Si un valor es -1, no se cambia ese valor
    */
void GDate::setDateValue( int year, int month, int day )
{
    if ( year != -1)
        valorInterno.tm_year = year-1900;

    if ( month != -1 )
        valorInterno.tm_mon = month-1;

    if ( day != -1 )
        valorInterno.tm_mday = day;

    mktime(&valorInterno);
}

/**
    * Establece el valor de la hora.
    * Si un valor es -1, no se cambia ese valor
    */
void GDate::setTimeValue( int hour, int minute, int second )
{
    if ( hour != -1 )
        valorInterno.tm_hour = hour;

    if ( minute != -1 )
        valorInterno.tm_min = minute;

    if ( second != -1 )
        valorInterno.tm_sec = second;

    mktime(&valorInterno);
}

/**
* Pone la hora a 0:00:00
*/
void GDate::resetTime()
{
    valorInterno.tm_hour = 0;
    valorInterno.tm_min = 0;
    valorInterno.tm_sec = 0;
}

/**
    * Agrega valores a la fecha.
    * Puede ser un valor negativo     
    */
void GDate::addDate( int year, int month, int day )
{
    valorInterno.tm_year+= (year-1900);
    valorInterno.tm_mon+= month;
    valorInterno.tm_mday+= day;
    mktime(&valorInterno);
}

/**
    * Agrega valores a la hora
    * Puede ser un valor negativo
    */
void GDate::addTime( int hour, int minute, int second )
{
    valorInterno.tm_hour+= hour;
    valorInterno.tm_min+= minute;
    valorInterno.tm_sec+= second;
    mktime(&valorInterno);
}

/**
    * Retorna la diferencia en segundos de la fecha actual en segundos
    */
double GDate::diff( GDate date )
{
    return difftime(toTimeStamp(),date.toTimeStamp());
}

/**
 * Hace que la fecha ponga sus datos a la hora actual
 */
void GDate::toCurrentTime()
{
    // free(valorInterno);
    time_t ahora = time(NULL);
    tm *ahora_tm  = localtime(&ahora);
    valorInterno = *ahora_tm;
}

/**
 * Retorna una fecha copia
 */
GDate GDate::getClone()
{
    GDate fecha;
    fecha.setTime(toTimeStamp());

    return fecha;
}

/**
 * Retorna el dia de la semana, valor entre 0 y 6
 */
int GDate::getWeekDay()
{
    return valorInterno.tm_wday;
}

/**
 * Asigna el valor desde un formato de hora
 */
void GDate::setTime( time_t time)
{
    // free(valorInterno);
    tm * ahora_tm = localtime(&time);
    valorInterno = *ahora_tm;
}

/**
 * Copara una fecha con otra.
 * Retorna valor < 0 en caso la fecha sea menor al parametro, 0 en caso sean iguales, > 0 en caso la fecha sea mayor al parametro
 */
int GDate::compareTo( GDate fecha )
{
    double delta = diff(fecha);
    if ( delta < 0 ) return -1;
    if ( delta > 0 ) return 1;

    return 0;
}

/**
 * Retorna la fecha en formato: yyyy-mm-dd h24:mm:ss
 */
string GDate::toString()
{
    string rpta;

    rpta.append(to_string(valorInterno.tm_year+1900));
    rpta.append("-");
    rpta.append(to_string(valorInterno.tm_mon+1));
    rpta.append("-");
    rpta.append(to_string(valorInterno.tm_mday));
    rpta.append(" ");
    rpta.append(to_string(valorInterno.tm_hour));
    rpta.append(":");
    rpta.append(to_string(valorInterno.tm_min));
    rpta.append(":");
    rpta.append(to_string(valorInterno.tm_sec));

    return rpta;
}

/**
 * Convierte la fecha a formato dd-mm-yyyy
 *  separator: separador entre los elementos de la fecha
 */
string GDate::toStringDMY( string separator )
{
    string rpta;

    rpta.append(to_string(valorInterno.tm_mday));
    rpta.append(separator);
    rpta.append(to_string(valorInterno.tm_mon+1));
    rpta.append(separator);
    rpta.append(to_string(valorInterno.tm_year+1900));
    
    return rpta;
}

/**
 * Convierte la fecha a formato dd-mm-yyyy hh:mm:ss
 *  separator: separador entre los elementos de la fecha
 */
string GDate::toStringDMYHmS( string separator )
{
    string rpta;

    rpta.append(to_string(valorInterno.tm_mday));
    rpta.append(separator);
    rpta.append(to_string(valorInterno.tm_mon+1));
    rpta.append(separator);
    rpta.append(to_string(valorInterno.tm_year+1900));
    rpta.append(" ");
    rpta.append(to_string(valorInterno.tm_hour));
    rpta.append(":");
    rpta.append(to_string(valorInterno.tm_min));
    rpta.append(":");
    rpta.append(to_string(valorInterno.tm_sec));

    return rpta;
}

/**
 * Convierte la fecha a formato mm-dd-yyyy
 *  separator: separador entre los elementos de la fecha
 */
string GDate::toStringMDY( string separator )
{
    string rpta;

    rpta.append(to_string(valorInterno.tm_mon+1));
    rpta.append(separator);    
    rpta.append(to_string(valorInterno.tm_mday));
    rpta.append(separator);
    rpta.append(to_string(valorInterno.tm_year+1900));
    
    return rpta;
}

/**
    * Convierte la fecha a formato mm-dd-yyyy hh:mm:ss
    *  separator: separador entre los elementos de la fecha
    */
string GDate::toStringMDYHmS( string separator )
{
    string rpta;

    rpta.append(to_string(valorInterno.tm_mon+1));    
    rpta.append(separator);
    rpta.append(to_string(valorInterno.tm_mday));
    rpta.append(separator);
    rpta.append(to_string(valorInterno.tm_year+1900));
    rpta.append(" ");
    rpta.append(to_string(valorInterno.tm_hour));
    rpta.append(":");
    rpta.append(to_string(valorInterno.tm_min));
    rpta.append(":");
    rpta.append(to_string(valorInterno.tm_sec));

    return rpta;
}