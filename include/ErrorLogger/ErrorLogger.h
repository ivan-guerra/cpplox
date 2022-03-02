#pragma once

#include <iostream>

#define LOG_ERROR(line, msg) \
    lox::ErrorLogger::GetInstance().ReportError(std::cerr, line, msg)

namespace lox
{
/*!
 * \class ErrorLogger
 * \brief Error message logger.
 *
 * The ErrorLogger class provides an interface for logging interpreter error
 * messages to an output stream. Internally, ErrorLogger maintains an error
 * flag. The ErrorLogger API allows access to a singleton instance of
 * ErrorLogger that can be used to query/clear the error flag and log messages
 * to any std::ostream. A convenience macro, LOG_ERROR, is provided to allow
 * easy logging to stderr.
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
     * \brief Return the error flag.
     *
     * \return Return \c true if an error has been reported via a call to
     *         ReportError(). If the error flag has been cleared via a call to
     *         ClearError() and no other errors have since been reported,
     *         return \c false.
     */
    bool HadError() const { return had_error_; }

    /*!
     * \brief Clear the error flag.
     */
    void ClearError() { had_error_ = false; }

    /*!
     * \brief Print an error message and line number to an output stream.
     *
     * \param os Output stream object.
     * \param line Line number on which the error occurred.
     * \param msg Error message.
     */
    void ReportError(std::ostream& os, int line, const std::string& msg)
        { Report(os, line, "", msg); }

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

    ErrorLogger() : had_error_(false) { }

    bool had_error_; /*!< Flag indicating the interpreter has encountered an error. */
}; // end ErrorLogger
} // end lox
