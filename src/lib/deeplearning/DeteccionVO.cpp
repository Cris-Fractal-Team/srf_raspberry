#include "lib/deeplearning/DeteccionVO.h"

DeteccionVO::DeteccionVO()
{
    
}

DeteccionVO::~DeteccionVO()
{
    //dtor
}

/**
 * Clona una deteccion
 */
DeteccionVO DeteccionVO::getClone()
{
    DeteccionVO det;

    det.confianza = this->confianza;
    det.id = this->id;
    det.indiceClase = this->indiceClase;
    det.imagen = this->imagen;
    det.region = this->region.getClone();

    return det;
}

/**
 * Retorna el tipo de dato
 */
string DeteccionVO::getType() 
{ 
    return string("DeteccionVO"); 
}
 

 /**
 * Asigna la imagen
 */
void DeteccionVO::setImage( GImage image )
{
    imagen = image;
}

/**
 * Retorna la imagen
 */
GImage DeteccionVO::getImage()
{
    return imagen;
}

/**
 * Copia los valores desde otra instancia de DeteccionVO.
 * Util para cuando hay herencia, se puede sobre escribir
 * este metodo y se puede igualar una clase padre a una hija
 */
void DeteccionVO::copyValues( DeteccionVO value )
{
    this->id = value.id;
    this->confianza = value.confianza;
    this->imagen = value.imagen;
    this->indiceClase = value.indiceClase;
    this->region = value.region;
}


/**
 * Retorna la posicion del ojo izquierdo
 */
GPoint DeteccionVO::getPosOjoIzq()
{
    return ptoOjoIzq;
}

/**
 * Retorna la posicion del ojo izquierdo
 */
GPoint DeteccionVO::getPosOjoDer()
{
    return ptoOjoDer;
}

/**
 * Retorna la posicion de la nariz
 */
GPoint DeteccionVO::getPosNariz()
{
    return ptoNariz;
}

/**
 * Retorna la posicion izquierda de la boca
 */
GPoint DeteccionVO::getPosBocaIzq()
{
    return ptoBocaIzq;
}

/**
 * Retorna la posicion derecha de la boca
 */
GPoint DeteccionVO::getPosBocaDer()
{
    return ptoBocaDer;
}