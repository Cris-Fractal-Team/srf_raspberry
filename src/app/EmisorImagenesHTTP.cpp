
#include <thread>
#include <chrono>

#include "app/EmisorImagenesHTTP.h"
#include "lib/utils/fileutils.h"
#include "app/MenuPrincipalCommand.h"


 /**
 * Constructor
 */
GetImageCommand::GetImageCommand()
{
    setRuta("/image","GET");
    sesionOblogatoria = true;
}

/**
 * Destructor
 */
GetImageCommand::~GetImageCommand()
{

}

/**
 * Procesa una peticion
 */
void GetImageCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion )
{
    GImage imagen;
    std::vector<uchar> jpegBuffer = serverImg->getJpegBuffer();

    uchar* data = jpegBuffer.data(); 
    size_t dataSize = jpegBuffer.size();

    response->setBinaryResponse((void*)data, dataSize, "image/jpeg");
}


/**
 * Constructor
 */
GetVideoCommand::GetVideoCommand()
{
    setRuta("/video","GET");
    sesionOblogatoria = true;
}

/**
 * Destructor
 */
GetVideoCommand::~GetVideoCommand()
{

}



/**
 * Procesa una peticion
 */
void GetVideoCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion )
{
    GSocket socket;
    std::chrono::milliseconds intervaloEspera(100);
       
    cout << "Nuevo cliente de video" << endl;

    response->setContentType("multipart/x-mixed-replace; boundary=---video-frame\r\n");
    response->setCustomResponse("HTTP/1.1 200 OK\r\n",NULL,0);
    
    this->serverImg->addVideoResponse(response, false);

    socket = response->getSocket();
    
    // bucle infinito hasta que se corte el socket
    while( socket.estaConectado() == 1 )
    {       
        std::this_thread::sleep_for(intervaloEspera);          
    }

    cout << "Cliente Video Finalizo" << endl;
}


/**
 * Constructor
 */
GetPreviewCommand::GetPreviewCommand()
{
    setRuta("/preview","GET");
    sesionOblogatoria = true;
}

/**
 * Destructor
 */
GetPreviewCommand::~GetPreviewCommand()
{

}



/**
 * Procesa una peticion
 */
void GetPreviewCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion )
{
    GSocket socket;
    std::chrono::milliseconds intervaloEspera(100);
       
    cout << "Nuevo cliente de preview" << endl;

    response->setContentType("multipart/x-mixed-replace; boundary=---video-frame\r\n");
    response->setCustomResponse("HTTP/1.1 200 OK\r\n",NULL,0);
    
    this->serverImg->addVideoResponse(response,true);

    socket = response->getSocket();
    
    // bucle infinito hasta que se corte el socket
    while( socket.estaConectado() == 1 )
    {       
        std::this_thread::sleep_for(intervaloEspera);          
    }

    cout << "Cliente Preview Finalizo" << endl;
}



/**
 * Constructor
 */
LoginCommand::LoginCommand()
{
    setRuta("/login","GET_POST");
}

/**
 * Destructor
 */
LoginCommand::~LoginCommand()
{

}

/**
 * Procesa una peticion
 */
void LoginCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion )
{
    string metodo = request->getMetodo();
    shared_ptr<GHashMap> lstValoresTag = make_shared<GHashMap>();

    // cout << "Metodo recibido :" << metodo<< endl;

    if ( request->getMetodo().compare("GET") == 0 )
    {
        string html = leeArchivoTexto("./www/login.html");
        html = reemplazaTagHtml(html, lstValoresTag);
        response->setHtmlResponse(html);
    }
    else
    if ( request->getMetodo().compare("POST") == 0 )
    {
        string usuario = request->getParam("usuario");
        string password = request->getParam("password");

        if (( usuario.compare("admin") == 0 ) && ( password.compare("123456") == 0 ))
        {           
            shared_ptr<GHttpServerSession> session = this->servidor->creaNuevaSesion(response);
            servidor->redirecciona("/menuprincipal", request, response);
        }
        else
        {
            string html = leeArchivoTexto("./www/login.html");
            string htmlError = "<b>Credenciales Invalidas</b>";

            lstValoresTag->putString("msgError", htmlError);
            html = reemplazaTagHtml(html, lstValoresTag);
            response->setHtmlResponse(html);
        }
    }
}


/**
 * Constructor
 */
ServidorHttpImagenes::ServidorHttpImagenes()
{
    shared_ptr<LoginCommand> loginCmd = make_shared<LoginCommand>();
    shared_ptr<GetImageCommand> imageCmd = make_shared<GetImageCommand>();
    shared_ptr<GetVideoCommand> videoCmd = make_shared<GetVideoCommand>();
    shared_ptr<GetPreviewCommand> previewCmd = make_shared<GetPreviewCommand>();
    shared_ptr<MenuPrincipalCommand> menuPrincipalCmd = make_shared<MenuPrincipalCommand>();
        
    // Agrega los procesadores de comando
    servidor.addCommand(loginCmd);
    servidor.addCommand(imageCmd);
    servidor.addCommand(videoCmd);
    servidor.addCommand(previewCmd);
    servidor.addCommand(menuPrincipalCmd);

    // configura el proveedor de contenido estatico
    // todos los urls de contenido estatico deberian ir relativos a esta ruta
    shared_ptr<GHttpStaticContentCommand> staticCmd = make_shared<GHttpStaticContentCommand>();

    staticCmd->setRuta("/resources/*","GET");
    staticCmd->pathBaseArchivos = "./www/resources/";
    staticCmd->cargaContentTypes("./config/contentypes.txt");
    servidor.addCommand(staticCmd);

    imageCmd->serverImg = this;
    videoCmd->serverImg = this;
    previewCmd->serverImg = this;
    loginCmd->serverImg = this;
    menuPrincipalCmd->serverImg = this;

    imagenNueva = false;
}

/**
 * Inicia el servidor
 */
void ServidorHttpImagenes::iniciar()
{
    std::thread *th,*th1;

    th = new thread( &ServidorHttpImagenes::ejecutaServidor,this);
    th1 = new thread( &ServidorHttpImagenes::bucleEnvioImagenes,this);
}

 /**
 * Metodo que ejecuta el bucle del servidor
 */
void ServidorHttpImagenes::ejecutaServidor()
{
    cout << "Inicia el thread del servidor web" << endl;

    servidor.iniciar("", 8080);

    cout << "Finaliza el thread del servidor web" << endl;
}



/**
 * Finaliza el servidor
 */
void ServidorHttpImagenes::finalizar()
{
    servidor.finalizar();
}

 /**
 * Metodo que envia las imagenes a los visores
 * Se ejecuta dentro de un Thread
 */
void ServidorHttpImagenes::bucleEnvioImagenes()
{
    int i,n;
    bool hayImagenNueva;
    GSocket socket;
    string finMsg = "\r\n";
    shared_ptr<GHttpResponse>response;
    std::chrono::milliseconds intervaloEspera(5);
    std::chrono::milliseconds intervaloEsperaIni(5000);

    std::this_thread::sleep_for(intervaloEsperaIni);

    cout << "<<<< Iniciando Thread de previsualizacin" << endl;
    while( servidor.enEjecucion() )
    {    
        mtx.lock();
        hayImagenNueva = imagenNueva;
        mtx.unlock();   

        if ( hayImagenNueva == false )
        {
            std::this_thread::sleep_for(intervaloEspera);
            continue;
        }

        // codifica la imagen como JPEG
        jpegBuffer.clear();
        jpegPreviewBuffer.clear();

        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};  // Calidad JPEG (opcional)

        n = lstVideoResponses.size();
        if ( n > 0 )
        {
            bool success = cv::imencode(".jpg", currentImage.imagenOpencv, jpegBuffer, params);
            
            // obtiene referencia al buffer y sus dimensiones
            uchar* data = jpegBuffer.data(); 
            size_t dataSize = jpegBuffer.size();
            
            if ( success )
            {
                // envia la imagen a todos los clientes de video          
                for(i=n-1; i >= 0; i--)
                {
                    response = dynamic_pointer_cast<GHttpResponse>(lstVideoResponses.get(i));
                    socket = response->getSocket();
                    if ( socket.estaConectado() == 0 )
                    {
                        lstVideoResponses.remove(i);
                        continue;
                    }
                                
                    string header = "---video-frame\r\nContent-Type: image/jpeg\r\nContent-Length: " +
                        to_string(dataSize) + "\r\n\r\n";

                    socket.write((char *)header.c_str(), header.size());
                    socket.write((char*)data, dataSize);
                    socket.write((char*)finMsg.c_str(),2);
                }
            }
        }

        n = lstPreviewResponses.size();
        if ( n > 0 )
        {        
            bool success = cv::imencode(".jpg", currentImage.cloneResize(640,360).imagenOpencv, jpegPreviewBuffer, params);    
        
            // obtiene referencia al buffer y sus dimensiones
            uchar* data = jpegPreviewBuffer.data(); 
            size_t dataSize = jpegPreviewBuffer.size();
            
            if ( success )
            {
                // envia la imagen a todos los clientes de video            
                for(i=n-1; i >= 0; i--)
                {
                    response = dynamic_pointer_cast<GHttpResponse>(lstPreviewResponses.get(i));
                    socket = response->getSocket();
                    if ( socket.estaConectado() == 0 )
                    {
                        lstPreviewResponses.remove(i);
                        continue;
                    }
                                
                    string header = "---video-frame\r\nContent-Type: image/jpeg\r\nContent-Length: " +
                        to_string(dataSize) + "\r\n\r\n";

                    socket.write((char *)header.c_str(), header.size());
                    socket.write((char*)data, dataSize);
                    socket.write((char*)finMsg.c_str(),2);
                }
            }
        }

        mtx.lock();
        imagenNueva = false;
        mtx.unlock();
    }

    cout << ">>> Finaliza Thread de previsualizacion" << endl;
}

 /**
 * Establece la imagen actual del sensor
 */
void ServidorHttpImagenes::setImage( GImage imagen )
{
    mtx.lock();
    if ( imagenNueva == false )
    {
        currentImage = imagen; 
        imagenNueva = true;
    }
    mtx.unlock();
}

/**
 * Agrega un response para que se envie video
 * El video puede o no ser en modo de preview es decir con resolucion mas baja
 */
void ServidorHttpImagenes::addVideoResponse( shared_ptr<GHttpResponse> response, bool esPreview )
{
    if ( esPreview == false ) lstVideoResponses.add(response);
    else lstPreviewResponses.add(response);
}

/**
 * Retorna el buffer de la ultima imagen
 */
std::vector<uchar> ServidorHttpImagenes::getJpegBuffer()
{
    return jpegBuffer;
}

