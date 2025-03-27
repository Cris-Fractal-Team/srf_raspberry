

#ifndef _IDENTIFICADOR_PERSONAS_HAILO_
#define _IDENTIFICADOR_PERSONAS_HAILO_

#include <stdio.h>
#include <string>
#include <memory>
#include "lib/hailolib/FaceRecHailo.h"

using namespace std;


/**
 * Clase que se encarga de buscar coincidenias
 * entre un rostro detectado y una lista de personas
 * ya conocidas
 */
class IdentificadorPerHailo
{
    public:

        /**
         * ID de la siguiente persona no desconocida
         */
        long long idSgtePerDesc;

        /**
         * Constructor
         */
        IdentificadorPerHailo();

        /**
         * Lista de personas 100% identificadas
         */
        GLinkedList<DescPersonaExterno> lstPerIdentificadas;

        /**
         * Lista de personas no identifiadas
         */
        GLinkedList<DescPersonaExterno> lstPerNoIdent;
        

        /**
         * Carga la BD de personas conocidas
         */
        void cargarpPerConocidas( string path );

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
         */
        DescPersonaExterno* buscaPersonaCon( vector<SIMD_TYPE> *descriptor, float tolerancia, float *distEucli );

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
        DescPersonaExterno* buscaPersonaDesc( vector<SIMD_TYPE> *descriptor, float tolareancia, float *distEucli );

        /**
        * Función para calcular la distancia euclidiana usando NEON SIMD
        * entre dos arreglos que contiene descripciones faciales 
        */ 
       float calculaDiferenciaSIMD( vector<SIMD_TYPE> *desc1, vector<SIMD_TYPE> *desc2 );

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
        *           si la distancia euclideana es mayor a tolerancia se ignora
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
       DescPersonaExterno*  encuentraPerCercana( GLinkedList<DescPersonaExterno> *lstUniv, vector<SIMD_TYPE> *desc2, float tolerancia, int *indice, float *distEucli );
        

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
        *           si la similaridad de coceno es mayor a tolerancia se ignora
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
       DescPersonaExterno*  encuentraPerCerCos( GLinkedList<DescPersonaExterno> *lstUniv, vector<SIMD_TYPE> *desc, float tolerancia, int *indice, float *distEucli );
        
};

#endif