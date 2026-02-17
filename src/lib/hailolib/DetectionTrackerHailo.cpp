
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/general/GLinkedList.h"
#include "lib/hailolib/DescPersonaExterno.h"

#include <math.h>
#include <cstring>


/**
 * Constructor
 */
TrackedDetectionHailo::TrackedDetectionHailo()
{
    ciclosNoDetectados = 0;
}

/**
 * Constructor
 */
DetectionTrackerHailo::DetectionTrackerHailo()
{
    deltaMaxX = 0.8;
    deltaMaxY = 0.8;
    maxCiclosInactivo = 1;
    sgteId = 1;
}

/**
 * Desctructor
 */
DetectionTrackerHailo::~DetectionTrackerHailo()
{
    reset();
}


/**
  * Agrega una nueva deteccion al universo de detecciones
  */
 TrackedDetectionHailo DetectionTrackerHailo::agregaDeteccion ( DeteccionCaraHailo det )
 {
    TrackedDetectionHailo trDet;
  
    trDet.id = sgteId;
    sgteId++;

    trDet.cara.deteccion = det;
    trDet.ciclosNoDetectados = 0;
        
    lstUniverso.add(trDet);

    return trDet;
 }

 /**
 * Agrega una nueva deteccion temporal
 */
TrackedDetectionHailo DetectionTrackerHailo::agregaDeteccionTemporal ( DeteccionCaraHailo det )
{
    TrackedDetectionHailo trDet;
  
    trDet.id = 0;
   
    trDet.cara.deteccion = det;
    trDet.ciclosNoDetectados = 0;
        
    lstUniversoTemp.add(trDet);

    return trDet;
}

 /**
 * Genera una lista de decciones a las que se les puede hacer tracking
 * las detecciones actuales y le asigna un ID a cada deeccion.
 */
vector<TrackedDetectionHailo *> DetectionTrackerHailo::generaListaTrackTemporal( vector<DeteccionCaraHailo> *lstDetActuales )
{
    vector<TrackedDetectionHailo *> rpta;
    TrackedDetectionHailo *rostroTrack;

    lstUniversoTemp.reset();
    for( DeteccionCaraHailo cara: *lstDetActuales )
    {
        agregaDeteccionTemporal(cara);
        rostroTrack = lstUniversoTemp.getAddrUltimo();        
        rpta.push_back(rostroTrack);
    }

    return rpta;
}


 /**
 * Analiza las detecciones actualesy reconocidas, les asigna un ID unico
 * para poder hacer un tracking, pero principalmente para poder llevar
 * estadisticas del rostro y agrupar las detecciones 
 */
vector<TrackedDetectionHailo *> DetectionTrackerHailo::analizaPorIdentificacion( vector<TrackedDetectionHailo *> *lstDetActuales )
{
    int i,iu,n,nu;
    vector<TrackedDetectionHailo *> rpta;
    TrackedDetectionHailo *detActual;
    DescPersonaExterno *descPerActual, *descPerUniv;
    IdentificacionPersona idenPersona;

    n = lstDetActuales->size();
    nu = lstUniverso.size();

    if ( nu == 0 )
    {
        // El universo esta vacio se agregan todas las detecciones y se les asigna un track unico
        for(i=0; i<n; i++)
        {
            detActual = lstDetActuales->at(i);

            descPerActual = detActual->cara.identificador.getDatosPerIden();
            if ( descPerActual == NULL )
            {
                rpta.push_back(detActual);
                continue;
            }

            detActual->id = sgteId;
            sgteId++;            
            lstUniverso.add(*detActual);
            detActual = lstUniverso.getAddrUltimo();
            rpta.push_back(detActual);
        }

        return rpta;
    }

    GLinkedList<TrackedDetectionHailo *> lstUnivPen;
    GLinkedList<int>lstUnivPenIndices;
    TrackedDetectionHailo *detUniv;
    bool encontro;

    // creamos la lista temporal
    for(i=0; i<nu; i++)
    {
        detUniv = lstUniverso.getAddr(i);        

        lstUnivPen.add(lstUniverso.getAddr(i));
        lstUnivPenIndices.add(i);
    }

    // Asociamos las detecciones actuales y las nuevas
    for( i=0; i < n; i++)
    {
        detActual = lstDetActuales->at(i);            
        descPerActual = detActual->cara.identificador.getDatosPerIden();
        if ( descPerActual == NULL )
        {
            rpta.push_back(detActual);
            continue;
        }
            
        
        encontro = false;
        nu = lstUnivPen.size()-1;
        for(iu=nu; iu>=0; iu--)
        {
            detUniv = lstUnivPen.get(iu);
            
            const std::string idUniv = detUniv->cara.identificador.getIdExternoPrincipal();
            if (idUniv.empty())
                continue;

            const bool anonUniv = detUniv->cara.identificador.getAnonimoPrincipal();

            if ((descPerActual->anonimo == anonUniv) && (descPerActual->id == idUniv))
            {
                // Encontramos que la persona actual o nueva coincide con una del universo
                idenPersona = detActual->cara.identificador.getUltimaIdentificacion();
                
                detUniv->cara.fotoCara = detActual->cara.fotoCara;
                detUniv->cara.deteccion = detActual->cara.deteccion;
                detUniv->ciclosNoDetectados = 0;
                std::memcpy(detUniv->cara.descriptor, detActual->cara.descriptor, NUM_ELEMS_DESC_FACIAL*sizeof(SIMD_TYPE));
                detUniv->cara.identificador.agregaIdentif(descPerActual, idenPersona, idenPersona.fecDet);
                lstUnivPen.remove(iu);
                lstUnivPenIndices.remove(iu);
                encontro = true;

                rpta.push_back(detUniv);

                break;
            }
        }
        
        if ( encontro == false )
        {
            // la deteccion es nueva
            detActual->id = sgteId;
            sgteId++;            
            lstUniverso.add(*detActual);
            detActual = lstUniverso.getAddrUltimo();
            rpta.push_back(detActual);
        }
    }

    // revisamos las detecciones anterioes que no coincidieron en las detecciones
    nu = lstUnivPen.size()-1;
    for(iu=nu; iu>=0; iu--)
    {
        detUniv = lstUnivPen.get(iu);
        detUniv->ciclosNoDetectados++;
        if ( detUniv->ciclosNoDetectados > maxCiclosInactivo )
        {
            i = lstUnivPenIndices.get(iu);
            detUniv = lstUniverso.getAddr(i);
            // detUniv->cara.identificador.lstGrupoIden.reset();            
            detUniv->cara.identificador.reset();
            lstUniverso.remove(i);
        }
    }

    lstUnivPen.reset();

    return rpta;
}

/**
 * Libera la RAM
 */
void DetectionTrackerHailo::reset()
{
    int n = lstUniverso.size();
    TrackedDetectionHailo *detUniv;

    for(int i=0; i < n; i++)
    {
        detUniv = lstUniverso.getAddr(i);
        detUniv->cara.identificador.reset();      
    }
    lstUniverso.reset();
}

/**
 *   Analiza las detecciones actuales y reconocidas, les asigna un ID unico
 * para poder hacer un tracking, pero principalmente para poder llevar
 * estadisticas del rostro y agrupar las detecciones adicionalmente aplica un algoritmo
 * de tracking KLT/KCF para poder identificar rostros que en la imagen anterior fueron identificados
 * pero en la actual no, pero existen rostro sin identificacion o anonimos cercano.
 *
 * ✅ Versión "asegurada":
 * - NO dereferencia DescPersonaExterno* (evita tipo incompleto y punteros colgantes).
 * - Matching estable por (anonimo + idExternoPrincipal).
 * - Protege accesos a getUltimaIdentificacion() y punteros nulos.
 * - Evita remover del universo con índices inválidos tras múltiples removes.
 */
vector<TrackedDetectionHailo *> DetectionTrackerHailo::analizaPorIdentificacionYTracker(
    vector<TrackedDetectionHailo *> *lstDetActuales,
    GImage *imagenVisorActual,
    GImage *imagenPreviaVisor,
    float factorXVisor,
    float factorYVisor)
{
    vector<TrackedDetectionHailo *> rpta;

    if (lstDetActuales == nullptr || imagenVisorActual == nullptr || imagenPreviaVisor == nullptr)
        return rpta;

    const int n = (int)lstDetActuales->size();
    int nu = lstUniverso.size();

    // Helpers locales seguros (evita crashes por indices)
    auto safeGetUniv = [&](int idx) -> TrackedDetectionHailo* {
        if (idx < 0 || idx >= lstUniverso.size()) return nullptr;
        return lstUniverso.getAddr(idx);
    };

    // ------------------------------------------------------------
    // Caso 1: Universo vacío -> asignar IDs y poblar universo
    // ------------------------------------------------------------
    if (nu == 0)
    {
        for (int i = 0; i < n; ++i)
        {
            TrackedDetectionHailo *detActual = lstDetActuales->at(i);
            if (detActual == nullptr)
                continue;

            const std::string idActual = detActual->cara.identificador.getIdExternoPrincipal();
            const bool anonActual = detActual->cara.identificador.getAnonimoPrincipal();

            // Si no hay ID y no es anónimo => no lo metemos al universo (solo retorno)
            if (idActual.empty() && !anonActual)
            {
                rpta.push_back(detActual);
                continue;
            }

            detActual->id = sgteId++;
            lstUniverso.add(*detActual);

            TrackedDetectionHailo *detUniv = lstUniverso.getAddrUltimo();
            if (detUniv)
                rpta.push_back(detUniv);
            else
                rpta.push_back(detActual);
        }
        return rpta;
    }

    // ------------------------------------------------------------
    // Listas temporales: pendientes del universo y nuevos (anon/noid)
    // ------------------------------------------------------------
    GLinkedList<TrackedDetectionHailo *> lstUnivPen, lstNuevos;
    GLinkedList<int> lstUnivPenIndices;

    // Cargar pendientes (guardamos puntero y el índice ORIGINAL)
    for (int i = 0; i < nu; ++i)
    {
        TrackedDetectionHailo *detUniv = safeGetUniv(i);
        if (!detUniv) continue;
        lstUnivPen.add(detUniv);
        lstUnivPenIndices.add(i);
    }

    // ------------------------------------------------------------
    // Asociar detecciones actuales con universo por (anon + id)
    // ------------------------------------------------------------
    for (int i = 0; i < n; ++i)
    {
        TrackedDetectionHailo *detActual = lstDetActuales->at(i);
        if (!detActual)
            continue;

        const std::string idActual = detActual->cara.identificador.getIdExternoPrincipal();
        const bool anonActual = detActual->cara.identificador.getAnonimoPrincipal();

        // Si no tiene ID y no es anon => puede ser "no reconocido" (o no se reporta)
        if (idActual.empty() && !anonActual)
        {
            lstNuevos.add(detActual);
            continue;
        }

        bool encontro = false;

        // Buscar desde el final para poder remove(iu) sin romper iteración
        for (int iu = lstUnivPen.size() - 1; iu >= 0; --iu)
        {
            TrackedDetectionHailo *detUniv = lstUnivPen.get(iu);
            if (!detUniv) continue;

            const std::string idUniv = detUniv->cara.identificador.getIdExternoPrincipal();
            if (idUniv.empty()) continue;

            const bool anonUniv = detUniv->cara.identificador.getAnonimoPrincipal();

            if (anonActual == anonUniv && idActual == idUniv)
            {
                // ✅ Coincide con una del universo
                // Solo usar ultima ident si existe (evita segfault)
                IdentificacionPersona idenPersona;
                if (detActual->cara.identificador.getNumIdentificaciones() > 0)
                    idenPersona = detActual->cara.identificador.getUltimaIdentificacion();
                else
                {
                    // Si no hay identificaciones, no tocamos el identificador del universo
                    // pero igual actualizamos foto/detección/descriptor.
                    idenPersona.fecDet = 0;
                    idenPersona.comparacion = 0.0f;
                }

                detUniv->cara.fotoCara = detActual->cara.fotoCara;
                detUniv->cara.deteccion = detActual->cara.deteccion;

                std::memcpy(detUniv->cara.descriptor,
                            detActual->cara.descriptor,
                            NUM_ELEMS_DESC_FACIAL * sizeof(SIMD_TYPE));

                // ✅ No dependemos de DescPersonaExterno* (puntero colgante)
                if (idenPersona.fecDet != 0)
                    detUniv->cara.identificador.agregaIdentif(nullptr, idenPersona, idenPersona.fecDet);

                detUniv->ciclosNoDetectados = 0;

                if (!detUniv->tracker.empty())
                    detUniv->tracker.release();

                // sacar de pendientes
                lstUnivPen.remove(iu);
                lstUnivPenIndices.remove(iu);

                rpta.push_back(detUniv);
                encontro = true;
                break;
            }
        }

        if (!encontro)
        {
            // Detección nueva: si NO es anónimo -> se guarda al universo
            if (!anonActual)
            {
                detActual->id = sgteId++;
                lstUniverso.add(*detActual);

                TrackedDetectionHailo *detUniv = lstUniverso.getAddrUltimo();
                if (detUniv)
                {
                    if (!detUniv->tracker.empty())
                        detUniv->tracker.release();
                    rpta.push_back(detUniv);
                }
                else
                {
                    rpta.push_back(detActual);
                }
            }
            else
            {
                // anónimos se guardan para intentar coincidencia visual
                lstNuevos.add(detActual);
            }
        }
    }

    // ------------------------------------------------------------
    // Analiza nuevas anónimas vs pendientes (si existe la función)
    // ------------------------------------------------------------
    nu = lstUnivPen.size();
    const int nNuevos = lstNuevos.size();
    if ((nNuevos >= 1) && (nu >= 1))
    {
        analizaDetNuevasAnonimas(
            &rpta,
            &lstUnivPen,
            &lstNuevos,
            &lstUnivPenIndices,
            imagenVisorActual,
            imagenPreviaVisor,
            factorXVisor,
            factorYVisor);
    }

    // ------------------------------------------------------------
    // Tracking para los pendientes del universo
    // OJO: lstUniverso puede cambiar tamaño si removemos.
    // Para hacerlo seguro, removemos en 2 fases:
    // 1) marcamos indices a borrar
    // 2) borramos del universo al final, en orden descendente
    // ------------------------------------------------------------
    vector<int> indicesParaBorrar;
    indicesParaBorrar.reserve(lstUnivPen.size());

    for (int iu = lstUnivPen.size() - 1; iu >= 0; --iu)
    {
        TrackedDetectionHailo *detUniv = lstUnivPen.get(iu);
        if (!detUniv) continue;

        detUniv->ciclosNoDetectados++;

        int idxUniv = lstUnivPenIndices.get(iu); // índice original en lstUniverso

        if (detUniv->ciclosNoDetectados > maxCiclosInactivo)
        {
            // marca para borrar del universo
            if (idxUniv >= 0)
                indicesParaBorrar.push_back(idxUniv);

            // no lo agregamos a rpta
            continue;
        }

        // Tracking de la imagen pendiente
        if (detUniv->tracker.empty())
        {
            detUniv->tracker = cv::TrackerKCF::create();
            detUniv->trackerBox = detUniv->cara.deteccion.getOpenCV2DRectBoundingBox();

            // Reducimos escala de bbox a visor
            detUniv->trackerBox.x = (int)(((float)detUniv->cara.deteccion.ptoSupIzq.x) / factorXVisor);
            detUniv->trackerBox.y = (int)(((float)detUniv->cara.deteccion.ptoSupIzq.y) / factorYVisor);
            detUniv->trackerBox.width  = (int)(((float)detUniv->cara.deteccion.getAncho()) / factorXVisor);
            detUniv->trackerBox.height = (int)(((float)detUniv->cara.deteccion.getAltura()) / factorYVisor);

            // init protegido
            if (!imagenPreviaVisor->imagenOpencv.empty())
                detUniv->tracker->init(imagenPreviaVisor->imagenOpencv, detUniv->trackerBox);
        }

        // update protegido
        if (!imagenVisorActual->imagenOpencv.empty())
            detUniv->tracker->update(imagenVisorActual->imagenOpencv, detUniv->trackerBox);

        // pasar bbox a escala original
        cv::Rect rc = detUniv->trackerBox;
        rc.x      = (int)(((float)rc.x) * factorXVisor);
        rc.y      = (int)(((float)rc.y) * factorYVisor);
        rc.width  = (int)(((float)rc.width) * factorXVisor);
        rc.height = (int)(((float)rc.height) * factorYVisor);

        // clamp mínimo (evita cajas negativas)
        if (rc.width < 1) rc.width = 1;
        if (rc.height < 1) rc.height = 1;
        if (rc.x < 0) rc.x = 0;
        if (rc.y < 0) rc.y = 0;

        detUniv->cara.deteccion.ptoSupIzq.x = rc.x;
        detUniv->cara.deteccion.ptoSupIzq.y = rc.y;
        detUniv->cara.deteccion.ptoInfDer.x = rc.x + rc.width;
        detUniv->cara.deteccion.ptoInfDer.y = rc.y + rc.height;

        rpta.push_back(detUniv);
    }

    // ------------------------------------------------------------
    // Borrado seguro del universo: ordenar desc
    // ------------------------------------------------------------
    if (!indicesParaBorrar.empty())
    {
        std::sort(indicesParaBorrar.begin(), indicesParaBorrar.end());
        indicesParaBorrar.erase(std::unique(indicesParaBorrar.begin(), indicesParaBorrar.end()),
                                indicesParaBorrar.end());

        for (int k = (int)indicesParaBorrar.size() - 1; k >= 0; --k)
        {
            const int idx = indicesParaBorrar[k];
            if (idx < 0 || idx >= lstUniverso.size())
                continue;

            TrackedDetectionHailo *u = lstUniverso.getAddr(idx);
            if (u)
            {
                u->cara.identificador.reset();
                if (!u->tracker.empty())
                    u->tracker.release();
            }
            lstUniverso.remove(idx);
        }
    }

    lstUnivPen.reset();
    lstNuevos.reset();
    lstUnivPenIndices.reset();

    return rpta;
}



/**
 * Analiza las detecciones nuevas anonimas, las compara con las detecciones universales que no tienen matching
 * busca los mas cercanos en distancia y comparacion facial, para finalmente aplicar un tracking y
 * decidir si hay o no una coincidencia.
 */
void DetectionTrackerHailo::analizaDetNuevasAnonimas( vector<TrackedDetectionHailo *> *lstTracActual, GLinkedList<TrackedDetectionHailo *> *lstUnivPen,
    GLinkedList<TrackedDetectionHailo *> *lstNuevos, GLinkedList<int> *lstUnivPenIndices, GImage *imagenVisorActual, GImage *imagenPreviaVisor, float factorXVisor, float factorYVisor )
{
    int iPend,nPend;
    int iNuevos,nNuevos;
    int num,indiceNuevo;
    int porcentajeInter, areaInter, areaUniv;
    bool encontro;
    Rect rectNuevo;
    GLinkedList<int> lstConteoInterNuevos;
    GLinkedList<GLinkedList<int> *> lstIndicesInterPendientesNuev;
    GLinkedList<int> *lstIndicesNuevo;
    GLinkedList<int> lstNuevosUsados;
    TrackedDetectionHailo *detUniv, *detActual;
    IdentificacionPersona idenPersona;
    DescPersonaExterno *descPerActual;
    cv::Rect rc;
    

    // hacemos 0 el conteo de veces en el que los objetos nuevos tiene interseccion
    // con el objeto trackeado de un rostro anterior
    nNuevos = lstNuevos->size();
    for(iNuevos=0; iNuevos<nNuevos; iNuevos++)
    {
        lstConteoInterNuevos.add(0);
    }

    // creamos una lista con todos los rostros trackeados que no fueron identificados
    // y cada lista tiene una lista de indices con los rostros nuevos con los que
    // se ha intersectado
    nPend = lstUnivPen->size();    
    for(iPend=0; iPend<nPend; iPend++)
    {
        lstIndicesNuevo = new GLinkedList<int>(); 
        lstIndicesInterPendientesNuev.add(lstIndicesNuevo);
    }


    // rastreamos cada deteccion del universo que no encontro persona identificada
    // se compararan con las detecciones nuevas y si hay interseccion con la comparacion
    // del tracker se considera la misma persona
    for(iPend=0; iPend<nPend; iPend++)
    {
        detUniv = lstUnivPen->get(iPend);
        areaUniv = detUniv->cara.deteccion.getArea();

        if ( detUniv->tracker.empty() == true ) 
        {
        //    detUniv->tracker.release();

            // creamos el tracker            
            detUniv->tracker = cv::TrackerKCF::create();
            detUniv->trackerBox = detUniv->cara.deteccion.getOpenCV2DRectBoundingBox();

            // reducimos la escala de la recta de la deteccion a la imagen que tiene una resolucion menor
            detUniv->trackerBox.x = (int)(((float)detUniv->cara.deteccion.ptoSupIzq.x) / factorXVisor);
            detUniv->trackerBox.y = (int)(((float)detUniv->cara.deteccion.ptoSupIzq.y) / factorYVisor);
            detUniv->trackerBox.width = (int)(((float)detUniv->cara.deteccion.getAncho()) / factorXVisor);
            detUniv->trackerBox.height = (int)(((float)detUniv->cara.deteccion.getAltura()) / factorYVisor);

            detUniv->tracker->init(imagenPreviaVisor->imagenOpencv, detUniv->trackerBox);
        }

        // // hacemos que el tracker estime la nueva posicion
        detUniv->tracker->update(imagenVisorActual->imagenOpencv, detUniv->trackerBox);

        // // eliminamos el tracker
        // detUniv->tracker.release();

        // detUniv->trackerBox.x = 0;
        // detUniv->trackerBox.y = 0;
        // detUniv->trackerBox.width = 10;
        // detUniv->trackerBox.height = 10;
                                
        // escalamos el rectangulo del tracker a la resolucion de las detecciones
        // la imagen esta a una resolucion mas baja para optimizar el trackeo
        rc = detUniv->trackerBox;
        rc.x = (int)(((float) rc.x) * factorXVisor);
        rc.y = (int)(((float) rc.y) * factorYVisor);
        rc.width = (int)(((float) rc.width) * factorXVisor);
        rc.height = (int)(((float) rc.height) * factorYVisor);
        lstIndicesNuevo = lstIndicesInterPendientesNuev.get(iPend);

        // comparamos cada nueva deteccion con las cara actual del universo
        for(iNuevos=0; iNuevos<nNuevos; iNuevos++)
        {
            detActual = lstNuevos->get(iNuevos);
            rectNuevo = detActual->cara.deteccion.getOpenCV2DRectBoundingBox();

            areaInter = (rc & rectNuevo).area() * 100;
            porcentajeInter = areaInter / areaUniv ;
            // Si la interseccion es mayor o igual a 60% se puede considerar el mismo rostro
            if ( porcentajeInter >= 40 )
            {
                // incrementamos el contador porque ese objeto nuevo se intersecta
                // en un 60% con el tracker del pendiente                
                lstConteoInterNuevos.set(iNuevos, lstConteoInterNuevos.get(iNuevos)+1);

                // Guardamos el indice del objeto nuevo que intersecta con el objeto del universo pendiente
                lstIndicesNuevo->add(iNuevos);
            }
        }   
    }

    // Buscamos los objetos pendientes que tienen solo un objeto nuevo intersectado
    // consideramos que es el mismo objeto
    for(iPend=nPend-1; iPend>=0; iPend--)
    {
        detUniv = lstUnivPen->get(iPend);
        lstIndicesNuevo = lstIndicesInterPendientesNuev.get(iPend);

        if ( lstIndicesNuevo->size() != 1 ) 
            continue;
        
        indiceNuevo = lstIndicesNuevo->get(0);
        // validamos que la deteccion nueva solo tenga una interseccion con un objeto pendiente del universo
        if  ( lstConteoInterNuevos.get(indiceNuevo) != 1 ) 
            continue;
        
        detActual = lstNuevos->get(indiceNuevo);
        descPerActual = detActual->cara.identificador.getDatosPerIden();
        if ( descPerActual != NULL )
        {
            // solo se copia el descriptor facial si la persona fue identificada
            idenPersona = detActual->cara.identificador.getUltimaIdentificacion();
            std::memcpy(detUniv->cara.descriptor, detActual->cara.descriptor, NUM_ELEMS_DESC_FACIAL*sizeof(SIMD_TYPE));
            detUniv->cara.identificador.agregaIdentif(descPerActual, idenPersona, idenPersona.fecDet);  
        }
            
        detUniv->cara.fotoCara = detActual->cara.fotoCara;
        detUniv->cara.deteccion = detActual->cara.deteccion;

        detUniv->ciclosNoDetectados = 0;
        detUniv->cara.deteccion = detActual->cara.deteccion;
        
        lstUnivPen->remove(iPend);
        lstUnivPenIndices->remove(iPend);
        lstNuevosUsados.add(indiceNuevo);
                                
        lstTracActual->push_back(detUniv);
    }

    // registramos los nuevos no usados como nuevas detecciones
    nPend = lstNuevosUsados.size();
    for(iNuevos=0; iNuevos<nNuevos; iNuevos++)
    {
        detActual = lstNuevos->get(iNuevos);        
        encontro = false;

        // validamos si el indice de la deteccion ya fue usado
        if ( lstNuevosUsados.indexOf(iNuevos) >= 0 )
            continue;
        
        descPerActual = detActual->cara.identificador.getDatosPerIden();
        if ( descPerActual == NULL )
            continue;    
        
        // la deteccion o nodo nuevo no fue usado se agrega como desconocido
        detActual->id = sgteId;
        sgteId++;            
        lstUniverso.add(*detActual);
        detActual = lstUniverso.getAddrUltimo();
    
        lstTracActual->push_back(detActual);
    }

    // liberamos RAM
    nPend = lstIndicesInterPendientesNuev.size();
    for(iPend=0; iPend<nPend; iPend++)
    {
        lstIndicesNuevo = lstIndicesInterPendientesNuev.get(iPend);
        lstIndicesNuevo->reset();
        delete lstIndicesNuevo;
    }

    lstIndicesInterPendientesNuev.reset();
    lstConteoInterNuevos.reset();
    lstNuevosUsados.reset();

}


/**
 * Analiza las detecciones actuales y le asigna un ID a cada deeccion
 * 
 *  lstDetecActuales : lista con punteros a instancias de DeteccionVO
 * 
 *  numDetAct : numero de detecciones de lstDetecActuales
 * 
 *  numDet : cantidad de detecciones reetornadas
 */
vector<TrackedDetectionHailo *> DetectionTrackerHailo::analizaRapido( vector<DeteccionCaraHailo> *lstDetActuales )
{
    int i,n,nu,j,k;       
    
    DeteccionCaraHailo detActual;
    vector<TrackedDetectionHailo *> rpta;

    // cout << "Inicia Tracking :" << endl;
    // cout << "=================" << endl << endl;

    n = lstDetActuales->size();
    nu = lstUniverso.size();

    if ( nu == 0 )
    {
        TrackedDetectionHailo *rostro;

        // No hay detecciones en el universo todas se agregan
        for(i=0; i < n; i++)
        {                  
            detActual = lstDetActuales->at(i);    
            agregaDeteccion(detActual);
            rostro = lstUniverso.getAddrUltimo();
            // cout << "Agregando por que el universo es nuevo : " << rostro->id << endl;
            rpta.push_back(rostro);
        }

        return rpta;
    }  

    int iu,deltaX,deltaY,anchoDetAct,alturaDetAct,anchoUniv,alturaUniv;
    TrackedDetectionHailo *detUniv, *detNueva;
    PuntoHailo centroACtual, centroUniv;
    GLinkedList<TrackedDetectionHailo *> lstUnivPen;
    bool encontro;

    // llena la lista de trackins pendientes de analizar
    for(i=0; i<nu; i++)
    {
        detUniv = lstUniverso.getAddr(i);
        // cout << "Agregando a pendientes " << detUniv->id << endl; 
        lstUnivPen.add(detUniv);
    }

    // Identificamos las detecciones actuales y las nuevas
    for( i=0; i < n; i++)
    {
        detActual = lstDetActuales->at(i);

        anchoDetAct =  detActual.getAncho();
        alturaDetAct = detActual.getAltura();
        centroACtual = detActual.getCentro();
        encontro = false;

        nu = lstUnivPen.size()-1;
        for(iu=nu; iu>=0; iu--)
        {
            detUniv = lstUnivPen.get(iu);
            if ( detUniv->cara.deteccion.clase != detActual.clase )
            {
                continue;
            }

            anchoUniv = ((float)(detUniv->cara.deteccion.getAncho()))*deltaMaxX;
            alturaUniv = ((float)(detUniv->cara.deteccion.getAltura()))*deltaMaxY;

            centroUniv = detUniv->cara.deteccion.getCentro();

            deltaX = abs(centroACtual.x-centroUniv.x);
            deltaY = abs(centroACtual.y-centroUniv.y);

            if (( deltaX <= min(anchoUniv, anchoDetAct) ) &&  ( deltaY <= min(alturaDetAct, alturaUniv) ))
            {               
                encontro = true;
                
                detUniv->cara.deteccion = detActual;
                detUniv->ciclosNoDetectados = 0;

                // cout << "Reusando ya existe : " << detUniv->id << endl;

                lstUnivPen.remove(iu);
                rpta.push_back(detUniv);
                break;
            }
        }

        if ( encontro == false )
        {
            // es una nueva deteccion
            agregaDeteccion(detActual);
            detNueva = lstUniverso.getAddrUltimo();

            // cout << "Nueva deteccion : " << detNueva->id << endl;
            rpta.push_back(detNueva);
        }
    }

    // actualiza ciclose no detectados y elimna detecciones antiguas
    nu = lstUnivPen.size();
    // cout << "Quedan pendientes por validar " << nu << " detecciones" << endl;
    for(i=0; i<nu; i++)
    {
        detUniv = lstUnivPen.get(i);
        if ( detUniv->ciclosNoDetectados > maxCiclosInactivo )
        {
            // busca la deteccion en el universo
            k = lstUniverso.size()-1;
            for(j=k;j>=0;j--)
            {
                detNueva = lstUniverso.getAddr(j);
                if ( detNueva->id == detUniv->id )
                {
                    // cout << "Elimna Deteccion " << detNueva->id << endl;
                    lstUniverso.remove(j);
                    break;            
                }
            }                       
        }
        else
        {          
            // se agrega la deteccion del tracking para que se vea, aunque no fue detectado
            detUniv->ciclosNoDetectados++;
            // cout << "Reusa Deteccion no visible " << detUniv->id << endl;
            rpta.push_back(detUniv);
        }
    }
    lstUnivPen.reset();

    return rpta;
}
