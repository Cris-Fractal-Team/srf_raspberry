
#include "lib/hailolib/hailo8l.h"
#include <chrono>

/**
 * Contrustor
 */
Hailo8LRunner::Hailo8LRunner()
{
    estadoEjec = H8LR_LIB_ESTADO_NUEVO;
    estadoInferencia = H8LR_LIB_INF_PENDIENTE;
}


/**
 * Destructor
 */
Hailo8LRunner::~Hailo8LRunner()
{
    cout << "Destructor de Hailo8LRunner" << endl;
    if ( isFinalizado() == false )
    {
        cout << "Se solicita finalizar thread" << endl;
        finalizar();
    }
    sleep(1);
}

/**
 * Carga una red neuronal
 * 
 *  path:
 *      Ruta de la red
 * 
 *  Retorno:
 *      0 : En caso de exito 
 *      Diferente a 0 en base a constantes definidas Hailo8LRunner_ERROR_CARGA_XXXXXX 
 */
int Hailo8LRunner::cargarRed( string path )
{
    mtxBloquea();
    pathModelo = path;
    mtxLibera();
    
    setEstadoEjec(H8LR_LIB_ESTADO_PEND_CARGAR_RED);
    start();
    sleep(1);

    while( getEstadoEjec() == H8LR_LIB_ESTADO_PEND_CARGAR_RED )
    {   
        sleepMS(50);
    }    

    if ( getEstadoEjec() != H8LR_LIB_ESTADO_RED_CARGADA ) 
        return getError();

    return 0;
}


/**
 * Imprime la informacion sobre el stream de salida
 */
void Hailo8LRunner::printInfoStreamSalida()
{
    int i,numSalidas;
    hailo_vstream_info_t infoVstr;

    numSalidas = outVStreamsInfo.size();        

    cout << endl;
    cout << "Num salidas :" << numSalidas << endl;
    for(i=0;i<numSalidas;i++)
    {
        infoVstr = outVStreamsInfo.at(i);
        cout << infoVstr.name << endl ;
        cout << "\t" << "Escala Cuantizacion : " << infoVstr.quant_info.qp_scale << endl;
        cout << "\t" << "Zero Point : " << infoVstr.quant_info.qp_zp << endl;
        cout << "\t" << "Lim.Min : " << infoVstr.quant_info.limvals_min << endl;
        cout << "\t" << "Lim.Max : " << infoVstr.quant_info.limvals_max << endl;
        cout << "\t" << "Features : " << infoVstr.shape.features << endl;          
        cout << "\t" << "Width : " << infoVstr.shape.width << endl;          
        cout << "\t" << "Height : " << infoVstr.shape.height << endl;                        
        cout << endl;
    }   
    cout << endl;
}

/**
 * Retorna la informacion de una capa dado su nombre
 */
hailo_vstream_info_t Hailo8LRunner::getOutVsTramInfo( string nomCapa )
{
    int i,numSalidas;
    hailo_vstream_info_t infoVstr;

    mtxBloquea();
    numSalidas = outVStreamsInfo.size();        
    for(i=0;i<numSalidas;i++)
    {
        infoVstr = outVStreamsInfo.at(i);
        if ( nomCapa.compare(infoVstr.name) == 0 )
        {
            mtxLibera();
            return infoVstr;
        }        
    }
    mtxLibera();

    string msg("No existe la capa : " + nomCapa);

    throw runtime_error(msg);
}

/**
 * Retorna un stream de salida dado su nombre
 */
vector<uint8_t> Hailo8LRunner::getOutputStream( string nombre )
{  
    std::map<std::string, std::vector<uint8_t>> mapaOutput;

    mtxBloquea();
    mapaOutput = lastOutputData;
    mtxLibera();

    vector<uint8_t> rpta = mapaOutput.at(nombre);

    return rpta;
}   


/**
 * Funcion que se ejecuta en paralelo desde el thread
 */
void Hailo8LRunner::runThread()
{    
    string path;
    uint8_t *data;
    uint32_t dataSize;

    // Crea el dispositivo virtual
    // cout << "Creando VDevice" << endl;
    // hailort::Expected<std::unique_ptr<hailort::VDevice>> device = hailort::VDevice::create();    
    // if (!device) 
    // {
    //     cerr << "Error: No se pudo inicializar el dispositivo Hailo." << endl;
    //     errorEjec = H8LR_LIB_ERR_CREAR_DISPOSITIVO;
    //     setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
    //     return;
    // }

    mtxBloquea();
    path = pathModelo;
    mtxLibera();

    // Cargar el modelo HEF
    cout << "Cargando el modelo" << endl;
    hailort::Expected<hailort::Hef> hef = hailort::Hef::create(path);
    if (!hef) 
    {
        cerr << "Error: No se pudo cargar el modelo " << pathModelo << endl;
        errorEjec = H8LR_LIB_ERR_CARGA_MODELO;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }
   

    // Obtiene informacion de los streams de salida
    cout << "Obtiene informacion de salida" << endl;
    hailort::Expected<std::vector<hailo_vstream_info_t>> output_vstream_info = hef->get_output_vstream_infos();
    if (!output_vstream_info) 
    {
        cerr << "Error: No se pudo obtener informacion de los streams de salida " << pathModelo << endl;
        errorEjec = H8LR_LIB_ERR_OBT_INFO_STREAM_SALIDA;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }    
    outVStreamsInfo = output_vstream_info.value();
    printInfoStreamSalida();

    // crea los parametros por defecto
    hailort::Expected<hailort::NetworkGroupsParamsMap> parametros = (*device).value()->create_configure_params(hef.value()); 
    if ( !parametros )
    {
        cerr << "Error: No se pudo crear los parametros por defecto." << endl;
        errorEjec = H8LR_LIB_ERR_CREA_PARAM_DEFECTO;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }

    // crea el grupo de red
    hailort::Expected<hailort::ConfiguredNetworkGroupVector> network_groups = (*device).value()->configure(hef.value(), parametros.value());
    if ( !network_groups )
    {
        cerr << "Error: No se pudo crear la lista de redes configuradas." << endl;
        errorEjec = H8LR_LIB_ERR_CREAR_GRUPO_RED;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }

    int numElem = network_groups->size();
    if ( 1 != numElem )
    {
        cerr << "Error: La lista de redes configuradas no tiene un elemento." << endl;
        errorEjec = H8LR_LIB_ERR_GRUP_SIN_UNA_RED;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }

    cout << "Se crea el grupo de redes" << endl;

    // obtenemos la primera red
    std::shared_ptr<hailort::ConfiguredNetworkGroup> network_group = std::move(network_groups->at(0));

    // Creamos parametros para el stream de entrada
    hailort::Expected<std::map<std::string, hailo_vstream_params_t>> input_params = network_group->make_input_vstream_params({}, HAILO_FORMAT_TYPE_AUTO, HAILO_DEFAULT_VSTREAM_TIMEOUT_MS, HAILO_DEFAULT_VSTREAM_QUEUE_SIZE);
    if ( !input_params )
    {
        cerr << "Error: No se pudo crear los parametros para el stream de entrada." << endl;
        errorEjec = H8LR_LIB_ERR_CREAR_STREAM_ENTRADA;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }

    // Creamos parametros para el stream de salida
    hailort::Expected<std::map<std::string, hailo_vstream_params_t>> output_params = network_group->make_output_vstream_params({}, HAILO_FORMAT_TYPE_AUTO, HAILO_DEFAULT_VSTREAM_TIMEOUT_MS, HAILO_DEFAULT_VSTREAM_QUEUE_SIZE);
    if ( !output_params )
    {
        cerr << "Error: No se pudo crear los parametros para el stream de salida." << endl;
        errorEjec = H8LR_LIB_ERR_CREAR_STREAM_SALIDA;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }

    // crea los streams para la inferencia
    hailort::Expected<hailort::InferVStreams> pipeline = hailort::InferVStreams::create(*network_group.get(), input_params.value(), output_params.value()) ;
    if ( !pipeline )
    {
        cerr << "Error: No se pudo crear los streams de entrada y salida." << endl;
        errorEjec = H8LR_LIB_ERR_CREAR_PIPELINE_INF;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    } 

    // Obtiene la lista de streams de entrada
    input_vstreams = pipeline->get_input_vstreams();
    if ( input_vstreams.empty() )
    {
        cerr << "Error: No se encontro el stream de entrada." << endl;
        errorEjec = H8LR_LIB_ERR_GET_STREAM_ENTRADA;
        setEstadoEjec(H8LR_LIB_ESTADO_ERROR_CARGA_RED);
        return;
    }

    cout << "Fin de carga" << endl;

    // Espera hasta que se solicite ejecutar redes
    setEstadoEjec(H8LR_LIB_ESTADO_RED_CARGADA);
    setEstadoInf(H8LR_LIB_INF_ESPERA_DATA);

    int estInf;
    
    while( isFinalizado() == false )
    {        
        estInf = getEstadoInf();
        if (( estInf == H8LR_LIB_INF_ESPERA_DATA ) || ( estInf == H8LR_LIB_INF_ERRADA ))
        {
            sleepMS(1);
            continue;
        }
        
        mtxBloquea();
        data = inferData;
        dataSize = inferDataSize;
        mtxLibera();

        // Se debe ejecutar la inferencia
        // crea un memory view para el ingreso de datos
        mtxBloquea();
        std::map<std::string, hailort::MemoryView> input_data_mem_views;
        input_data_mem_views.emplace(input_vstreams[0].get().name(), hailort::MemoryView(data, dataSize));

        // Crear buffer para la salida de la inferencia
        auto output_vstreams = pipeline->get_output_vstreams();
        std::map<std::string, std::vector<uint8_t>> output_data;
        std::map<std::string, hailort::MemoryView> output_data_mem_views;

        size_t numBytesOutput;
        
        for (const auto &output_vstream : output_vstreams) {
            numBytesOutput = output_vstream.get().get_frame_size() * inferNumber;
            output_data[output_vstream.get().name()] = std::vector<uint8_t>(numBytesOutput);            
            output_data_mem_views.emplace(output_vstream.get().name(), hailort::MemoryView(output_data[output_vstream.get().name()].data(), numBytesOutput));
        }
        mtxLibera();


        // Ejecutar la inferencia
        mtxBloquea();
        hailo_status status = pipeline->infer(input_data_mem_views, output_data_mem_views, inferNumber);
        mtxLibera();


        mtxBloquea();
        lastOutputData = output_data;
        mtxLibera();
                
        if (status != HAILO_SUCCESS) 
        {
            std::cerr << "Error en la inferencia: " << hailo_get_status_message(status) << std::endl;            
            setEstadoInf(H8LR_LIB_INF_ERRADA);
        }
        else
        {
            setEstadoInf(H8LR_LIB_INF_ESPERA_DATA);
        }
        
    }
    
    cout << "Thread de ejecucion de Hailo8L Runner finalizado" << endl;
}


/**
 * Establece el estado de ejecucion
 */
void Hailo8LRunner::setEstadoEjec( int estado )
{
    mtxBloquea();
    estadoEjec = estado;
    mtxLibera();

    if ( estado == H8LR_LIB_ESTADO_ERROR_CARGA_RED )
        finalizar();
}

/**
 * Retorna el estado de ejecucion
 */
int Hailo8LRunner::getEstadoEjec()
{
    int rpta;

    mtxBloquea();
    rpta = estadoEjec;
    mtxLibera();

    return rpta;
}

/**
 * Establece el estado de ejecucion de inferencias
 */
void Hailo8LRunner::setEstadoInf( int estado )
{
    mtxBloquea();
    estadoInferencia = estado;
    mtxLibera();
}


/**
 * Retorna le esado de ejecucion de inferencias
 */
int Hailo8LRunner::getEstadoInf()
{   
    int rpta;

    mtxBloquea();
    rpta = estadoInferencia;
    mtxLibera();

    return rpta;
}

 /**
 * Retorna el codigo del ultimo error
 */
int Hailo8LRunner::getError()
{   
    int rpta;

    mtxBloquea();
    rpta = errorEjec;
    mtxLibera();

    return rpta;
}

 /**
 * Ejecuta la inferencia sobre los datos que se pasan
 */
bool Hailo8LRunner::ejecutarInferencia( uint8_t *data, uint32_t dataSize, uint32_t num_inferencias )
{       
    int estado;
    auto inicio = std::chrono::high_resolution_clock::now(); // Inicia el cronómetro

    mtxBloquea();
    inferData = data;
    inferDataSize = dataSize;
    inferNumber = num_inferencias;
    mtxLibera();

    setEstadoInf(H8LR_LIB_INF_ESPERA_INF);

    estado = getEstadoInf();
    while( estado == H8LR_LIB_INF_ESPERA_INF)
    {
        sleepUS(100);
        estado = getEstadoInf();
    }

    auto fin = std::chrono::high_resolution_clock::now(); // Finaliza el cronómetro
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio);
    // std::cout << "Tiempo de ejecución interno: " << duracion.count() << " ms\n";

    if ( estado == H8LR_LIB_INF_ESPERA_DATA )
        return true;

    setEstadoInf(H8LR_LIB_INF_ESPERA_DATA);
    return false;
}

/**
 * Convierte un valor quantizado a float
 */
float Hailo8LRunner::dequantize(uint8_t value, hailo_quant_info_t *quant_info )
{
    // Aplicar la ecuación de conversión
    float valuef;
    float result;
    
    valuef = value;
    result = (valuef - quant_info->qp_zp) * quant_info->qp_scale;
    
    // Opcional: Asegurar que el valor esté dentro de los límites
    if (result < quant_info->limvals_min) 
        result = quant_info->limvals_min;

    if (result > quant_info->limvals_max) 
        result = quant_info->limvals_max;

    return result;
}

/**
 * Retorna infomacion sobre los streams de salida
 */
std::vector<hailo_vstream_info_t> Hailo8LRunner::getOutVStreamsInfo()
{
    std::vector<hailo_vstream_info_t> out;

    mtxBloquea();
    out = outVStreamsInfo;
    mtxLibera();

    return out;
}




