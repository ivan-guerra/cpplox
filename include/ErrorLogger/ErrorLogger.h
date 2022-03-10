#pragma once

#include <iostream>

#define LOG_STATIC_ERROR(line, msg) \
    lox::ErrorLogger::GetInstance().ReportStaticError(std::cerr, line, msg)

#define LOG_RUNTIME_ERROR(line, msg) \
    lox::ErrorLogger::GetInstance().ReportRuntimeError(std::cerr, line, msg)

namespace lox
{
/*!
 * \class ErrorLogger
 * \brief Error message logger.
 *
 * The ErrorLogger class provides an interface for logging interpreter static
 * and runtime error messages to an output stream. Internally, ErrorLogger
 * maintains an error flag for each error type (static and runtime). The
 * ErrorLogger API allows access to a singleton instance of ErrorLogger that
 * can be used to query/clear the error flags and log messages to any
 * std::ostream. Convenience macros, LOG_STATIC_ERROR and LOG_RUNTIME_ERROR,
 * are provided to allow easy logging to stderr.
 */
class ErrorLogger
{
public:
    /* Disable copy construction and assignment. */
    ErrorLogger(const ErrorLogger&) = delete;
    ErrorLogger& operator=(const ErrorLogger&) = delete;

    /* Disable move construction and assignment. */
    ErrorLogger(ErrorLogger&&) = delete;
    ErrorLogger& operator=(ErrorLogger&&) = delete;

    /*!
     * \brief Return the singleton instance of ErrorLogger.
     */
    static ErrorLogger& GetInstance();

    /*!
     * \brief Return the static error flag.
     *
     * \return Return \c true if an error has been reported via a call to
     *         ReportStaticError(). If the error flag has been cleared via a
     *         call to ClearStaticError() and no other errors have since been
     *         reported, return \c false.
     */
    bool HadStaticError() const { return had_static_error_; }

    /*!
     * \brief Return the runtime error flag.
     *
     * \return Return \c true if an error has been reported via a call to
     *         ReportRuntimeError(). If the error flag has been cleared via a
     *         call to ClearRuntimeError() and no other errors have since been
     *         reported, return \c false.
     */
    bool HadRuntimeError() const { return had_runtime_error_; }

    /*!
     * \brief Clear the static error flag.
     */
    void ClearStaticError() { had_static_error_ = false; }

    /*!
     * \brief Clear the runtime error flag.
     */
    void ClearRuntimeError() { had_runtime_error_ = false; }

    /*!
     * \brief Print a static error message and line number to an output stream.
     *
     * \param os Output stream object.
     * \param line Line number on which the error occurred.
     * \param msg Error message.
     */
    void ReportStaticError(std::ostream& os, int line, const std::string& msg);

    /*!
     * \brief Print a runtime error message and line number to an output stream.
     *
     * \param os Output stream object.
     * \param line Line number on which the error occurred.
     * \param msg Error message.
     */
    void ReportRuntimeError(std::ostream& os, int line, const std::string& msg);

private:
    /*!
     * \brief Print an error message, line number, and location to \a os.
     *
     * \param os Output stream object.
     * \param line Line number on which the error occurred.
     * \param where Further error location information.
     * \param msg Error message.
     */
    void Report(std::ostream& os, int line, const std::string& where,
                const std::string msg);

    ErrorLogger() :
        had_static_error_(false), had_runtime_error_(false) { }

    bool had_static_error_;  /*!< Flag indicating the interpreter has encountered a static error. */
    bool had_runtime_error_; /*!< Flag indicating the interpreter has encountered a runtime error. */
}; // end ErrorLogger
} // end lox
