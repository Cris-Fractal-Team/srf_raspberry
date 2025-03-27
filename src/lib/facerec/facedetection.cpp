
#include "lib/facerec/facedetection.h"
#include "lib/facerec/facerecyn.h"
#include "lib/utils/timedate.h"

#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

/**
 * Constructor
 */
FaceDetection::FaceDetection()
{   
    fecCreacion = TimeDateUtils::getDateTimeMs();
    fecDescriptor = -1;    
    fecDetect = -1;       
}

/**
 * Ddestructor
 */
FaceDetection::~FaceDetection()
{
}

/**
 * Constructor
 */
ThreadCalculaFaceDescriptor::ThreadCalculaFaceDescriptor()
{
    
}


/**
 * Solicita el calculo de los descriptores para una lista de rostros detectados
 *  lstFaceDet : lista con instancias de FaceDetection
 * 
 *  imagen : imgen desde la que se deben extraer los rectangulos de cada rostro
 */
void ThreadCalculaFaceDescriptor::calcularDescriptores( GLinkedList<FaceDetection> lstFaceDet, GImage imagen )
{   
    mtxBloquea();     
    this->lstFaceDet = lstFaceDet;
    this->imagen = imagen;  
    calculando = true;
    mtxLibera();
}


/**
 * Retorna el valor de si el thread esta o no calculando descripciones
 */
bool ThreadCalculaFaceDescriptor::getCalculando()
{
    bool rpta;

    mtxBloquea();
    rpta = calculando;
    mtxLibera();
    
    return rpta;
}


/**
 * Bucle del thread
 */
void ThreadCalculaFaceDescriptor::runThread()
{
    int i,n;
    GImage img;
    shared_ptr<DeteccionFaceVO>detFace;
    double escalaHor,escalaVer,valor;
    FaceDetection *face;
    FaceDescriptor descriptor;
    GLinkedList<FaceDetection> lstDetLocal;
    
    std::cout << "Iniciando Thread Identificador de Rostros :" << this->id_unico << endl;

    
    std::chrono::milliseconds intervaloEspera(1);

    calculando = false;
    while ( isFinalizado() == false )
    {
        if (( getCalculando() == false ) || ( mtxExterno == NULL ))
        {
            sleepMS(1);
            continue;
        }
                
        cout << "Inicia calculo DF " << endl;
        this->mtxExterno->lock();
        // lstDetLocal.addAll(lstFaceDet);
        lstDetLocal = lstFaceDet;
        this->mtxExterno->unlock();
        
        n = lstDetLocal.size();
        cout << "Calculando DF " << n << endl;
        
        for(i=0; i < n; i++)
        {
            face = lstDetLocal.getAddr(i);
            if ( face->imagenRostro.isEmpty() == false )
            {                
                img = face->imagenRostro;
            }
            else
            {
                img = imagen.getRect(face->deteccion.region);
            }                                    

            descriptor = faceDescProc->getDescriptorRostro(img, face->deteccion);
            mtxExterno->lock();
            face->descriptor = descriptor;
            mtxExterno->unlock();
        }

        // cout << "Fin calculo DF " << lstDetLocal->size() << endl;
        while( listener->isProcDescFacialesGenerados() == true )
        {
            sleepMS(1);
        }
        cout << "Notifica fin calcul DF " <<  endl;
        listener->calculoDescFinalizado(lstDetLocal);

        cout << "Calculo DF Libre " <<  endl;

        mtxBloquea();
        calculando = false;
        mtxLibera();

        std:this_thread::sleep_for(intervaloEspera);
    }

    std::cout << "Thread Identificador de rostros FINALIZADO" << endl;
}

