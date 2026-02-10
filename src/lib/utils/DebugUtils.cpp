#include "lib/utils/DebugUtils.h"
#include "lib/utils/Logger.h"

#include <exception>
#include <string>

namespace DebugUtils
{
    void logStdExceptionDebug(const char* component, const char* etapa, const std::exception& ex)
    {
        std::string msg;
        msg.append("Bucle roto por std::exception, etapa=");
        msg.append(etapa ? etapa : "NULL");
        msg.append(" msg=");
        msg.append(ex.what());
        LOG_ERROR(component, msg);
    }

    void logUnknownExceptionDebug(const char* component, const char* etapa)
    {
        std::string msg;
        msg.append("Bucle roto por excepción desconocida, etapa=");
        msg.append(etapa ? etapa : "NULL");
        LOG_ERROR(component, msg);
    }
}
