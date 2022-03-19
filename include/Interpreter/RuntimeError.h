#pragma once

#include <any>
#include <string>
#include <stdexcept>

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
struct RuntimeError :
    public std::runtime_error
{
public:
    RuntimeError() = delete;

    /*!
     * \brief Construct a Lox runtime error with error token and message.
     *
     * \param token_ Last token seen at the moment the error was detected.
     * \param message_ Error message to that will be reported to the User.
     */
    RuntimeError(const Token& token_, const std::string& message_) :
        std::runtime_error(message_.c_str()),
        token(token_),
        message(message_)
    { }

    RuntimeError(const RuntimeError&) = default;
    RuntimeError& operator=(const RuntimeError&) = default;

    RuntimeError(RuntimeError&&) = default;
    RuntimeError& operator=(RuntimeError&&) = default;

    Token       token;   /*!< Last Token processed before this error was thrown. */
    std::string message; /*!< Message describing the runtime error. */
}; // end RuntimeError

/*!
 * \class Return
 * \brief The Return class is used to return values from Lox functions.
 *
 * When a Lox function call completes a Return exception is thrown containing
 * the return value of the function. Return provides a way for us unwind
 * the interpreter's call stack while capturing the return value of the
 * most recently executed User function.
 */
struct Return :
    public std::runtime_error
{
public:
    Return() = delete;

    /*!
     * \brief Constructs a Return exception with a return value.
     *
     * \param value_ The value being returned by the most recently executed
     *               function.
     */
    Return (const std::any& value_) :
        std::runtime_error(""),
        value(value_)
    { }

    Return(const Return&) = default;
    Return& operator=(const Return&) = default;

    Return(Return&&) = default;
    Return& operator=(Return&&) = default;

    std::any value; /*!< Lox function return value. */
}; // end Return
} // end lox
