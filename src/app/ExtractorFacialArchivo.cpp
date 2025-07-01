
#include "app/ExtractorFacialArchivo.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/GStringUtils.h"
#include "lib/hailolib/DetectionTrackerHailo.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

/**
 * Ejecuta el proceso
 */
void ExtractorFacialArchivo:: ejecutar()
{
    int i,n;

    vector<DeteccionCaraHailo> lstDet;
    string fila,idPersona,pathFoto,nombre;
    string datosArchivo = leeArchivoTexto(pathArchivoDatos);
    GVector lstFilas = GStringUtils::split(datosArchivo, "\n");
    hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr;
    DetectorCarasHailoSCRFD detector;        
    FaceRecHailo generadorDesc;
    std::ofstream archivoBd(pathArchivoBD, std::ios::out);
    
    if ( !archivoBd.is_open() )
    {
        cerr << "Error: no se pudo abrir/crear el archivo de BD" << endl;
        return;
    }

    // inicializa el dispositivo
    hailort::Expected<std::unique_ptr<hailort::VDevice>> device = hailort::VDevice::create();    
    if (!device) 
    {
        cerr << "Error: No se pudo inicializar el dispositivo Hailo." << endl;        
        return;
    }
    devicePtr = &device;

    // configura el detector
    detector.setDimImagenes(640,640);
    detector.runner.device = &device;
    if ( detector.cargarModelo("./models/scrfd_2.5ga.hef") != 0 )
    {
        cout << "Error al cargar el modelo detector" << endl;        
        return;
    }           
    cout << "Detector de rostros iniciado" << endl;

    // crea el generador de descriptores faciales
    generadorDesc.runner.device = &device;
    if ( generadorDesc.cargarModelo("./models/arcface_mobilefacenet.hef") != 0 )
    {
        cout << "Error al cargar modelo generador de descriptores" << endl;
        return;
    }
    cout << "Generador de descriptores iniciado" << endl;
     
    n = lstFilas.size();
    for(i=0;i<n;i++)
    {
        fila = lstFilas.getString(i);
        fila = GStringUtils::trim(fila);
        GVector lstValores = GStringUtils::split(fila,",");
        if ( lstValores.size() != 3 )
            continue;

        idPersona = lstValores.getString(0);
        nombre = lstValores.getString(1);
        pathFoto = lstValores.getString(2);

        cout << "ID:" << idPersona << " Nombre:" << nombre << " PATH:" << pathFoto << endl;
        
        GImage rostro = GDibujo::read(pathFotos + pathFoto);        
        
        cout << "Ancho del rostro " << rostro.ancho << " Altura " << rostro.altura << endl;
        lstDet = detector.detectar_2_5g(rostro, toleranciaDetec);        
        if ( detector.errorDeteccion == true )
        {
            cout << "Error al detectar rostros" << endl;
            break;
        }

        if ( lstDet.size() > 0 )
        {            
            DeteccionCaraHailo det = lstDet.at(0);
            GImage fotoCara = rostro.getRect(det.ptoSupIzq.x, det.ptoSupIzq.y, det.ptoInfDer.x, det.ptoInfDer.y);
            
            // GDibujo::show(fotoCara, "Visor");
            // GDibujo::waitForKey(0);
            
            SIMD_TYPE descriptor[512];
            
            generadorDesc.calculaDescriptor(rostro, det, descriptor);        
            
            string fila = nombre;
            fila.append(",");
            fila.append(idPersona);
            fila.append(",");

            for(int j=0; j<512; j++)
            {
                fila.append(to_string(descriptor[j]));
                fila.append(",");
            }
            fila.append("\n");
            archivoBd << fila;

            // cout << fila << endl;
        }
        else
        {
            cout << "!!! NO se detecto un rostro" << endl;
        }
    }

    archivoBd.close();

}
