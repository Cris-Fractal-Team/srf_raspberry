#ifndef _DEBUG_UTILS_H_
#define _DEBUG_UTILS_H_

#include <string>

namespace DebugUtils
{
    void logStdExceptionDebug(const char* component, const char* etapa, const std::exception& ex);
    void logUnknownExceptionDebug(const char* component, const char* etapa);
}

#endif
