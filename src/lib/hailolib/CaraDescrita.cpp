#include "lib/hailolib/CaraDescrita.h"

CaraDescrita::CaraDescrita()
{
    descCalculado = false;
    personaIdent = false;
    id = 0;
}

void CaraDescrita::calculaFotoCara(GImage foto)
{
    fotoCara = foto.getRect(deteccion.ptoSupIzq.x, deteccion.ptoSupIzq.y, deteccion.ptoInfDer.x, deteccion.ptoInfDer.y);
}
