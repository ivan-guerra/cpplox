#include <iostream>

#include "ErrorLogger.h"

namespace lox
{
void ErrorLogger::Report(std::ostream& os, int line, const std::string& where,
                         const std::string msg)
{
    os << "[line " << line << "] Error" << where << ": " << msg << std::endl;
}

ErrorLogger& ErrorLogger::GetInstance()
{
    static ErrorLogger logger;
    return logger;
}

void ErrorLogger::ReportStaticError(std::ostream& os, int line,
                                    const std::string& msg)
{
    had_static_error_ = true;
    Report(os, line, "", msg);
}

void ErrorLogger::ReportRuntimeError(std::ostream& os, int line,
                                     const std::string& msg)
{
    had_runtime_error_ = true;
    Report(os, line, "", msg);
}
} // end lox
