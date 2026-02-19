#include "lib/hailolib/UnivIdenPersona.h"
#include "lib/hailolib/DescPersonaExterno.h"

UnivIdenPersona::UnivIdenPersona()
{
    indiceGrupoPrin = -1;
    numIden = 0;
    cambioIdentificacion = false;
    fechaUltEvento = 0;
}

void UnivIdenPersona::recalcularPrincipal()
{
    const int n = lstGrupoIden.size();
    if (n <= 0)
    {
        indiceGrupoPrin = -1;
        numIden = 0;
        return;
    }

    int bestIdx = -1;
    int bestLen = -1;

    for (int i = 0; i < n; ++i)
    {
        const GrupoIdenPersona* g = lstGrupoIden.getAddr(i);
        if (!g) continue;

        const int len = g->lstIdentificaciones.size();
        if (len > bestLen)
        {
            bestLen = len;
            bestIdx = i;
        }
    }

    // si todos están vacíos -> principal inválido
    if (bestLen <= 0)
    {
        indiceGrupoPrin = -1;
        numIden = 0;
        return;
    }

    indiceGrupoPrin = bestIdx;
    numIden = bestLen;
}

void UnivIdenPersona::agregaIdentif(DescPersonaExterno* descExterno, IdentificacionPersona iden, long long fecDet)
{
    const std::string idKey     = (descExterno != nullptr) ? descExterno->id      : std::string("__ANON__");
    const std::string nombreKey = (descExterno != nullptr) ? descExterno->nombre  : std::string("");
    const bool anonKey          = (descExterno != nullptr) ? descExterno->anonimo : true;

    const int idxAntes = indiceGrupoPrin;
    const int n = lstGrupoIden.size();

    // 1) buscar grupo existente
    for (int i = 0; i < n; ++i)
    {
        GrupoIdenPersona* g = lstGrupoIden.getAddr(i);
        if (!g) continue;

        if (g->anonimoExterno == anonKey && g->idExterno == idKey)
        {
            iden.fecDet = fecDet;
            g->lstIdentificaciones.add(iden);

            if (g->nombreExterno.empty() && !nombreKey.empty())
                g->nombreExterno = nombreKey;

            if (g->descExterno == nullptr)
                g->descExterno = descExterno;

            // actualizar principal sólo si superó numIden
            const int len = g->lstIdentificaciones.size();
            if (len > numIden)
            {
                cambioIdentificacion = (i != indiceGrupoPrin);
                indiceGrupoPrin = i;
                numIden = len;
            }

            // registrar backref sin duplicados
            if (descExterno != nullptr)
            {
                if (!descExterno->lstPersonas.contains(this))
                    descExterno->lstPersonas.add(this);
            }
            return;
        }
    }

    // 2) crear nuevo grupo
    GrupoIdenPersona nuevo;
    nuevo.idExterno = idKey;
    nuevo.nombreExterno = nombreKey;
    nuevo.anonimoExterno = anonKey;
    nuevo.descExterno = descExterno; // non-owning cache
    nuevo.fecCrea = fecDet;

    iden.fecDet = fecDet;
    nuevo.lstIdentificaciones.add(iden);

    lstGrupoIden.add(nuevo);

    // recomputa principal (porque puede ser el primero o cambiar ranking)
    recalcularPrincipal();

    // marca cambio si cambió el principal o si antes no había
    if (idxAntes != indiceGrupoPrin || idxAntes == -1)
        cambioIdentificacion = true;

    // registrar backref sin duplicados
    if (descExterno != nullptr)
    {
        if (!descExterno->lstPersonas.contains(this))
            descExterno->lstPersonas.add(this);
    }
}

void UnivIdenPersona::agregaIdentifStable(
    const std::string &idExterno,
    bool anonimoExterno,
    const std::string &nombreExterno,
    const IdentificacionPersona &iden,
    long long fecDet)
{
    // Si no hay ID estable, no hacemos nada
    if (idExterno.empty()) return;

    bool encontro = false;
    const int n = lstGrupoIden.size();

    for (int i = 0; i < n; ++i)
    {
        GrupoIdenPersona *g = lstGrupoIden.getAddr(i);
        if (!g) continue;

        if (g->anonimoExterno == anonimoExterno && g->idExterno == idExterno)
        {
            IdentificacionPersona tmp = iden;
            tmp.fecDet = fecDet;
            g->lstIdentificaciones.add(tmp);

            // Completar nombre si faltaba
            if (g->nombreExterno.empty() && !nombreExterno.empty())
                g->nombreExterno = nombreExterno;

            // MUY IMPORTANTE: NO tocamos g->descExterno aquí (no punteros)
            // g->descExterno queda como estaba.

            // Actualiza principal SOLO si este grupo supera al actual
            const int len = g->lstIdentificaciones.size();
            if (len > numIden)
            {
                cambioIdentificacion = (i != indiceGrupoPrin);
                indiceGrupoPrin = i;
                numIden = len;
            }

            encontro = true;
            break;
        }
    }

    if (!encontro)
    {
        GrupoIdenPersona nuevo;
        nuevo.idExterno = idExterno;
        nuevo.nombreExterno = nombreExterno;
        nuevo.anonimoExterno = anonimoExterno;
        nuevo.descExterno = nullptr; // NO punteros
        nuevo.fecCrea = fecDet;

        IdentificacionPersona tmp = iden;
        tmp.fecDet = fecDet;
        nuevo.lstIdentificaciones.add(tmp);

        lstGrupoIden.add(nuevo);

        // Si no había principal, lo seteamos
        if (indiceGrupoPrin < 0)
        {
            indiceGrupoPrin = 0;
            numIden = 1;
            cambioIdentificacion = true;
        }
        else
        {
            // si el nuevo supera (normalmente arranca en 1)
            const int idxNuevo = lstGrupoIden.size() - 1;
            const int len = lstGrupoIden.getAddr(idxNuevo)->lstIdentificaciones.size();
            if (len > numIden)
            {
                indiceGrupoPrin = idxNuevo;
                numIden = len;
                cambioIdentificacion = true;
            }
        }
    }
}

void UnivIdenPersona::borrarDescPersonaExterno(DescPersonaExterno* descExterno)
{
    const int idxAntes = indiceGrupoPrin;

    for (int i = lstGrupoIden.size() - 1; i >= 0; --i)
    {
        GrupoIdenPersona* g = lstGrupoIden.getAddr(i);
        if (g != nullptr && g->descExterno == descExterno)
        {
            g->lstIdentificaciones.reset();
            lstGrupoIden.remove(i);
        }
    }

    recalcularPrincipal();

    if (idxAntes != indiceGrupoPrin)
        cambioIdentificacion = true;

    if (lstGrupoIden.size() == 0)
        cambioIdentificacion = true;
}

DescPersonaExterno* UnivIdenPersona::getDatosPerIden()
{
    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return nullptr;

    GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    return g ? g->descExterno : nullptr;
}

const DescPersonaExterno* UnivIdenPersona::getDatosPerIden() const
{
    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return nullptr;

    const GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    return g ? g->descExterno : nullptr;
}

float UnivIdenPersona::getPromedioComparacion() const
{
    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return 0.0f;

    const GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    if (!g) return 0.0f;

    const int n = g->lstIdentificaciones.size();
    if (n <= 0) return 0.0f;

    float sum = 0.0f;
    for (int i = 0; i < n; ++i)
        sum += g->lstIdentificaciones.get(i).comparacion;

    return sum / (float)n;
}

int UnivIdenPersona::getNumIdentificaciones() const
{
    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return 0;

    const GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    return g ? g->lstIdentificaciones.size() : 0;
}

IdentificacionPersona UnivIdenPersona::getUltimaIdentificacion() const
{
    IdentificacionPersona r;

    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return r;

    const GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    if (!g) return r;

    if (g->lstIdentificaciones.size() <= 0)
        return r;

    return g->lstIdentificaciones.getLast();
}

void UnivIdenPersona::reset()
{
    for (int i = 0; i < lstGrupoIden.size(); ++i)
    {
        GrupoIdenPersona* g = lstGrupoIden.getAddr(i);
        if (g) g->lstIdentificaciones.reset();
    }

    lstGrupoIden.reset();
    indiceGrupoPrin = -1;
    numIden = 0;
    cambioIdentificacion = false;
    fechaUltEvento = 0;
}

std::string UnivIdenPersona::getIdExternoPrincipal() const
{
    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return "";

    const GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    return g ? g->idExterno : "";
}

std::string UnivIdenPersona::getNombreExternoPrincipal() const
{
    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return "";

    const GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    return g ? g->nombreExterno : "";
}

bool UnivIdenPersona::getAnonimoPrincipal() const
{
    if (indiceGrupoPrin < 0 || indiceGrupoPrin >= lstGrupoIden.size())
        return false;

    const GrupoIdenPersona* g = lstGrupoIden.getAddr(indiceGrupoPrin);
    return g ? g->anonimoExterno : false;
}
