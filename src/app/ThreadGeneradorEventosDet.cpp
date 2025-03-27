

#include "app/ThreadGeneradorEventosDet.h"


/**
 * Inicia el Thread Notificador
*/
void ThreadGeneradorEventosDet::iniciarBucle()
{

}

/**
 * Bucle del thread
 */
void ThreadGeneradorEventosDet::runThread()
{
    int n;

    // while( this->isFinalizado() == false )
    // {
    //     mtxBloquea();
    //     n = this->lstFaceDet->size();
    //     mtxLibera();
    //     if ( n == 0 )
    //     {
    //         sleepMS(5);
    //         continue;
    //     }

        
    // }   
}

/**
 * Agrega una lista de detecciones para generar un evento
 */
void ThreadGeneradorEventosDet::addDetecciones( shared_ptr<GVector> lstRostros, GImage imagen )
{
    
}
