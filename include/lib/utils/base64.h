
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "vector"

#ifndef BASE64_H
#define BASE64_H

typedef unsigned char uchar;

/**
 * Codifica un arreglo de bytes en base 64 y el resultado lo almacena en un buffer pasado como parametro
 *
 * param data : arreglo que se codifica
 *
 * param input_length : cantidad de bytes del arreglo
 *
 * param buffer : arreglo en el que se guardan los datos
 *
 * param output_length : retorna la cantidad de bytes que se crearon en la codificacion
 * 
 */
void base64_encode_intoBuffer( char *data, size_t input_length, char *buffer, size_t *output_length);

/**
 * Codifica en base 64, agrega al final del buffer un caracter extra al que se le agrega el valor 0
 * para que la cadena se pueda usar directamente
 */
char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);




unsigned char *base64_decode( char *data,
                             size_t input_length,
                             size_t *output_length);

#endif