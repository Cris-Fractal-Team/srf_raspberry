

#include "lib/hailolib/IdentificadorPersonasHailo.h"

/**
 * Constructor
 */
IdentificadorPerHailo::IdentificadorPerHailo()
{
    idSgtePerDesc = 1;
}

/**
 *  Función para calcular la distancia euclidiana usando NEON SIMD
 */ 
float IdentificadorPerHailo::calculaDiferenciaSIMD( vector<SIMD_TYPE>*desc1, vector<SIMD_TYPE>*desc2 ) 
{
    size_t size = desc1->size();

    SIMD_TYPE *val1 = desc1->data();
    SIMD_TYPE *val2 = desc2->data();
    
    size_t simd_size = size / NUMELEM_VECTOR_SIMD;  // Cada registro SIMD maneja 4 floats
    SIMD_VECTOR sum_vec = DUP_SIMD(0.0f);  // Inicializa a cero el acumulador SIMD
    
    // Procesamiento en bloques de 4 floats
    for (size_t i = 0; i < simd_size * NUMELEM_VECTOR_SIMD; i += NUMELEM_VECTOR_SIMD) {
        SIMD_VECTOR v1 = LOAD_SIMD(&val1[i]);    // Carga 4 elementos de arr1
        SIMD_VECTOR v2 = LOAD_SIMD(&val2[i]);    // Carga 4 elementos de arr2
        SIMD_VECTOR diff = SUB_SIMD(v1, v2);    // Resta los vectores
        SIMD_VECTOR sq = MUL_SIMD(diff, diff);  // Eleva al cuadrado
        sum_vec = ADD_SIMD(sum_vec, sq);        // Acumula
    }

    // Reduce el vector SIMD a un escalar
    SIMD_TYPE result_array[NUMELEM_VECTOR_SIMD];
    STORE_SIMD(result_array, sum_vec);
    SIMD_TYPE sum = 0;
    
    for(int i=0; i<NUMELEM_VECTOR_SIMD; i++)
        sum+= sum_vec[i];

    // Procesa los elementos restantes (si el tamaño no es múltiplo de 4)
    for (size_t i = simd_size * NUMELEM_VECTOR_SIMD; i < size; ++i) {
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
 * 
 */
DescPersonaExterno* IdentificadorPerHailo::buscaPersonaCon( vector<SIMD_TYPE> *descriptor, float tolerancia, float *distEucli )
{
    int indice;
    DescPersonaExterno *ptr;

    ptr = encuentraPerCerCos(&lstPerIdentificadas, descriptor, tolerancia, &indice, distEucli);
        
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
 */
DescPersonaExterno* IdentificadorPerHailo::buscaPersonaDesc( vector<SIMD_TYPE> *descriptor, float tolerancia, float *distEucli )
{
    int indice;
    DescPersonaExterno *ptr;

    ptr = encuentraPerCercana(&lstPerIdentificadas, descriptor, tolerancia, &indice, distEucli);

    return ptr;
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
DescPersonaExterno* IdentificadorPerHailo::encuentraPerCercana( GLinkedList<DescPersonaExterno> *lstUniv, vector<SIMD_TYPE> *desc, float tolerancia, int *indice, float *distEucli )
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
    val2 = desc->data();

    // variables usadas para el bucle que calcula distancia euclideana
    
    // numero de elementos de los descriptores
    size_t size = desc->size();

    // numero de veces que se debe repetir el bucle en base a la 
    // cantidad de elementos que tiene cada vector SIMD
    size_t simd_size = size / NUMELEM_VECTOR_SIMD;  
    size_t numIteraciones = simd_size * NUMELEM_VECTOR_SIMD;

    // Recorre todos los nodos
    ptrNodo = lstUniv->getNode(0);
    for(int iUniv=0; iUniv<nUniv; iUniv++ )
    {
        val1 = ptrNodo->data.vecDescripcion.data();    

        // Inicio del codigo que calcula la diferencia entre el arreglo dataUniv y desc              
        
        SIMD_VECTOR sum_vec = DUP_SIMD(0.0f);  // Inicializa a cero el acumulador SIMD
        
        // Procesamiento en bloques de 4 floats
        for (size_t i = 0; i < numIteraciones; i += NUMELEM_VECTOR_SIMD) {
            SIMD_VECTOR v1 = LOAD_SIMD(&val1[i]);    // Carga N elementos de arr1
            SIMD_VECTOR v2 = LOAD_SIMD(&val2[i]);    // Carga N elementos de arr2
            SIMD_VECTOR diff = SUB_SIMD(v1, v2);    // Resta los vectores
            SIMD_VECTOR sq = MUL_SIMD(diff, diff);  // Eleva al cuadrado
            sum_vec = ADD_SIMD(sum_vec, sq);        // Acumula            
        }

        // Reduce el vector SIMD a un escalar
        SIMD_TYPE result_array[NUMELEM_VECTOR_SIMD];
        STORE_SIMD(result_array, sum_vec);
        SIMD_TYPE sum = 0;
        
        for(int i=0; i<NUMELEM_VECTOR_SIMD; i++)
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

        cout << "DistFinal : " << distFinal << endl;
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
 */
void IdentificadorPerHailo::cargarpPerConocidas( string path )
{
    std::ifstream file(path);

    if ( !file.is_open() )
    {
        cout << "No se pudo leer el archivo con rostros conocidos" << endl;
        return;
    }

    string line,nombre,dato;    
    float *datos;
    int pos;

    while( getline(file, line))
    {       
        if ( line.at(0) == '#' )
            continue;
        try
        {
            DescPersonaExterno desc;    

            stringstream ss(line);
            pos = -2;
            while( pos < 512 )
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
                    desc.vecDescripcion.push_back(stof(dato));
                }
                pos++;
            }
            if ( desc.vecDescripcion.size() == 512 )
                lstPerIdentificadas.add(desc);
        }
        catch(const std::exception& e)
        {
            cout << "Error al leer el descriptor de rostro conocido " <<  line  << endl;
        }        
    }
    file.close();

    cout << "Se leyeron " << lstPerIdentificadas.size() << " rostros conocidos" << endl;
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
DescPersonaExterno*  IdentificadorPerHailo::encuentraPerCerCos( GLinkedList<DescPersonaExterno> *lstUniv, vector<SIMD_TYPE> *desc, float tolerancia, int *indice, float *distEucli )
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
    val2 = desc->data();
    
    // numero de elementos de los descriptores
    size_t size = desc->size();

    float dot_product;
    float norm1;
    float norm2;
    
    // Recorre todos los nodos
    ptrNodo = lstUniv->getNode(0);
    for(int iUniv=0; iUniv<nUniv; iUniv++ )
    {
        val1 = ptrNodo->data.vecDescripcion.data();          
        
        dot_product = 0.0;
        norm1 = 0.0;
        norm2 = 0.0;

        // Procesamiento en bloques de 4 floats
        for (size_t i = 0; i < size; i ++) 
        {
            dot_product += val1[i] * val2[i];
            norm1+= val1[i] * val1[i];
            norm2+= val2[i] * val2[i];
        }
        distFinal = dot_product / ( sqrt(norm1) * sqrt(norm2) );

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