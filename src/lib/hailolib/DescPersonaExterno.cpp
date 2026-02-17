// src/lib/hailolib/DescPersonaExterno.cpp
#include "lib/hailolib/DescPersonaExterno.h"
#include "lib/hailolib/UnivIdenPersona.h"
#include <cmath>

DescPersonaExterno::DescPersonaExterno()
{
    norm_coseno = 0.0f;
    norm_coceno_calculado = false;
}

void DescPersonaExterno::reset()
{
    GLinkedList<UnivIdenPersona*> copia = lstPersonas.getClone();

    for (int i = 0; i < copia.size(); ++i)
    {
        UnivIdenPersona* u = copia.get(i);
        if (u != nullptr)
        {
            u->borrarDescPersonaExterno(this);
        }
    }

    lstPersonas.reset();
}

void DescPersonaExterno::normaliza()
{
    float norm1 = 0.0f;

    for (size_t i = 0; i < NUM_ELEMS_DESC_FACIAL; ++i)
    {
        float v = (float)vecDescripcion[i];
        norm1 += v * v;
    }

    norm1 = std::sqrt(norm1);
    if (norm1 <= 0.0f || std::isnan(norm1) || std::isinf(norm1))
    {
        norm_coceno_calculado = false;
        return;
    }

    for (size_t i = 0; i < NUM_ELEMS_DESC_FACIAL; ++i)
    {
        float v = (float)vecDescripcion[i];
        v /= norm1;
        vecDescripcion[i] = (SIMD_TYPE)v;
    }

    norm_coceno_calculado = true;
}
