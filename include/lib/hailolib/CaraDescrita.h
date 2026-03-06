#ifndef CARADESCRITA_H
#define CARADESCRITA_H

#include "lib/hailolib/FaceTypes.h"
#include "lib/hailolib/UnivIdenPersona.h"
#include "lib/hailolib/DetectorCaraHaloScrfd.h"
#include "lib/graphics/GDibujo.h"

class CaraDescrita
{
public:
    DeteccionCaraHailo deteccion;
    DeteccionCaraHailo detRelCara;

    GImage fotoCara;

    UnivIdenPersona identificador;

    bool personaIdent = false;
    SIMD_TYPE descriptor[NUM_ELEMS_DESC_FACIAL]{};
    bool descCalculado = false;

    long long id = 0;

    CaraDescrita();

    void calculaFotoCara(GImage foto);
};

#endif
