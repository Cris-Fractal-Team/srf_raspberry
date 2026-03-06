#ifndef IDENTIFICACIONPERSONA_H
#define IDENTIFICACIONPERSONA_H

#include "lib/hailolib/FaceTypes.h"

class IdentificacionPersona
{
public:
    long long fecDet = 0;
    float comparacion = 0.0f;
    SIMD_TYPE vecDescripcion[NUM_ELEMS_DESC_FACIAL]{};
};

#endif
