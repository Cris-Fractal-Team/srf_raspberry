

#ifndef _DEFINE_FACE_DETECTION_
#define _DEFINE_FACE_DETECTION_

#include "lib/general/GObject.h"
#include "lib/graphics/GDibujo.h"
#include "lib/deeplearning/DeteccionVO.h"
#include "lib/facerec/facerec.h"
#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"

#include <stdio.h>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;


/**
 * Clase que permite hacer un programa que haga deteccion y reconocimiento de rostros
 */
class FaceDetection : public GObject
{
    public:

        /**
         * Deteccion con la que se trabajo el rostro
         */
        DeteccionVO deteccion;

        /**
         * Tiempo de deteccion en milisegundos
         */
        long long fecCreacion;

        /**
         * Tiempo en el que se calculo el descriptor
         */
        long long fecDescriptor;

        /**
         * Fecha de la ultima deteccion esta fecha se actualiza 
         */
        long long fecDetect;       

        /**
         * Imagen que contiene el rostro a procesar
         * este rostro ha sido escalado
         */
        GImage imagenRostro;
        /**
         * Imagen completa escalada al visor que se debe enviar
         * cuando ocurra un evento
         */
        GImage imagenVisor;
       
        /**
         * Descriptor del rostro
         */
        FaceDescriptor descriptor;

        /**
         * Constructor
         */
        FaceDetection();

        /**
         * Ddestructor
         */
        ~FaceDetection();
};  


/**
 * Representa una clase que es noficiada cuando el Thread que calcula las descripciones
 * de rostros ha terminado su trabajo
 */
class ThreadCalculaFacDescListener
{
    public:

        /**
         * Metodo invocado cuando el thread que calcula las descripciones de rostros ha terminado
         * 
         *  lstFaceDet : lista de detecciones procesada
         */
        virtual void calculoDescFinalizado( GLinkedList<FaceDetection> lstFaceDet ) = 0;

        /**
         * Metodo al que se le pregunta se se debe o no notificar que se ha terminado el calculo
         * de los descriptores.
         * Si retorna true indica que se debe esperar porque el listener esta ocupado, si retorna
         * false indica que se puede llamar a calculoDescFinalizado
         */
        virtual bool isProcDescFacialesGenerados() = 0;
};


/**
 * Clase que representa un thread que hace el calculo de la descripcion de 
 * rostros de una lista de detecciones de rostro
 */
class ThreadCalculaFaceDescriptor : public GThread
{
    public:

        /**
         * Constructor
         */
        ThreadCalculaFaceDescriptor();

        /**
         * Referencia al objeto que hace el calculo de los descriptores
         */
        FaceRecProcessor *faceDescProc;

        /**
         * Objecto al que se notifica cada vez que se termine de calcular descriptores
         */
        ThreadCalculaFacDescListener *listener;

        /**
         * Solicita el calculo de los descriptores para una lista de rostros detectados
         *  lstFaceDet : lista con instancias de FaceDetection
         * 
         *  imagen : imgen desde la que se deben extraer los rectangulos de cada rostro
         */
        void calcularDescriptores( GLinkedList<FaceDetection> lstFaceDet, GImage imagen );

        
        /**
         * Retorna el valor de si el thread esta o no calculando descripciones
         */
        bool getCalculando();

        /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Referencia al mutex con el que se accede a los componentes compartidos
         */
        std::mutex *mtxExterno;


    private:

        /**
         * Indica que el thread esta calculando descripciones
         */
        bool calculando;

        /**
         * Lista con instancias de FaceDetection para procesar
         */
        GLinkedList<FaceDetection> lstFaceDet;

        /**
         * Imagen desde la que se deben extraer los rostros
         */
        GImage imagen;

};

#endif