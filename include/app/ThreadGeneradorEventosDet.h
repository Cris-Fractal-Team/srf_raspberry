
#ifndef _GENERADOREVENTOS_
#define _GENERADOREVENTOS_

#include "lib/general/GThread.h"
#include "lib/deeplearning/DeteccionVO.h"
#include "lib/facerec/facerec.h"


/** 
 * Clase que representa un thread que genera eventos de deteccion
 * de personas
*/
class ThreadGeneradorEventosDet : public GThread
{
    public:

        /**
         * Inicia el Thread Notificador
         */
        void iniciarBucle();

         /**
         * Bucle del thread
         */
        void runThread() override;

        /**
         * Agrega una lista de detecciones para generar un evento
         */
        void addDetecciones( shared_ptr<GVector> lstRostros, GImage imagen );

    private:

        /**
         * Lista con instancias de shared_ptr<GVector> tiene dos elementos :
         *      Instancia de shared_ptr<GVector> con instancias de FaceDetection
         *      Instancia de GImage con la foto del visor
         */
        shared_ptr<GVector> lstFaceDet;
        

};

#endif
