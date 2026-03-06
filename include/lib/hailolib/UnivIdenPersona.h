#ifndef UNIVIDENPERSONA_H
#define UNIVIDENPERSONA_H

#include <string>
#include "lib/general/GLinkedList.h"
#include "lib/hailolib/IdentificacionPersona.h"

class DescPersonaExterno;

class GrupoIdenPersona
{
public:
    std::string idExterno;
    std::string nombreExterno;
    bool anonimoExterno = false;

    // non-owning cache
    DescPersonaExterno* descExterno = nullptr;

    long long fecCrea = 0;
    GLinkedList<IdentificacionPersona> lstIdentificaciones;
};

class UnivIdenPersona
{
public:
    GLinkedList<GrupoIdenPersona> lstGrupoIden;

    long long id = 0;
    int indiceGrupoPrin = -1;
    int numIden = 0;
    bool cambioIdentificacion = false;
    long long fechaUltEvento = 0;

    UnivIdenPersona();

    void agregaIdentif(DescPersonaExterno* descExterno, IdentificacionPersona iden, long long fecDet);
    
    void agregaIdentifStable(
    const std::string &idExterno,
    bool anonimoExterno,
    const std::string &nombreExterno,
    const IdentificacionPersona &iden,
    long long fecDet);

    void borrarDescPersonaExterno(DescPersonaExterno* descExterno);

    DescPersonaExterno* getDatosPerIden();
    const DescPersonaExterno* getDatosPerIden() const; // ✅ overload const

    float getPromedioComparacion() const;              // ✅ const
    int getNumIdentificaciones() const;                // ✅ const
    IdentificacionPersona getUltimaIdentificacion() const; // ✅ const
    void reset();

    std::string getIdExternoPrincipal() const;
    std::string getNombreExternoPrincipal() const;
    bool getAnonimoPrincipal() const;

private:
    void recalcularPrincipal();
};

#endif
