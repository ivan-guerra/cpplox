#include <iostream>

#include "ErrorLogger.h"

namespace lox
{
void ErrorLogger::Report(std::ostream& os, int line, const std::string& where,
                         const std::string msg)
{
    had_error_ = true;
    os << "[line " << line << "] Error" << where << ": " << msg << std::endl;
}

ErrorLogger& ErrorLogger::GetInstance()
{
    static ErrorLogger logger;
    return logger;
}
} // end lox
