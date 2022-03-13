#pragma once

#include <string>
#include <exception>

#include "Scanner.h"

namespace lox
{
/*!
 * \class RuntimeError
 * \brief The RuntimeError class represents a lox runtime error.
 *
 * We don't want lox throwing C++ exceptions or crashing when faced with
 * erroneous runtime behavior. When the interpreter encounters an error
 * it will throw a RuntimeError that is handled by Interpreter::Interpret()
 * gracefully.
 */
class RuntimeError : public std::exception
{
public:
    RuntimeError(const Token& token_, const std::string& message_) :
        token(token_), message(message_) { }

    Token       token;   /*!< Last Token processed before this error was thrown. */
    std::string message; /*!< Message describing the runtime error. */
}; // end RuntimeError
} // end lox
