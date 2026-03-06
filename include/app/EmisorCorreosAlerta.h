#ifndef _EMISOR_CORREOS_ALERTA_
#define _EMISOR_CORREOS_ALERTA_

#include <string>

class EmisorCorreosAlerta
{
public:
    std::string appWebUrl;
    std::string endpointAuth;
    std::string endpointEmailAlerta;
    std::string username;
    std::string password;
    std::string serieEquipo;

    void configure();
    void setSerieEquipo(const std::string& serie);

    void postEmailAlerta(double temperaturaC);

private:
    static constexpr const char* LOG_COMPONENT = "EmisorCorreosAlerta";

    std::string iniciarSesion();
    std::string construirUrl(const std::string& endpoint) const;
};

#endif
