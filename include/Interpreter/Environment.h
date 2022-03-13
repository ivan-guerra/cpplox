#pragma once

#include <any>
#include <string>
#include <unordered_map>
#include <memory>

#include "Scanner.h"

namespace lox
{
/*!
 * \class Environment
 * \brief The Environment class stores variable to value bindings.
 *
 * Environment objects come in handy when implementing lexical scope. The
 * Environment class provides a convenience wrapper around a std::unordered_map
 * which maps variable lexemes to values. Environment objects are recursively
 * defined: each Environment points to its parent scope's Environment. The
 * latter is useful when implementing nesting/shadowing of scopes.
 */
class Environment
{
public:

    /*!
     * \brief Construct the root Environment.
     *
     * The root Environment has no parent scope.
     */
    Environment() : enclosing_(nullptr) { }

    /*!
     * \brief Define a child Environment.
     *
     * \param env Pointer to this Environment's parent scope.
     */
    Environment(const std::shared_ptr<Environment>& env) : enclosing_(env) { }

    ~Environment() = default;

    /* Allow default copy construction and assignment. */
    Environment(const Environment&) = default;
    Environment& operator=(const Environment&) = default;

    /* Allow default move construction and assignment. */
    Environment(Environment&&) = default;
    Environment& operator=(Environment&&) = default;

    /*!
     * \brief Define a variable.
     *
     * Define() adds a variable with name \a name and value \a value to the
     * environment. No checks are performed.
     */
    void Define(const std::string& name, const std::any& value)
        { env_[name] = value; }

    /*!
     * \brief Get the value of the variable with name \a name.
     *
     * \param name Token containing the name of the variable to be fetched.
     *
     * \return The value of \a name if \a name has been registered in this or
     *         an ancestor Environment. If the \a name has never been
     *         registered, Get() will throw a RuntimeError.
     */
    std::any Get(const Token& name) const;

    /*!
     * \brief Assign \a value to the variable with name \a name.
     *
     * If no variable name \a name exists in this or any enclosing environment,
     * a RuntimeError is thrown.
     */
    void Assign(Token name, const std::any& value);

private:
    std::shared_ptr<Environment>              enclosing_; /*!< Pointer to parent Environment. */
    std::unordered_map<std::string, std::any> env_;       /*!< Map of variable names to their values. */
}; // end Environment
} // end lox
