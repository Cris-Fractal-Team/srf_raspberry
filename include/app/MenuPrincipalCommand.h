
#ifndef _MENU_PRINCIPAL_COMMAND
#define _MENU_PRINCIPAL_COMMAND

#include <thread>
#include <mutex>

#include "lib/web/GHttpServer.h"
#include "lib/web/GHttpServerCommand.h"
#include "lib/web/GHttpRequest.h"
#include "lib/web/GHttpResponse.h"
#include "lib/graphics/GDibujo.h"
#include "app/IdentificadorFacial.h"


class IdentificadorFacial;
class ServidorHttpImagenes;


/**
 * Clase que implementa la logica para procesar el menu principal
 */
class MenuPrincipalCommand : public GHttpServerCommand
{
    public:

        /**
         * Referencia al objeto que gestiona la WEB
         */
        ServidorHttpImagenes *serverImg;

        /**
         * Constructor
         */
        MenuPrincipalCommand();

        /**
         * Destructor
         */
        ~MenuPrincipalCommand();

        /**
         * Procesa una peticion
         */
        void procesaRequest( shared_ptr<GHttpRequest> request, shared_ptr<GHttpResponse> response, bool redireccion ) override;

    private:

        /**
         * Calcula los valores que se muestran en el HTML del Menu Principal
         * Este me
         */
        void calculaValoresMenuPrincipal( shared_ptr<GHashMap> lstValores );

        /**

        * Actuliza los parametros a partir del formulario enviado
        * 
        *  request : peticion web recibida
        * 
        *  lstValoresTag : Hashmap con la lista de valores que se deben visualizar en el HTML
        */
        void procesaActualizaParams( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );


        /**
        * Actuliza la configuracion de la visualizacion de imagenes
        * 
        *  request : peticion web recibida
        * 
        *  lstValoresTag : Hashmap con la lista de valores que se deben visualizar en el HTML
        */
        void procesaAjustaImagen( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );


        /**
         * Procesa el evento sobre el combobox que indica el rango de horas sobre el que se trabaja
         */
        void cmbRangoHora_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );

        /**
         * Procesa el evento sobre el combobox que ha cambiado el brillo
         */
        void cmbBrillo_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );

        /**
         * Procesa el evento sobre el combobox que ha cambiado el contraste
         */
        void cmbContraste_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );

        /**
         * Procesa el evento sobre el combobox que ha cambiado el indicador de reflejo vertical
         */
        void cmbReflejarVerticalmente_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );

        /**
         * Procesa el evento que procesa el cambio en la exposicion
         */
        void txtExposicion_onChange( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );

        /**
         * Procesa el evento click sobre el boton para guardar cambios
         */
        void btnGuardar_onClick( shared_ptr<GHttpRequest> request,  shared_ptr<GHashMap> lstValoresTag );

        /**
         * Procesa el evento click sobre el boton para procesar la salida de la pantalla de configuracion
         */
        void btnSalir_onClick( shared_ptr<GHttpRequest> request,  shared_ptr<GHttpResponse> response );
};


#endif