
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/general/GLinkedList.h"
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
    deltaMaxX = 0.5;
    deltaMaxY = 0.5;
    maxCiclosInactivo = 0;
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
            
            descPerUniv = detUniv->cara.identificador.getDatosPerIden();
            if ( descPerUniv == NULL ) 
                continue;
                
            if (( descPerActual->anonimo== descPerUniv->anonimo ) &&  ( descPerActual->id.compare(descPerUniv->id) == 0 ))
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
 *   Analiza las detecciones actualesy reconocidas, les asigna un ID unico
 * para poder hacer un tracking, pero principalmente para poder llevar
 * estadisticas del rostro y agrupar las detecciones adicionalmente aplica un algoritmo
 * de tracking KLT para poder identificar rostros que en la imagen anterior fueron identificados
 * pero en la actual no, pero existen rostro sin identificacion o anonimos cercano 
 */
vector<TrackedDetectionHailo *> DetectionTrackerHailo::analizaPorIdentificacionYTracker( vector<TrackedDetectionHailo *> *lstDetActuales, GImage *imagenVisorActual, GImage *imagenPreviaVisor, float factorXVisor, float factorYVisor  )
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

    GLinkedList<TrackedDetectionHailo *> lstUnivPen, lstNuevos;
    GLinkedList<int>lstUnivPenIndices;
    TrackedDetectionHailo *detUniv;
    bool encontro;
    cv::Rect rc;

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
            // es un no reconocido incluso sin descriptor facial pues no se desea
            // reportar desconocidos
            lstNuevos.add(detActual);
            continue;
        }
        
        encontro = false;
        nu = lstUnivPen.size()-1;
        for(iu=nu; iu>=0; iu--)
        {
            detUniv = lstUnivPen.get(iu);
            
            descPerUniv = detUniv->cara.identificador.getDatosPerIden();
            if ( descPerUniv == NULL )
            {
                continue;
            }

            if (( descPerActual->anonimo== descPerUniv->anonimo ) &&  ( descPerActual->id.compare(descPerUniv->id) == 0 ))
            {
                // Encontramos que la persona actual o nueva coincide con una del universo
                idenPersona = detActual->cara.identificador.getUltimaIdentificacion();
                
                detUniv->cara.fotoCara = detActual->cara.fotoCara;
                detUniv->cara.deteccion = detActual->cara.deteccion;
                std::memcpy(detUniv->cara.descriptor, detActual->cara.descriptor, NUM_ELEMS_DESC_FACIAL*sizeof(SIMD_TYPE));
                detUniv->cara.identificador.agregaIdentif(descPerActual, idenPersona, idenPersona.fecDet);
                detUniv->ciclosNoDetectados = 0;

                if ( detUniv->tracker.empty() == false )
                {
                    // si el objeto universal tenia tracking activo, se elimina pues la coinciencia de
                    // descriptor deprecia al tracker
                    detUniv->tracker.release();
                }                

                lstUnivPen.remove(iu);
                lstUnivPenIndices.remove(iu);
                encontro = true;

                idenPersona = detActual->cara.identificador.getUltimaIdentificacion();
                rpta.push_back(detUniv);

                break;
            }
        }

        
        if ( encontro == false )
        {
            // la deteccion es nueva
            descPerActual = detActual->cara.identificador.getDatosPerIden();
            if ( descPerActual->anonimo == false )
            {
                detActual->id = sgteId;
                sgteId++;            
                lstUniverso.add(*detActual);
                detActual = lstUniverso.getAddrUltimo();

                if ( detActual->tracker.empty() == false )                
                    detActual->tracker.release();
                                     
                rpta.push_back(detActual);
            }
            else
            {
                // los anonimos se guardan para ver si hay coincidencia visual
                lstNuevos.add(detActual);
            }           
        }
    }

    nu = lstUnivPen.size();
    n = lstNuevos.size();
    if (( n >= 1 ) && ( nu >= 1 ))
        analizaDetNuevasAnonimas(&rpta,&lstUnivPen,&lstNuevos,&lstUnivPenIndices, imagenVisorActual, imagenPreviaVisor, factorXVisor, factorYVisor);

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
            detUniv->cara.identificador.reset();
            if ( detUniv->tracker.empty() == false )
                detUniv->tracker.release();

            lstUniverso.remove(i);
        }
        else
        {
            // hacemos tracking de la imagen que aun esta pendiente
            if ( detUniv->tracker.empty() == true ) 
            {
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

            rc = detUniv->trackerBox;
            rc.x = (int)(((float) rc.x) * factorXVisor);
            rc.y = (int)(((float) rc.y) * factorYVisor);
            rc.width = (int)(((float) rc.width) * factorXVisor);
            rc.height = (int)(((float) rc.height) * factorYVisor);            

            detUniv->cara.deteccion.ptoSupIzq.x = rc.x;
            detUniv->cara.deteccion.ptoSupIzq.y = rc.y;
            detUniv->cara.deteccion.ptoInfDer.x = rc.x+rc.width;
            detUniv->cara.deteccion.ptoInfDer.y = rc.y+rc.height;

            rpta.push_back(detUniv);
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
