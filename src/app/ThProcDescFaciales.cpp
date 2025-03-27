
#include "app/ThProcDescFaciales.h"
#include "app/IdentificadorFacial.h"


/**
 * Constructor
 */
ThProcDescFaciales::ThProcDescFaciales()
{

}

/**
 * Bucle del thread
 */
void ThProcDescFaciales::runThread()
{
    mtxBloquea();
    proceDescFaciales = false;
    mtxLibera();

    cout << "Thead Proc Desc. Faciales iniciado" << endl;
    while( isFinalizado() == false  )
    {
        if ( isProcDescFacialesGenerados() == false )
        {
            sleepMS(1);
            continue;
        }    

        if ( lstFaceDet.size() > 0 )
        {
            identificadorFacial->calculoDescFinalizado(lstFaceDet);
            lstFaceDet.reset();
        }        
        mtxBloquea();
        proceDescFaciales = false;
        mtxLibera();
    }
    
}


/**
 * Metodo invocado cuando el thread que calcula las descripciones de rostros ha terminado
 * 
 *  lstFaceDet : lista de detecciones procesada
 */
void ThProcDescFaciales::calculoDescFinalizado( GLinkedList<FaceDetection> lstFaceDet )
{
    mtxBloquea();
    this->lstFaceDet = lstFaceDet;
    proceDescFaciales = true;  
    mtxLibera();       
}

/**
 * Metodo al que se le pregunta se se debe o no notificar que se ha terminado el calculo
 * de los descriptores.
 * Si retorna true indica que se debe esperar porque el listener esta ocupado, si retorna
 * false indica que se puede llamar a calculoDescFinalizado
 */
bool ThProcDescFaciales::isProcDescFacialesGenerados()
{
    bool rpta;

    mtxBloquea();
    rpta = proceDescFaciales;
    mtxLibera();

    return rpta;
}