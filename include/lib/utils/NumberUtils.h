
#ifndef _NUMBER_UTILS_
#define _NUMBER_UTILS_


/**
 * Clase con utilitarios matematicos
 */
class NumberUtils
{
    public:

        /**
         * Dado un valor y sus posibles valores maximos y minimos retorna el valor
         * que este dentro de ese rango.
         *      Si valor < valMin retorna valMin
         *      Si valor >= valMax retorna valMax-1
         *      De lo contrario retorna valor
         */
        float static getValorRango( float valor, float valMin, float valMax);
};

#endif