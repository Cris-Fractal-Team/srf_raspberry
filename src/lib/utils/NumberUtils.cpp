
#include "lib/utils/NumberUtils.h"


float NumberUtils::getValorRango( float valor, float valMin, float valMax)
{
    if ( valor < valMin )
        return valMin;

    if ( valor >= valMax )
        return valMax-1;

    return valor;
}