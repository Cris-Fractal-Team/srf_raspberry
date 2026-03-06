#ifndef DESCPERSONAEXTERNO_H
#define DESCPERSONAEXTERNO_H

#include <string>
#include "lib/hailolib/FaceTypes.h"
#include "lib/general/GLinkedList.h"

class UnivIdenPersona;

class DescPersonaExterno
{
public:
    DescPersonaExterno();

    std::string id;
    std::string nombre;
    bool anonimo = false;
    bool norm_coceno_calculado = false;
    long long ultimaFechaDetectada = 0;

    SIMD_TYPE vecDescripcion[NUM_ELEMS_DESC_FACIAL]{};
    float norm_coseno = 0.0f;

    GLinkedList<UnivIdenPersona*> lstPersonas;

    void reset();
    void normaliza();
};

#endif
