
#include <thread>
#include <chrono>

#include "app/EmisorImagenesHTTP.h"
#include "lib/utils/fileutils.h"
#include "app/ProcRecFacial.h"
#include "app/MenuPrincipalCommand.h"


/**
 * Constructor
 */
MenuPrincipalCommand::MenuPrincipalCommand()
{
    setRuta("/menuprincipal","GET_POST");
    sesionOblogatoria = true;    
}

/**
 * Destructor
 */
MenuPrincipalCommand::~MenuPrincipalCommand()
{

}

/**
 * Procesa una peticion
 */
void MenuPrincipalCommand::procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion )
{
    shared_ptr<GHashMap> lstValoresTag = make_shared<GHashMap>();

    cout << "MenuPrincipal - Procesa Request " << redireccion << endl;

    if ( redireccion == false )
    {
        string valor;

        cout << "NO hay redireccion, guardar = " << valor << endl;
       
        valor = request->getParam("operacion");
        cout << "Operacion: " << valor << endl;        
       
        if ( valor == "cmbRangoHora_onChange" )
        {
            cout << "Antes de procesar cmbRangoHora_onChage" << endl;
            cmbRangoHora_onChange(request, lstValoresTag);
        }
        else
        if ( valor == "cmbBrillo_onChange" )
        {
            cout << "Antes de procesar cmbBrillo_onChange" << endl;
            cmbBrillo_onChange(request,lstValoresTag);
        }
        else
        if ( valor == "cmbContraste_onChange" )
        {
            cout << "Antes de procesar cmbContraste_onChange" << endl;
            cmbContraste_onChange(request,lstValoresTag);
        }
        else
        if ( valor == "txtExposicion_onChange" )
        {
            cout << "Antes de procesar txtExposicion_onChange" << endl;
            txtExposicion_onChange(request,lstValoresTag);
        }
        else
        if ( valor == "cmbReflejarVerticalmente_onChange" )
        {
            cout << "Antes de procesar cmbReflejarVerticalmente_onChange" << endl;
            cmbReflejarVerticalmente_onChange(request,lstValoresTag);
        }
        else
        if ( valor == "btnGuardar_onClick" )
        {
            cout << "Antes de procesar btnGuardar_onClick" << endl;
            btnGuardar_onClick(request,lstValoresTag);
        }
        else
        if ( valor == "btnSalir_onClick" )
        {
            cout << "Antes de procesar btnSalir_onClick" << endl;
            btnSalir_onClick(request,response);
            return;
        }
    }

    string html = leeArchivoTexto("./www/menuPrincipal.html");    
    calculaValoresMenuPrincipal(lstValoresTag);
    html = reemplazaTagHtml(html, lstValoresTag);
        
    response->setHtmlResponse(html);
}

/**
 * Procesa el evento sobre el combobox que indica el rango de horas sobre el que se trabaja
 */
void MenuPrincipalCommand::cmbRangoHora_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag )
{
    string param = request->getParam("cmbRangoHora");
    double brillo = request->getParamDouble("cmbBrillo", 0.0);
    double contraste = request->getParamDouble("cmbContraste", 0.0);
    long exposicion = request->getParamLong("txtExposicion", 15000);
    
    cout << "cmbRangoHora_onChange, brillo: " << brillo << " contraste: " << contraste << " exposicion: " << exposicion << endl;

    serverImg->procRecFacial->imageSource->ajustaVideo(PARAM_VIDEO_BRILLO, brillo );
    serverImg->procRecFacial->imageSource->ajustaVideo(PARAM_VIDEO_CONTRASTE, contraste );
    serverImg->procRecFacial->imageSource->ajustaVideo(PARAM_VIDEO_EXPOSICION, exposicion );
}

/**
 * Procesa el evento sobre el combobox que indica el brillo se ha cambiado
 */
void MenuPrincipalCommand::cmbBrillo_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag )
{    
    double brillo = request->getParamDouble("cmbBrillo", 0.0);
    
    cout << "cmbBrillo_onChange, brillo: " << brillo << endl;
    serverImg->procRecFacial->imageSource->ajustaVideo(PARAM_VIDEO_BRILLO, brillo );
}

/**
 * Procesa el evento sobre el combobox que indica el brillo se ha cambiado
 */
void MenuPrincipalCommand::cmbContraste_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag )
{    
    double contraste = request->getParamDouble("cmbContraste", 0.0);
    
    cout << "cmbContraste_onChange, contraste: " << contraste << endl;    
    serverImg->procRecFacial->imageSource->ajustaVideo(PARAM_VIDEO_CONTRASTE, contraste );
}

/**
 * Procesa el evento que procesa el cambio en la exposicion
 */
void MenuPrincipalCommand::txtExposicion_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag )
{
    long exposicion = request->getParamLong("txtExposicion", 15000);
    
    cout << "txtExposicion_onChange, exposicion: " << exposicion << endl;    
    serverImg->procRecFacial->imageSource->ajustaVideo(PARAM_VIDEO_EXPOSICION, exposicion );
}

/**
 * Procesa el evento sobre el combobox que ha cambiado el indicador de reflejo vertical
 */
void MenuPrincipalCommand::cmbReflejarVerticalmente_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag )
{
    string reflejar;
    
    reflejar = request->getParam("cmbReflejarVerticalmente");
    serverImg->lstParamsApp->putString("reflejarVerticalmente", reflejar); 
}

/**
 * Procesa el evento que procesa el cambio en la exposicion
 */
void MenuPrincipalCommand::btnGuardar_onClick( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag )
{
    string param,nombreParam,indice;
    int i;

    param = request->getParam("girar");
    serverImg->lstParamsApp->putString("girar", param); 

    param = request->getParam("reflejarVerticalmente");
    serverImg->lstParamsApp->putString("reflejarVerticalmente", param); 

    param = request->getParam("urlBaseServidor");
    serverImg->lstParamsApp->putString("urlBaseServidor", param); 

    param = request->getParam("reportarDesconocidos");
    serverImg->lstParamsApp->putString("reportarDesconocidos", param); 

    for(i=1;i<=3;i++)
    {
        indice = to_string(i);
        
        nombreParam = "horaInicio";
        nombreParam.append(to_string(i));        
        param = request->getParam(nombreParam);
        serverImg->lstParamsApp->putString(nombreParam, param); 

        nombreParam = "horaFin";
        nombreParam.append(to_string(i));        
        param = request->getParam(nombreParam);
        serverImg->lstParamsApp->putString(nombreParam, param); 

        nombreParam = "brillo";
        nombreParam.append(to_string(i));        
        param = request->getParam(nombreParam);
        serverImg->lstParamsApp->putString(nombreParam, param); 

        nombreParam = "contraste";
        nombreParam.append(to_string(i));        
        param = request->getParam(nombreParam);
        serverImg->lstParamsApp->putString(nombreParam, param); 

        nombreParam = "exposicion";
        nombreParam.append(to_string(i));        
        param = request->getParam(nombreParam);
        serverImg->lstParamsApp->putString(nombreParam, param); 
    }

    serverImg->procRecFacial->guardaParametros();
    cout << "Configuracion guardada" << endl;
}


/**
 * Procesa el evento click para salir de la pantalla de configuracion
 */
void MenuPrincipalCommand::btnSalir_onClick( shared_ptr<GHttpRequest> request,  shared_ptr<GHttpResponse> response )
{
    string html = leeArchivoTexto("./www/salida.html");    
       
    servidor->delSession(servidor->getSession(request)->idSession);
    response->setHtmlResponse(html);
}


/**
 * Actuliza los parametros a partir del formulario enviado
 * 
 *  request : peticion web recibida
 * 
 *  lstValoresTag : Hashmap con la lista de valores que se deben visualizar en el HTML
 */
void MenuPrincipalCommand::procesaActualizaParams( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag )
{
    string txt,txtError;

    txtError = "";

    cout << "Procesando Guardar " << endl;
    
    txt = request->getParam("urlBaseServidor");
    if ( txt.length() == 0 )
    {
        txtError+= "El URL base del servidor es requerido<br>";
    }

    txt = request->getParam("girar");
    if ( txt.length() == 0 )
    {
        txtError+= "El angulo de giro es requerido<br>";
    }

    txt = request->getParam("reportarDesconocidos");
    if ( txt.length() == 0 )
    {
        txtError+= "Se debe indicar si se desea o no reportar desconocidos<br>";
    }

    txt = request->getParam("reflejarVerticalmente");
    if ( txt.length() == 0 )
    {
        txtError+= "Se debe indicar si se desea o no reflejar verticalmente las imagenes<br>";
    }
    else
    {
        cout << "Reflejar Verticalmente " << txt << endl;
    }

    txt = request->getParam("brillo1");
    if ( txt.length() == 0 )
    {
        txtError+= "Se debe especificar el brillo para el rango 1 de horas<br>";
    }
    else
    {
        cout << "Brillo1 :" << txt << endl;
    }

    if ( txtError.length() > 0 )
    {
        lstValoresTag->putString("msgError", txtError);
        return;
    }    

    serverImg->lstParamsApp->putString("urlBaseServidor", request->getParam("urlBaseServidor"));
    serverImg->lstParamsApp->putString("girar", request->getParam("girar"));
    serverImg->lstParamsApp->putString("reportarDesconocidos", request->getParam("reportarDesconocidos"));
    serverImg->lstParamsApp->putString("reflejarVerticalmente", request->getParam("reflejarVerticalmente"));    

    serverImg->lstParamsApp->putString("brillo1", request->getParam("brillo1"));
}


/**
 * Calcula los valores que se muestran en el HTML del Menu Principal
 * Este me
 */
void MenuPrincipalCommand::calculaValoresMenuPrincipal( shared_ptr<GHashMap> lstValores )
{
    int giro;
    bool valorLogico;
    string idControl;
    
    lstValores->putString("versionSensor","N");
    lstValores->putString("numSerie","234234vcc");

    giro = serverImg->lstParamsApp->getStringLong("girar",0);
    if ( giro == 90 )
    {
        lstValores->putString("girar_90"," selected ");
    }
    else
    if ( giro == -90 )
    {
        lstValores->putString("girar_-90"," selected ");
    }
    else
    {
        lstValores->putString("girar_0"," selected ");
    }

    lstValores->putString("urlBaseServidor", serverImg->lstParamsApp->getString("urlBaseServidor"));

    valorLogico = serverImg->lstParamsApp->getStringBool("reportarDesconocidos", false);
    if ( valorLogico )
    {
        lstValores->putString("reportarDesconocidos_si"," selected ");
    }
    else
    {
        lstValores->putString("reportarDesconocidos_no"," selected ");
    }

    valorLogico = serverImg->lstParamsApp->getStringBool("reflejarVerticalmente", false);
    if ( valorLogico )
    {
        lstValores->putString("reflejarVerticalmente_si"," selected ");
    }
    else
    {
        lstValores->putString("reflejarVerticalmente_no"," selected ");
    }

    string jsonConfiImagenes = "[";

    jsonConfiImagenes+= "{";
    jsonConfiImagenes+= "\"horaInicio\":"+ to_string(serverImg->lstParamsApp->getStringDouble("horaInicio1",0)) + ",";
    jsonConfiImagenes+= "\"horaFin\":"+ to_string(serverImg->lstParamsApp->getStringDouble("horaFin1",0)) + ",";
    jsonConfiImagenes+= "\"brillo\":"+ to_string(serverImg->lstParamsApp->getStringDouble("brillo1",0)) + ",";
    jsonConfiImagenes+= "\"contraste\":"+ to_string(serverImg->lstParamsApp->getStringDouble("contraste1",0)) + ",";
    jsonConfiImagenes+= "\"exposicion\":"+ to_string(serverImg->lstParamsApp->getStringLong("exposicion1",25000)) ;
    jsonConfiImagenes+= "},";

    jsonConfiImagenes+= "{";
    jsonConfiImagenes+= "\"horaInicio\":"+ to_string(serverImg->lstParamsApp->getStringDouble("horaInicio2",0)) + ",";
    jsonConfiImagenes+= "\"horaFin\":"+ to_string(serverImg->lstParamsApp->getStringDouble("horaFin2",0)) + ",";
    jsonConfiImagenes+= "\"brillo\":"+ to_string(serverImg->lstParamsApp->getStringDouble("brillo2",0)) + ",";
    jsonConfiImagenes+= "\"contraste\":"+ to_string(serverImg->lstParamsApp->getStringDouble("contraste2",0)) + ",";
    jsonConfiImagenes+= "\"exposicion\":"+ to_string(serverImg->lstParamsApp->getStringLong("exposicion2",25000)) ;
    jsonConfiImagenes+= "},";

    jsonConfiImagenes+= "{";
    jsonConfiImagenes+= "\"horaInicio\":"+ to_string(serverImg->lstParamsApp->getStringDouble("horaInicio3",0)) + ",";
    jsonConfiImagenes+= "\"horaFin\":"+ to_string(serverImg->lstParamsApp->getStringDouble("horaFin3",0)) + ",";
    jsonConfiImagenes+= "\"brillo\":"+ to_string(serverImg->lstParamsApp->getStringDouble("brillo3",0)) + ",";
    jsonConfiImagenes+= "\"contraste\":"+ to_string(serverImg->lstParamsApp->getStringDouble("contraste3",0)) + ",";
    jsonConfiImagenes+= "\"exposicion\":"+ to_string(serverImg->lstParamsApp->getStringLong("exposicion3",25000)) ;
    jsonConfiImagenes+= "}";

    jsonConfiImagenes+= "]";

    lstValores->putString("configImagenes", jsonConfiImagenes);
}

