
#include "lib/hailolib/IdentificadorPersonasHailo.h"
#include "lib/general/GLinkedList.h"
#include "lib/utils/timedate.h"

/**
 * Constructor
 */
IdentificadorPerHailo::IdentificadorPerHailo()
{
    idSgtePerDesc = 1;
    gestorThreads = new GestorThIdentificacionPersonas();
    gestorThreads->ptrIdentificadorPadre = this;
}

/**
 * Destructor
 */
IdentificadorPerHailo::~IdentificadorPerHailo()
{
    delete gestorThreads;
}

/**
 * Establece la cantidad de threads que se deben usar para identificar personas
 */
void IdentificadorPerHailo::setNumThreadsIdentificacion( int numThreads )
{
    gestorThreads->setNumThreads(numThreads);
}


/**
 *  Función para calcular la distancia euclidiana usando NEON SIMD
 */ 
float IdentificadorPerHailo::calculaDiferenciaSIMD( SIMD_TYPE *desc1, SIMD_TYPE *desc2 ) 
{
    size_t size = NUM_ELEMS_DESC_FACIAL;

    SIMD_TYPE *val1 = desc1;
    SIMD_TYPE *val2 = desc2;
    
    size_t simd_size = size / NUM_ELEMS_DESC_FACIAL;  // Cada registro SIMD maneja 4 floats
    SIMD_VECTOR sum_vec = DUP_SIMD(0.0f);  // Inicializa a cero el acumulador SIMD
    
    // Procesamiento en bloques de 4 floats
    for (size_t i = 0; i < simd_size * NUM_ELEMS_DESC_FACIAL; i += NUM_ELEMS_DESC_FACIAL) {
        SIMD_VECTOR v1 = LOAD_SIMD(&val1[i]);    // Carga 4 elementos de arr1
        SIMD_VECTOR v2 = LOAD_SIMD(&val2[i]);    // Carga 4 elementos de arr2
        SIMD_VECTOR diff = SUB_SIMD(v1, v2);    // Resta los vectores
        SIMD_VECTOR sq = MUL_SIMD(diff, diff);  // Eleva al cuadrado
        sum_vec = ADD_SIMD(sum_vec, sq);        // Acumula
    }

    // Reduce el vector SIMD a un escalar
    SIMD_TYPE result_array[NUM_ELEMS_DESC_FACIAL];
    STORE_SIMD(result_array, sum_vec);
    SIMD_TYPE sum = 0;
    
    for(int i=0; i<NUM_ELEMS_DESC_FACIAL; i++)
        sum+= sum_vec[i];

    // Procesa los elementos restantes (si el tamaño no es múltiplo de 4)
    for (size_t i = simd_size * NUM_ELEMS_DESC_FACIAL; i < size; ++i) {
        SIMD_TYPE diff = val1[i] - val2[i];
        sum += diff * diff;
    }

    #ifdef USE_INT16
        return (float)sum; /// como se trabaja con enteros no es necesaria la raiz
    #else
        return std::sqrt((float)sum);  // Devuelve la raíz cuadrada del resultado
    #endif
}

/**
 * Dado un descriptor facial, se retorna una referencia en caso
 * se encuentre a la descripcion de una persona externa
 * 
 *      descriptor: 
 *          Descriptor facial que se evalua
 * 
 *      tolerancia:
 *          Valor limite para considerar que una persona conocida coincide
 *          con el descriptor, si la distancia ecuclidiana es mayor no es la misma persona
 * 
 *      distEucli :
 *          Puntero en el que se guarda la distancia euclidiana entre el descriptor 
 *          y la persona detectada
 *      
 *      userDistEucli :
 *          Indica si se debe o no usar la distancia euclideana como criterio para buscar
 * 
 */
DescPersonaExterno* IdentificadorPerHailo::buscaPersonaCon( SIMD_TYPE *descriptor, float tolerancia, float *distEucli, bool usarDistEucli )
{
    DescPersonaExterno *ptr;
    
    // ptr = encuentraPerCerCos(&lstPerIdentificadas, descriptor, tolerancia, &indice, distEucli);
    ptr = gestorThreads->encuentraPerCercana(descriptor, tolerancia, distEucli, usarDistEucli);
        
    return ptr;
}

/**
 * Dado un descriptor facial, se retorna una referencia en caso
 * se encuentre a la descripcion de una persona no identiciada o desconocida
 *
 *      descriptor: 
 *          Descriptor facial que se evalua
 * 
 *      tolerancia:
 *          Valor limite para considerar que una persona conocida coincide
 *          con el descriptor, si la distancia ecuclidiana es mayor no es la misma persona
 * 
 *      distEucli :
 *          Puntero en el que se guarda la distancia euclidiana entre el descriptor 
 *          y la persona detectada
 * 
 *      usarDistEuclideana:
 *          Indica si se usa distancia euclideana o no
 */
DescPersonaExterno* IdentificadorPerHailo::buscaPersonaDesc( SIMD_TYPE *descriptor, float tolerancia, float *distEucli, bool usarDistEuclideana )
{
    int indice;
    DescPersonaExterno *ptr;

    if ( usarDistEuclideana == true )
    {
        ptr = encuentraPerCercana(&lstPerNoIdent, descriptor, tolerancia, &indice, distEucli);
    }
    else 
    {
        ptr = encuentraPerCerCos(&lstPerNoIdent, descriptor, tolerancia, &indice, distEucli);
    }

    return ptr;
}


/**
 * Elimina las personas no reconocidas que tengan mas de una cantidad
 * de segundos que no son reconocidas
 */
void IdentificadorPerHailo::eliminaDesAntiguos(long tiempoMaxNoReconocido)
{
    int i,n,numElem;
    long long delta,ahora;
    DescPersonaExterno *persona;
    
    numElem = 0;
    ahora = TimeDateUtils::getDateTimeMs();
    n= lstPerNoIdent.size();
    i = 0;
    while( i < n )
    {
        persona = lstPerNoIdent.getAddr(i);
        delta = ahora - persona->ultimaFechaDetectada;
        if ( (delta/1000) > tiempoMaxNoReconocido )
        {
            numElem++;
            lstPerNoIdent.remove(i);
            n--;
        }
        else
        {
            i++;
        }
    }
}


/**
* Calcula el descriptor facial mas cernado de una lista de personas
* conocidas con una descriptor de alguien recien encontrado
* 
*       lstUniv:
*           Vector con descriptores de personas conocidas
* 
*       desc: 
*           Desctriptor facial que se usa para bucar a la persona conocida
* 
*       tolerancia:
*           Valor maximo para conciderar que una persona coincide con el vector desc2
*           si la distancia euclideana es mayor a tolerancia se ignora
* 
        indice :
*           Puntero a un entero en el que se retorna el indice de la descripcion
*           de persona externa encontrada
*
*       distEucli :
*           Puntero en el que se guarda la distancia euclidiana entre el descriptor 
*           y la persona detectada
* 
*   Retorna un puntero a la definicion de la persona externa dentro de lstUniv
*   si no se encuentra se retorna NULL
*/ 
DescPersonaExterno* IdentificadorPerHailo::encuentraPerCercana( GLinkedList<DescPersonaExterno> *lstUniv, SIMD_TYPE *desc, float tolerancia, int *indice, float *distEucli )
{
    float distFinal, distMin = 10000;
    int indiceMin = -1;
    int nUniv = lstUniv->size();
    SIMD_TYPE *val1, *val2;
    
    GLinkedListNode<DescPersonaExterno> *ptrNodo;
    GLinkedListNode<DescPersonaExterno> *ptrNodoMin = NULL;

    if ( lstUniv->size() == 0 )
        return NULL;
    
    // referencia a los datos del descriptor
    val2 = desc;

    // variables usadas para el bucle que calcula distancia euclideana
    
    // numero de elementos de los descriptores
    size_t size = NUM_ELEMS_DESC_FACIAL;

    // numero de veces que se debe repetir el bucle en base a la 
    // cantidad de elementos que tiene cada vector SIMD
    size_t simd_size = size / NUM_ELEMS_DESC_FACIAL;  
    size_t numIteraciones = simd_size * NUM_ELEMS_DESC_FACIAL;

    // Recorre todos los nodos
    ptrNodo = lstUniv->getNode(0);
    for(int iUniv=0; iUniv<nUniv; iUniv++ )
    {
        val1 = ptrNodo->data.vecDescripcion;    

        // Inicio del codigo que calcula la diferencia entre el arreglo dataUniv y desc              
        
        SIMD_VECTOR sum_vec = DUP_SIMD(0.0f);  // Inicializa a cero el acumulador SIMD
        
        // Procesamiento en bloques de 4 floats
        for (size_t i = 0; i < numIteraciones; i += NUM_ELEMS_DESC_FACIAL) {
            SIMD_VECTOR v1 = LOAD_SIMD(&val1[i]);    // Carga N elementos de arr1
            SIMD_VECTOR v2 = LOAD_SIMD(&val2[i]);    // Carga N elementos de arr2
            SIMD_VECTOR diff = SUB_SIMD(v1, v2);    // Resta los vectores
            SIMD_VECTOR sq = MUL_SIMD(diff, diff);  // Eleva al cuadrado
            sum_vec = ADD_SIMD(sum_vec, sq);        // Acumula            
        }

        // Reduce el vector SIMD a un escalar
        SIMD_TYPE result_array[NUM_ELEMS_DESC_FACIAL];
        STORE_SIMD(result_array, sum_vec);
        SIMD_TYPE sum = 0;
        
        for(int i=0; i<NUM_ELEMS_DESC_FACIAL; i++)
            sum+= sum_vec[i];

        // Procesa los elementos restantes (si el tamaño no es múltiplo de 4)
        for (size_t i = numIteraciones; i < size; ++i) {
            SIMD_TYPE diff = val1[i] - val2[i];
            sum += diff * diff;
        }

        #ifdef USE_INT16
            distFinal = (float)sum; /// como se trabaja con enteros no es necesaria la raiz
        #else
            distFinal = std::sqrt((float)sum);  // Devuelve la raíz cuadrada del resultado
        #endif

        // cout << "DistFinal : " << distFinal << endl;
        // Termino de calcula la distancia Euclidiana

        if (( distFinal < distMin ) && ( distFinal < tolerancia ))
        {
            // guardamos la distancia porque es menor a la menor actual
            distMin = distFinal;
            indiceMin = iUniv;
            ptrNodoMin = ptrNodo;
        }

        // cambia al siguiente nodo de la lista
        ptrNodo = ptrNodo->next;
    }

    *indice = indiceMin;
    *distEucli = distMin;

    if ( ptrNodoMin == NULL ) 
        return NULL;

    return &ptrNodoMin->data;
}
        

/**
 * Carga la BD de personas conocidas
 * 
 *      path:
 *          Ruta del archivo que se lee
 * 
 *      unificarDesc:
 *          Indica si se deben unificar los descriptores con el mismo ID externo
 */
void IdentificadorPerHailo::cargarpPerConocidas( string path, bool unificarDesc )
{
    resetUniverso();

    std::ifstream file(path);

    if ( !file.is_open() )
    {
        cout << "No se pudo leer el archivo con rostros conocidos" << endl;
        return;
    }

    string line,nombre,dato,idExterno;    
    int pos,numPer;
    DescPersonaExterno descPromedio; 
    GLinkedList<DescPersonaExterno> lstDescPersona;

    idExterno = "";
    numPer = 0;
    while( getline(file, line))
    {       
        if ( line.length() == 0 )
            continue;
        if ( line.at(0) == '#' )
            continue;
        try
        {
            DescPersonaExterno desc;    

            stringstream ss(line);
            pos = -2;
            int indice=0;
            while( pos < NUM_ELEMS_DESC_FACIAL )
            {
                getline(ss, dato, ',');
                if ( pos == -2 )
                {
                    desc.nombre = dato;
                }
                else
                if ( pos == -1 )
                {
                    desc.id = dato;
                }
                else
                {
                    desc.vecDescripcion[indice++]=stof(dato);
                }
                pos++;                
            }
            if ( indice != NUM_ELEMS_DESC_FACIAL )
                    continue;
            if ( unificarDesc == false )
            {                
                // lstPerIdentificadas.add(desc);                
                gestorThreads->addPersona(desc);
                numPer++;
            }
            else
            {               
                // Se debe unificar los IDs
                if ( desc.id.compare(idExterno) != 0 )
                {
                    // es una nueva persona
                    if ( idExterno.length() > 0 )
                    {                        
                        if ( lstDescPersona.size() > 1 )
                        {
                            // se debe calcular el promedio
                            descPromedio = calculaPromedio(&lstDescPersona);
                            lstPerIdentificadas.add(descPromedio);
                        }
                        else
                        {
                            // solo hay una descripcion, no es necesario calcular promedio
                            lstPerIdentificadas.add(lstDescPersona.get(0));
                        }
                        lstDescPersona.reset();
                    }
                    idExterno = desc.id;
                    // lstDescPersona.add(desc);
                    gestorThreads->addPersona(desc);
                    numPer++;
                }
                else
                {
                    // lstDescPersona.add(desc);
                    gestorThreads->addPersona(desc);
                    numPer++;
                }
            }                        
        }
        catch(const std::exception& e)
        {
            cout << "Error al leer el descriptor de rostro conocido " <<  line  << endl;
        }        
    }
    file.close();

    cout << "Se leyeron " << numPer << " rostros conocidos" << endl;
}

/**
 * Reinciar universo de personas
 */
void IdentificadorPerHailo::resetUniverso()
{
    // Reinicia IDs para desconocidos
    idSgtePerDesc = 1;

    // Limpia listas internas
    lstPerIdentificadas.reset();
    lstPerNoIdent.reset();

    // Limpia las personas repartidas entre los threads
    if (gestorThreads != nullptr)
    {
        gestorThreads->resetPersonas();
    }
}

/**
 * Retorna el promedio de los descriptores
 */
DescPersonaExterno IdentificadorPerHailo::calculaPromedio( GLinkedList<DescPersonaExterno> *lst )
{
    int i,n,j;
    DescPersonaExterno descPrimero,desc;

    descPrimero = lst->get(0);
    n = lst->size();
    for(i=1;i<n;i++)
    {
        desc = lst->get(i);
        for(j=0;j<NUM_ELEMS_DESC_FACIAL;j++)
        {
            descPrimero.vecDescripcion[j]+= desc.vecDescripcion[j];
        }
    }

    float numElem = n;
    for(j=0;j<NUM_ELEMS_DESC_FACIAL;j++)
    {
        descPrimero.vecDescripcion[j]/= numElem;
    }

    return descPrimero;
}


/**
* Calcula el descriptor facial mas cernado de una lista de personas
* conocidas con una descriptor de alguien recien encontrado
* 
*       lstUniv:
*           Vector con descriptores de personas conocidas
* 
*       desce: 
*           Desctriptor facial que se usa para bucar a la persona conocida
* 
*       tolerancia:
*           Valor maximo para conciderar que una persona coincide con el vector desc2
*           si la similaridad de coceno es mayor a tolerancia se considera
* 
*       indice :
*           Puntero a un entero en el que se retorna el indice de la descripcion
*           de persona externa encontrada
* 
*      distEucli :
*          Puntero en el que se guarda la distancia euclidiana entre el descriptor 
*          y la persona detectada
* 
*   Retorna un puntero a la definicion de la persona externa dentro de lstUniv
*   si no se encuentra se retorna NULL
*/ 
DescPersonaExterno*  IdentificadorPerHailo::encuentraPerCerCos( GLinkedList<DescPersonaExterno> *lstUniv, SIMD_TYPE *desc, float tolerancia, int *indice, float *distEucli )
{
    float distFinal, distMax = 0;
    int indiceMin = -1;
    int nUniv = lstUniv->size();
    SIMD_TYPE *val1, *val2;
    
    GLinkedListNode<DescPersonaExterno> *ptrNodo;
    GLinkedListNode<DescPersonaExterno> *ptrNodoMin = NULL;

    if ( lstUniv->size() == 0 )
        return NULL;
    
    // referencia a los datos del descriptor
    val2 = desc;
    
    // numero de elementos de los descriptores
    size_t size = NUM_ELEMS_DESC_FACIAL;

    float dot_product;
    float norm1;

    // Calcula normalizacion L2 para rostro desconocido
    // norm1 = 0.0;
    // for (size_t i = 0; i < size; i ++) 
    // {
    //     norm1+= val2[i] * val2[i];
    // }
    // norm1 = sqrt(norm1);
    // for (size_t i = 0; i < size; i ++) 
    // {
    //     val2[i] /= norm1;
    // }
    
    // Recorre todos los nodos
    ptrNodo = lstUniv->getNode(0);
    for(int iUniv=0; iUniv<nUniv; iUniv++ )
    {
        val1 = ptrNodo->data.vecDescripcion;          

        // validamos si se debe calcular la normalizacion L2 del descriptor conocido
        if ( ptrNodo->data.norm_coceno_calculado == false )
        {
            norm1 = 0.0;
            for(size_t i=0;i<size;i++)
            {   
                norm1+= val1[i] * val1[i];
            }
            norm1 = sqrt(norm1);
            for(size_t i=0;i<size;i++)
            {   
                val1[i] /= norm1;
            }         
            ptrNodo->data.norm_coseno = norm1;
            ptrNodo->data.norm_coceno_calculado = true;            
        }
            
        dot_product = 0.0;
        
        // Procesamiento en bloques de 4 floats
        for (size_t i = 0; i < size; i ++) 
        {
            dot_product += val1[i] * val2[i];        
        }
        
        distFinal = dot_product;

        // cout << "DistFinal : " << distFinal << endl;
        // Termino de calcula la distancia Euclidiana

        if (( distFinal > distMax ) && ( distFinal > tolerancia ))
        {
            // guardamos la distancia porque es menor a la menor actual
            distMax = distFinal;
            indiceMin = iUniv;
            ptrNodoMin = ptrNodo;
        }

        // cambia al siguiente nodo de la lista
        ptrNodo = ptrNodo->next;
    }

    *indice = indiceMin;
    *distEucli = distMax;

    if ( ptrNodoMin == NULL ) 
        return NULL;

    return &ptrNodoMin->data;
}


/**
 * Contructor
 */
GestorThIdentificacionPersonas::GestorThIdentificacionPersonas()
{
    numPersonas = 0;
    threadsIniciados = false;
}

/**
 * Destructor
 */
GestorThIdentificacionPersonas::~GestorThIdentificacionPersonas()
{
    int i,n;
    ThIdentificadorPersonas *ptrThread;

    n = lstThreads.size();
    for(i=0;i<n;i++)
    {
        ptrThread = lstThreads.get(i);
        delete ptrThread;
    }
}

/**
 * Establece el numreo de thread
 */
void GestorThIdentificacionPersonas::setNumThreads(int num)
{
    ThIdentificadorPersonas *th;
    for(int i=0; i<num; i++)
    {
        th = new ThIdentificadorPersonas();
        th->gestorPadre = this;
        th->nucleoAsociado = i;
        th->esDemonio = true;

        lstThreads.add(th);
    }
}

/**
 * Meotodo llamado por un thread indicando que ha terminado de hacer los calculos
 */
void GestorThIdentificacionPersonas::notificaFinCalculo()
{
    {
        auto lock = getLock();
        numThFinalizado++;
        cv.notify_one();
    }
}


/**
 * Calcula el descriptor facial mas cernado de una lista de personas
 * conocidas con una descriptor de alguien recien encontrado
 *
 *       lstUniv:
 *           Vector con descriptores de personas conocidas
 *
 *       desce:
 *           Desctriptor facial que se usa para bucar a la persona conocida
 *
 *       tolerancia:
 *           Valor maximo para conciderar que una persona coincide con el
 *           vector desc2 si la distancia euclideana es mayor a tolerancia se ignora
 *
 *      distEucli :
 *          Puntero en el que se guarda la distancia euclidiana entre el
 *          descriptor y la persona detectada
 * 
 *      usarDistEuclidiana :
 *          Indica si se debe usar distancia euclidiana (true) o similaridad de coceno (false)
 *
 *   Retorna un puntero a la definicion de la persona externa dentro de
 * lstUniv si no se encuentra se retorna NULL
 */
DescPersonaExterno *GestorThIdentificacionPersonas::encuentraPerCercana(SIMD_TYPE *desc2,float tolerancia, float *distEucli, bool usarDistEuclidiana )
{
    int n = lstThreads.size();
    ThIdentificadorPersonas *ptrThread;
    float norm1;
    int size=NUM_ELEMS_DESC_FACIAL;

    // normaliza el decriptor
    norm1 = 0.0;
    for (size_t i = 0; i < size; i ++) 
    {
        norm1+= desc2[i] * desc2[i];
    }
    norm1 = sqrt(norm1);
    for (size_t i = 0; i < size; i ++) 
    {
        desc2[i] /= norm1;
    }

    if ( threadsIniciados == false )
    {
        // inicia los threads para que esten a la espera de ordenes
        for(int i=0;i<n; i++)
        {
            ptrThread = lstThreads.get(i);            
            ptrThread->start();
        }
        threadsIniciados = true;
    }

    // solisita a los threads hijos para que hagan la busqueda    
    numThFinalizado = 0;
    for(int i=0;i<n; i++)
    {
        ptrThread = lstThreads.get(i);            
        ptrThread->buscarPersona(desc2, tolerancia, usarDistEuclidiana);
    }        
        
    // Esperar hasta que cada thread notifique que termino de hacer su busqueda
    {
        auto lock = getLock();         
        cv.wait(lock, [this]() { return (numThFinalizado >= lstThreads.size() ); });        
    }

    // busca el thread con la mayor cercania
    float distanciaMinima;
    float distanciaThread;
    DescPersonaExterno *rpta = NULL;

    if ( usarDistEuclidiana == true ) distanciaMinima = 1000;
    else distanciaMinima = 0;
     
    for(int i=0;i<n; i++)
    {
        ptrThread = lstThreads.get(i);  
        if ( ptrThread->getEncontro() )
        {
            distanciaThread = ptrThread->getDistanciaEncontrada();
            if ( usarDistEuclidiana == true )
            {
                if ( distanciaThread < distanciaMinima )
                {
                    distanciaMinima = distanciaThread;
                    *distEucli = distanciaMinima;
                    rpta = ptrThread->getCoincidencia();
                }
            }
            else
            {
                if ( distanciaThread > distanciaMinima )
                {
                    distanciaMinima = distanciaThread;
                    *distEucli = distanciaMinima;
                    rpta = ptrThread->getCoincidencia();
                }
            }
        }
    }        
        
    return rpta;
}

/**
 * Metodo que se debe invocar cuando se desea iniciar un codigo critico
 * que solo un proceso a la vez debe hacer
 */
void GestorThIdentificacionPersonas::mtxBloquea()
{
    mtx.lock();
}

/**
 * Metodo que se debe invocar cuando se indica que ya se termino de ejecutar
 * un codigo critico
 */
void GestorThIdentificacionPersonas::mtxLibera()
{
    mtx.unlock();
}

/**
 * Retorna un lock sobre el mutex
 */
std::unique_lock<std::mutex> GestorThIdentificacionPersonas::getLock()
{
    return std::unique_lock<std::mutex>(mtx);
}


/**
 * Agrega una persona a la lista de personas conocidas sobre las que se haria la busqueda
 */
void GestorThIdentificacionPersonas::addPersona( DescPersonaExterno persona )
{
    int indice = numPersonas % lstThreads.size();
    ThIdentificadorPersonas *ptrThread = lstThreads.get(indice);

    // normaliza el descriptor
    float norm1;
    int size = NUM_ELEMS_DESC_FACIAL;
    SIMD_TYPE *val1 = persona.vecDescripcion;

    norm1 = 0.0;
    for(size_t i=0;i<size;i++)
    {   
        norm1+= val1[i] * val1[i];
    }
    norm1 = sqrt(norm1);
    for(size_t i=0;i<size;i++)
    {   
        val1[i] /= norm1;
    }         
    persona.norm_coseno = norm1;
    persona.norm_coceno_calculado = true;     

    ptrThread->addPersonaExterna(persona);
    numPersonas++;
}


void GestorThIdentificacionPersonas::resetPersonas()
{
    numPersonas = 0;

    int n = lstThreads.size();
    for (int i = 0; i < n; ++i)
    {
        ThIdentificadorPersonas *th = lstThreads.get(i);
        th->resetPersonas();
    }
}

/**
 * Constructor
 */
ThIdentificadorPersonas::ThIdentificadorPersonas()
{
    hayDatos = false;
    indiceEncontrado = -1;
    distanciaCalculada = -1;
}
\
void ThIdentificadorPersonas::resetPersonas()
{
    auto lock = getLock();
    lstDatos.reset();
    indiceEncontrado     = -1;
    distanciaCalculada   = -1;
}

/**
 * Constructor
 */
ThIdentificadorPersonas::~ThIdentificadorPersonas()
{
    if ( isFinalizado() == false )
    {
        finalizar();
        while( isFinalizado() == false )
        {
            sleepMS(10);            
        }
    }    
}


/**
 * Bucle del thread
 */
void ThIdentificadorPersonas::runThread()
{
    while( isFinalizado() == false )
    {
        auto lock = getLock();

        cv.wait(lock, [this]() {  return hayDatos || isFinalizado(false);  }  );
               
        if ( isFinalizado(false) == true )
            break;
        
        if ( usarDistanciaEuclidiana == true )
        {
            gestorPadre->ptrIdentificadorPadre->encuentraPerCercana(&lstDatos, descBuscado, tolerancia, &indiceEncontrado, &distanciaCalculada);
        }
        else
        {
            gestorPadre->ptrIdentificadorPadre->encuentraPerCerCos(&lstDatos, descBuscado, tolerancia, &indiceEncontrado, &distanciaCalculada);
        }
        
        gestorPadre->notificaFinCalculo();     
        hayDatos = false;   
    }
    lstDatos.reset();
}

/**
 * Busca una persona
 * 
 *      descriptor :
 *          Vector descriptor del rostro de una persona que se busca
 * 
 * 
 *      tolerancie :
 *          Tolerancia en la busqueda.
 * 
 * 
 *      usarDistEuclidiana>
 *          Indica si se debe usar la distancia ecuclidiana (true) o la similaridad de coceno (false)
 *          para hacer la busqueda
 *      
 * 
 */
void ThIdentificadorPersonas::buscarPersona(SIMD_TYPE *descriptor, float toleranciaBus, bool usarDistEuclidiana )
{
    {
        auto lock = getLock();

        indiceEncontrado = -1;
        descBuscado = descriptor;
        tolerancia = toleranciaBus;
        hayDatos = true;
        usarDistanciaEuclidiana = usarDistEuclidiana;

        cv.notify_one();
    }
}

/**
 * Agrega una persona externa al thread
 */
void ThIdentificadorPersonas::addPersonaExterna( DescPersonaExterno persona)
{
    {
        auto lock = getLock();
        lstDatos.add(persona);
    }
}

/**
 * Indica si encontro o no una coincidencia
 */
bool ThIdentificadorPersonas::getEncontro()
{
    {
        auto lock = getLock();

        if ( indiceEncontrado == -1 ) return false;
        return true;
    }
}

/**
 * Retorna una referencia a la coincidencia
 */
DescPersonaExterno *ThIdentificadorPersonas::getCoincidencia()
{
    {
        auto lock = getLock();
        
        if ( indiceEncontrado == -1 ) return NULL;
        return lstDatos.getAddr(indiceEncontrado);
    }
}

/**
 * Retorna la distancia encontrada
 */
float ThIdentificadorPersonas::getDistanciaEncontrada()
{
    {
        auto lock = getLock();

        return distanciaCalculada;
    }
}

/**
 * Se invoca para indicar que el Thread debe finalizar su bucle de ejecucion
 */
void ThIdentificadorPersonas::finalizar()
{
    GThread::finalizar();    
    cv.notify_one();
}


