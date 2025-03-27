

#ifndef _TH_PROC_DESCRIPTROR_FACIAL_
#define _TH_PROC_DESCRIPTROR_FACIAL_

#include "lib/graphics/ImageSource.h"
#include "lib/deeplearning/DetectionTracker.h"
#include "lib/deeplearning/DetectorBO.h"
#include "lib/facerec/facedetection.h"
#include "lib/general/GThread.h"
#include "lib/general/GLinkedList.h"
#include "app/ThreadGeneradorEventosDet.h"
#include "app/EmisorImagenesHTTP.h"

class IdentificadorFacial;


/**
 * Thread que procesa descriptores faciales calculados 
 */
class ThProcDescFaciales : public ThreadCalculaFacDescListener, public GThread 
{
    public:

        /**
         * Constructor
         */
        ThProcDescFaciales();

        /**
         * Identificador facial asociado
         */
        IdentificadorFacial *identificadorFacial;

        /**
         * Bucle del thread
         */
        void runThread() override;
        
        /**
         * Referencia al mutex con el que se accede a los componentes compartidos
         */
        std::mutex *mtxExterno;

        /**
         * Metodo invocado cuando el thread que calcula las descripciones de rostros ha terminado
         * 
         *  lstFaceDet : lista de detecciones procesada
         */
        void calculoDescFinalizado( GLinkedList<FaceDetection> lstFaceDet ) override;

        /**
         * Metodo al que se le pregunta se se debe o no notificar que se ha terminado el calculo
         * de los descriptores.
         * Si retorna true indica que se debe esperar porque el listener esta ocupado, si retorna
         * false indica que se puede llamar a calculoDescFinalizado
         */
        bool isProcDescFacialesGenerados() override;

    private:

        /**
         * Indica que esta procesando los descriptores faciales
         */
        bool proceDescFaciales;

        /**
         * Lista de rostros que se deben procesar
         */
        GLinkedList<FaceDetection> lstFaceDet;
};

#endif