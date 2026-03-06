#include "app/PreProcesadorDataSet.h"
#include "lib/graphics/GDibujo.h"
#include "lib/utils/fileutils.h"
#include "lib/utils/timedate.h"
#include "lib/hailolib/DetectionTrackerHailo.h"
#include "lib/hailolib/FaceRecHailo.h"
#include "lib/hailolib/IdentificadorPersonasHailo.h"


#include <filesystem>
namespace fs = std::filesystem;

/**
 * Pre-procesa un directorio con imagenes capturadas
 * eliminando las imagenes que no tienen una cara luego 
 * de que se transformo.
 * Se espera que las imagenes estan en formato mucho menor a 800x600
 */
void PreProcesadorDataSet::eliminaNoCaras( hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr, string path )
{
    GImage imagen(800,600, GDIBUJO_IMAGEN_MODO_RGB);
    DetectorCarasHailoSCRFD detector;
    string rutaArchivoFoto;
    vector<string> lstArchivos = listArchivosDirectorio(path);
    vector<DeteccionCaraHailo> lstCaras;
    int i;
    int n = lstArchivos.size();

    // configura el detector
    detector.setDimImagenes(640,640);
    detector.runner.device = devicePtr;
    if ( detector.cargarModelo("./models/scrfd_2.5ga.hef") != 0 )
    {
        cout << "Error al cargar el modelo detector" << endl;        
        return;
    }           
    cout << "Detector de rostros iniciado" << endl;

    // itera sobre todos los archivos
    for(i=0; i<n; i++)
    {
        rutaArchivoFoto = lstArchivos[i];

        GImage foto = GDibujo::read(rutaArchivoFoto);
        GDibujo::drawImage(imagen, 90, 90, foto);

        lstCaras = detector.detectar_2_5g(imagen, 0.50);
        if ( lstCaras.size() == 0 )
        {
            cout << "Borrando el archivo : " << rutaArchivoFoto << endl;
            fs::remove(rutaArchivoFoto);
        }
    }   
}

/**
 * Dada una lista de nombres de archivos que terminan en _indice.extecion
 * Calcula el indice o secuencia maxia
 */
int PreProcesadorDataSet::buscaSecuenciaMaxima( vector<string>lstNombres )
{
    int posSep,posPunto;
    int secuencia,secuenciaMax;
    int i,n;
    string rutaArchivoFoto;

    secuenciaMax = 0;
    n = lstNombres.size();
    for(i=0; i<n; i++)
    {
        rutaArchivoFoto = lstNombres[i];
        posSep = rutaArchivoFoto.find_last_of("_");
        if ( posSep == -1 )
            continue;

        posPunto = rutaArchivoFoto.find_last_of(".");
        if ( posPunto == -1 )
            continue;

        secuencia = stoi(rutaArchivoFoto.substr(posSep+1, (posPunto-posSep-1)));
        if ( secuencia > secuenciaMax )
            secuenciaMax = secuencia;
    }

    return secuenciaMax;
}

/**
 * Ordena un vector con una lista de nombres de archivos en base al sufijo
 * de secuancia que tiene, el formata del nombre es : _secuencia.extension
 * 
 *  param lstNombres: referencia a la lista que se desea ordenar
 * 
 *  param secMaxima: valor maximo de la secuencia
 * 
 * Retorna un vector con los nombres ordenados
 */
vector<string> PreProcesadorDataSet::ordenaPorSecuencia( vector<string> *lstNombres, int secMaxima )
{
    int i,n,iSec,posSep,posExt,sec;
    string nombre;
    vector<bool> lstExiste(secMaxima+1);
    vector<string> lstNombOrd(secMaxima+1);
    vector<string>lstFinal;

    // inicializa los vectores
    for(iSec=1; iSec <= secMaxima; iSec++)
    {
        lstExiste[iSec] = false;
        lstNombOrd[iSec] = "";
    }
    
    // crea vectores on flags para saber si existe o no un nombre con una secuencia dada
    n = lstNombres->size();
    for(i=0; i<n; i++ )
    {
        nombre = lstNombres->at(i);
        posSep = nombre.find_last_of("_");
        posExt = nombre.find_first_of(".");
        sec = stoi(nombre.substr(posSep+1,posExt-posSep-1));

        lstExiste[sec] = true;
        lstNombOrd[sec] = nombre;
    }
    
    // guarda solo los valores que existen
    for(iSec=1; iSec <= secMaxima; iSec++)
    {
        if ( lstExiste[iSec] == false )
            continue;

        nombre = lstNombOrd[iSec];
        lstFinal.push_back(nombre);
    }

    return lstFinal;
}

/**
 * Analiza los archivos de un directorio donde cada archivo
 * contiene una cara, se supone que los debe ordener en orden
 * alfabetico de nombre y luego calcular el descriptor facial
 * y mientras pertenezcan a la misma persona agruparlos en un directorio
 * de un pathDestino
 */
void PreProcesadorDataSet::agrupaCaras( hailort::Expected<std::unique_ptr<hailort::VDevice>> *devicePtr, string pathOrig, string pathDestino, float similaridadCoseno, shared_ptr<GHashMap> params )
{
    FaceRecHailo generadorDesc;   
    string rutaArchivoFoto, pathGrupo;
    vector<string> lstArchivos;
    int i,secuenciaMax,n,tecla;

    // configura el generador de descriptores faciales
    string pathModeloDescFacial = params->getString("pathModeloDescFacial");
    string nombreCapaSalidaRedFacial = params->getString("nombreUltimaCapaRedNeuronal");

    generadorDesc.runner.device = devicePtr;
    generadorDesc.paramContraste = params->getStringDouble("paramContraste", 0);
    generadorDesc.previsualizaImgReconocimiento = params->getStringBool("previsualizaImgReconocimiento", false);
    generadorDesc.esperarPrevImgReconocimiento = params->getStringBool("esperarPrevImgReconocimiento", false);
    generadorDesc.nombreUltimaCapaModelo = nombreCapaSalidaRedFacial;

    if ( generadorDesc.cargarModelo(pathModeloDescFacial) != 0 )
    {
        cout << "Error al cargar modelo generador de descriptores" << endl;
        return;
    }
    cout << "Generador de descriptores iniciado" << endl;

    // Configura el identificador, objeto que busca un descriptor en una lista de personas conocidas
    IdentificadorPerHailo identificador;

    // Obtiene los archivos y los ordena
    lstArchivos = listArchivosDirectorio(pathOrig);
    secuenciaMax = buscaSecuenciaMaxima(lstArchivos);
    lstArchivos = ordenaPorSecuencia(&lstArchivos, secuenciaMax);

    DeteccionCaraHailo det;
    float norm1 = 0.0;
    float distancia;
    int indiceIdentif;
    DescPersonaExterno *descEncontrado;

    // Ponemos en duro la ubicacion de la deteccion a las dimensiones 
    // esperadas por el modelo, que deben coincidir con las dimensiones de la imagen
    det.ptoSupIzq.x = 0;
    det.ptoSupIzq.y = 0;
    det.ptoInfDer.x = 111;
    det.ptoInfDer.y = 111;

    // Ponemos en duro las coordenadas de los ojos, nariz y boca
    // en base a las coordenadas esandares de arcface
    det.ojoIzq.x = 38.2946;
    det.ojoIzq.y = 51.6963;
    det.ojoDer.x = 73.5318;
    det.ojoDer.y = 51.5014;
    det.nariz.x = 56.0252;
    det.nariz.y = 71.7366;
    det.bocaIzq.x = 41.5493;
    det.bocaIzq.y = 92.3655;
    det.bocaDer.x = 70.7299;
    det.bocaDer.y = 92.2041;

    n = lstArchivos.size();

    // Creamos la lista con el universo de personas conocidas
    GLinkedList<DescPersonaExterno> lstUniv;
    DescPersonaExterno desc2;
    bool iniciaGrupo;
    int posUltimo;
    
    GImage foto, fotoGrupo;
    fs::copy_options opts = fs::copy_options::overwrite_existing;

    for(i=0; i<n; i++)
    {
        rutaArchivoFoto = lstArchivos[i];
        foto = GDibujo::read(rutaArchivoFoto);
                
        // if ( foto.estaDesenfocada() == true )
        // {
        //     cout << "Descartamos por desenfocada: " << rutaArchivoFoto << endl; 
        //     continue;
        // }   
        // if ( foto.estaRuidosa() == true )
        // {
        //     cout << "Descartamos por ruiodosa en oscuridad: " << rutaArchivoFoto << endl; 
        //     continue;
        // }

        for(int j=0; j < 512; j++) desc2.vecDescripcion[j] = 0.0;
        generadorDesc.calculaDescriptor(foto, det, desc2.vecDescripcion);

        fs::path pathArchivoOrigen = rutaArchivoFoto;
                
        if ( i == 0 )
        {            
            iniciaGrupo = true;
            desc2.normaliza();
        }
        else
        {            
            desc2.normaliza();
            descEncontrado = identificador.encuentraPerCerCos(&lstUniv, desc2.vecDescripcion, 0.10, &indiceIdentif, &distancia );
            if (( descEncontrado != NULL ) && ( distancia >= similaridadCoseno ))
            {
                iniciaGrupo = false;
                cout << "Misma persona : " << descEncontrado->id <<  endl;
                // GDibujo::show(foto,"Foto");
                pathGrupo = pathDestino + "/p_" + descEncontrado->id;
                                
                fs::path dirDestino = pathGrupo;
                fs::path pathArchivoFinal = dirDestino / pathArchivoOrigen.filename();
                fs::rename(pathArchivoOrigen, pathArchivoFinal);
            }
            else
            {
                iniciaGrupo = true;
            }
        }

        if ( iniciaGrupo == true )
        {        
            DescPersonaExterno descUniverso;
            
            cout << "Iniciando grupo: " << rutaArchivoFoto << " Similaridad " << distancia << endl;
            for(int j=0; j < 512; j++) descUniverso.vecDescripcion[j] = desc2.vecDescripcion[j]; 
                        
            descUniverso.id = to_string(i);
            descUniverso.norm_coceno_calculado = true;  
            lstUniv.add(descUniverso);

            pathGrupo = pathDestino + "/p_" + descUniverso.id;

            fs::path dirDestino = pathGrupo;
            fs::path pathArchivoFinal = dirDestino / pathArchivoOrigen.filename();

            fs::create_directory(dirDestino);
            fs::rename(pathArchivoOrigen, pathArchivoFinal);
            
            // fotoGrupo = foto.clone();
            // GDibujo::show(fotoGrupo,"Foto Grupo");
        }

        // tecla = GDibujo::waitForKey(0);
        // if ( tecla == 'q' )
        //    break;
    }
}