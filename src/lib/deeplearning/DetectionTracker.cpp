
#include "lib/deeplearning/DetectionTracker.h"
#include <math.h>

/**
 * Constructor
 */
DetectionTracker::DetectionTracker()
{
    deltaMaxX = 0.8;
    deltaMaxY = 0.8;
    maxCiclosInactivo = 10;
    sgteId = 1;
}

/**
 * Destructor
 */
TrackedDetection::~TrackedDetection()
{

}

/**
  * Agrega una nueva deteccion al universo de detecciones
  */
 void DetectionTracker::agregaDeteccion ( DeteccionVO *det )
 {
    TrackedDetection trDet;
  
    det->id = sgteId;
    sgteId++;

    trDet.deteccion = *det;
    trDet.ciclosNoDetectados = 0;
        
    lstUniverso.add(trDet);
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
GLinkedList<DeteccionVO> DetectionTracker::analizaRapido( GLinkedList<DeteccionVO> lstDetActuales )
{
    int i,n,nu,j,k;
    shared_ptr<TrackedDetection> trDet;
    DeteccionVO detActual;
    shared_ptr<TrackedDetection> detBorrada;
    GLinkedList<DeteccionVO> rpta;

    n = lstDetActuales.size();
    nu = lstUniverso.size();

    if ( nu == 0 )
    {
        // No hay detecciones en el universo todas se agregan
        for(i=0; i < n; i++)
        {        
            detActual = lstDetActuales.get(i);    
            agregaDeteccion(&detActual);
            rpta.add(detActual);
        }

        return rpta;
    }  

    int iu,deltaX,deltaY,anchoDetAct,alturaDetAct,anchoUniv,alturaUniv;
    TrackedDetection *detUniv;
    GPoint centroACtual, centroUniv;
    GLinkedList<TrackedDetection*> lstUnivPen;
    bool encontro;

    // llena la lista de trackins pendientes de analizar
    for(i=0; i<nu; i++)
    {
        lstUnivPen.add(lstUniverso.getAddr(i));
    }

    // Identificamos las detecciones actuales y las nuevas
    for( i=0; i < n; i++)
    {
        detActual = lstDetActuales.get(i);
        anchoDetAct = ((float)(detActual.region.x2 - detActual.region.x1))*deltaMaxX;
        alturaDetAct = ((float)(detActual.region.y2 - detActual.region.y1))*deltaMaxY;
        centroACtual = detActual.region.getCenter();
        encontro = false;

        nu = lstUnivPen.size()-1;
        for(iu=nu; iu>=0; iu--)
        {
            detUniv = lstUnivPen.get(iu);
            if ( detUniv->deteccion.indiceClase != detActual.indiceClase )
            {
                continue;
            }

            anchoUniv = ((float)(detUniv->deteccion.region.x2 - detUniv->deteccion.region.x1))*deltaMaxX;
            alturaUniv = ((float)(detUniv->deteccion.region.y2 - detUniv->deteccion.region.y1))*deltaMaxY;

            centroUniv = detUniv->deteccion.region.getCenter();

            deltaX = abs(centroACtual.x-centroUniv.x);
            deltaY = abs(centroACtual.y-centroUniv.y);

            if (( deltaX <= min(anchoUniv, anchoDetAct) ) &&  ( deltaY <= min(alturaDetAct, alturaUniv) ))
            {               
                encontro = true;
                detActual.id = detUniv->deteccion.id;

                detUniv->deteccion.region = detActual.region;
                detUniv->ciclosNoDetectados = 0;


                lstUnivPen.remove(iu);
                rpta.add(detActual);
                break;
            }
        }

        if ( encontro == false )
        {
            // es una nueva deteccion
            agregaDeteccion(&detActual);
            rpta.add(detActual);
        }
    }

    // actualiza ciclose no detectados y elimna detecciones antiguas
    nu = lstUnivPen.size();
    for(i=0; i<nu; i++)
    {
        detUniv = lstUnivPen.get(i);
        if ( detUniv->ciclosNoDetectados > maxCiclosInactivo )
        {
            // busca la deteccion en el universo
            k = lstUniverso.size()-1;
            for(j=k;j>=0;j--)
            {
                detActual = lstUniverso.get(j).deteccion;
                if ( detActual.id == detUniv->deteccion.id )
                {
                    lstUniverso.remove(j);
                    break;            
                }
            }                       
        }
        else
        {          
            // se agrega la deteccion del tracking para que se vea, aunque no fue detectado
            detUniv->ciclosNoDetectados++;
            rpta.add(detUniv->deteccion);
        }
    }
    lstUnivPen.reset();

    return rpta;
}

/**
 * Analiza las detecciones actuales y le asigna un ID a cada deeccion
 * 
 *  lstDetecActuales : lista con punteros a instancias de DeteccionVO
 */
// void DetectionTracker::analizaOpt( std::shared_ptr<GVector> lstDetecActuales )
// {
//     int i,n,ndx;
//     GVector lstListasDistancias;
//     shared_ptr<GVector> lstDistancias, dataDist;
//     GVector lstDetPend, lstTrackPend;
//     shared_ptr<DeteccionVO> det,detCercano;
//     shared_ptr<TrackedDetection> trDet;
//     shared_ptr<DeteccionVO> detActual;
//     bool encontro;

//     n = lstDetecActuales->size();

//     if (  lstUniverso.size() == 0 )
//     {
//         // No hay detecciones en el universo todas se agregan
//         for(i=0; i < n; i++)
//         {
//             detActual = dynamic_pointer_cast<DeteccionVO>(lstDetecActuales->get(i));
//             agregaDeteccion(detActual);
//         }

//         return;
//     }  

//     // Guarda los trackings pendientes de procesar
//     n = lstUniverso.size();
//     for(i=0; i < n; i++)
//     {
//         trDet = dynamic_pointer_cast<TrackedDetection>(lstUniverso.get(i));
//         lstTrackPend.add(trDet);
//     }

//     // Calcula los trackins posibles por cada deteccion
//     n = lstDetecActuales->size();
//     for(i=0; i<n; i++)
//     {
//         det = dynamic_pointer_cast<DeteccionVO>(lstDetecActuales->get(i));
//         lstDistancias = calculaTrackersEquivalentes(det);
//         lstListasDistancias.add(lstDistancias);
//         lstDetPend.add(det);
//     }

//     while( lstDetPend.size() > 0 )
//     {
//         det = dynamic_pointer_cast<DeteccionVO>(lstDetPend.get(0));
//         ndx = lstDetecActuales->indexOf(det);
//         lstDistancias = dynamic_pointer_cast<GVector>(lstDistancias->get(ndx));

//         n = lstDistancias->size();
//         if ( n > 0 )
//         {
//             encontro = false;
//             for(i=0; i<n; i++)
//             {
//                 dataDist = dynamic_pointer_cast<GVector>(lstDistancias->get(i));
//                 trDet = dynamic_pointer_cast<TrackedDetection>(dataDist->get(0));
//                 ndx = lstTrackPend.indexOf(trDet);
//                 if ( ndx == -1 )
//                 {
//                     // el tracking ya fue usado
//                     continue;
//                 }
//                 detCercano = calculaDetecionCercanoTracker(trDet, lstDistancias, lstDetecActuales);
                
//                 // la deteccion es la mas cercana a trDet
//                 detCercano->id = trDet->deteccion->id;
//                 trDet->deteccion->region = detCercano->region;
//                 trDet->ciclosNoDetectados = 0;
//                 lstTrackPend.remove(trDet);
//                 lstDetPend.remove(lstDetPend.indexOf(detCercano));
//                 encontro = true;
//                 break;
                
//             }
//             if ( encontro == false )
//             {
//                 agregaDeteccion(det);
//                 lstDetPend.remove(lstDetPend.indexOf(det));
//             }
            
//         }   
//         else     
//         {
//             // la deteccion no tiene tracking equivalente
//             agregaDeteccion(det);
//             lstDetPend.remove(lstDetPend.indexOf(det));
//         }
//     }

//     // actualiza ciclose no detectados y elimna detecciones 
//     n = lstTrackPend.size();
//     for(i=0; i<n; i++)
//     {
//         trDet = dynamic_pointer_cast<TrackedDetection>(lstTrackPend.get(i));
//         if ( trDet->ciclosNoDetectados == maxCiclosInactivo )
//         {
//             lstUniverso.remove(lstTrackPend.get(i));
//         }
//         else
//         {
//             // se agrega la deteccion del tracking para que se vea, aunque no fue detectado
//             trDet->ciclosNoDetectados++;
//             lstDetecActuales->add(trDet->deteccion);
//         }
//     }
// }


/**
 * Retorna una lista de todos los TrackerDetection que podrian ser iguales
 * a una deteccion dada.
 * La lista esta ordenada de forma ascendente en base a la distancia hacia la deteccion
 * 
 * Retorna una lista de vectores con dos elementos cada uno:
 * 
 * 1ro: instancia de TrackerDetection
 * 2do: int distancia del TrackerDetecion a la deteccion pasada como parametro
 */
// shared_ptr<GVector> DetectionTracker::calculaTrackersEquivalentes( shared_ptr<DeteccionVO> deteccion )
// {
//     int i,n,dx,dy,distancia;
//     GPoint pd,pt;
//     shared_ptr<TrackedDetection> track;
//     shared_ptr<GVector> rpta = make_shared<GVector>();
//     shared_ptr<GVector> trackCandidato;

//     pd = deteccion->region.getCenter();
//     n = lstUniverso.size();
//     for( i = 0; i < n; i++ )
//     {
//         track = dynamic_pointer_cast<TrackedDetection>(lstUniverso.get(i));
//         if ( track->deteccion->indiceClase != deteccion->indiceClase )
//         {
//             continue;
//         }

//         pt = track->deteccion->region.getCenter();
//         dx = abs(pt.x - pd.x);
//         dy = abs(pt.y - pd.y);

//         if (( dx <= deltaMaxX ) && ( dy <= deltaMaxY ))
//         {
//             distancia = pd.calculaDistancia(pt);
//             trackCandidato = make_shared<GVector>();
//             trackCandidato->add(track);
//             trackCandidato->add(distancia);
//             rpta->add(trackCandidato);
//         }
//     }

//     rpta->sort( [] ( shared_ptr<GObject> p1, shared_ptr<GObject> p2) -> int {
        
//         shared_ptr<GVector> lst1 = dynamic_pointer_cast<GVector>(p1);
//         shared_ptr<GVector> lst2 = dynamic_pointer_cast<GVector>(p2);

//         int delta = lst1->getInt(1) - lst2->getInt(1);
//         return delta;
//     } );

//     return rpta;
// }

/**
 * Busca en la deteccion mas cercana a un track.
 * Solo se consideran las detecciones que no tienen ID.
 * 
 *  track : tracking para el que se busca la deteccion mas cercana
 * 
 *  lst : vector que tiene una lista de vectores generados por calculaTrackersEquivalentes
 * 
 *  lstDetec : lista de detecciones, desde ella se retorna la instancia de deteccion
 *
 *  Retorna la deteccion mas cercana
 */
// shared_ptr<DeteccionVO> DetectionTracker::calculaDetecionCercanoTracker( shared_ptr<TrackedDetection> track, shared_ptr<GVector> lst, shared_ptr<GVector> lstDetec )
// {
//     int i,n,j,k,min,dist;
//     shared_ptr<DeteccionVO> det, detCercana;
//     shared_ptr<GVector> lstDist, dataDist;
//     shared_ptr<TrackedDetection> trackedDet;

//     min = 1000000;
//     n=lstDetec->size();
//     for(i=0; i<n; i++)
//     {
//         det = dynamic_pointer_cast<DeteccionVO>(lstDetec->get(i));
//         if ( det->id > 0 )
//         {
//             // se ignora la deteccion porque ya fue identificada
//             continue;
//         }
//         lstDist = dynamic_pointer_cast<GVector>(lst->get(i));

//         k = lstDist->size();
//         for(j=0; j<k; j++)
//         {
//             dataDist = dynamic_pointer_cast<GVector>(lstDist->get(j));
//             trackedDet = dynamic_pointer_cast<TrackedDetection>(dataDist->get(0));
//             if ( trackedDet != track )
//             {
//                 continue;
//             }
//             dist = dataDist->getInt(1);
//             if ( dist < min )
//             {
//                 min = dist;
//                 detCercana = det;
//                 break;
//             }
//         }
//     }

//     return detCercana;
// }