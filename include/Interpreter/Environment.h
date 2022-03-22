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
class Environment :
    public std::enable_shared_from_this<Environment>
{
public:
    /*!
     * \brief Construct the root Environment.
     *
     * The root Environment has no parent scope.
     */
    Environment() : enclosing_(nullptr) { }

    ~Environment() = default;

    /* Allow default copy construction and assignment. */
    Environment(const Environment&) = default;
    Environment& operator=(const Environment&) = default;

    /* Allow default move construction and assignment. */
    Environment(Environment&&) = default;
    Environment& operator=(Environment&&) = default;

    /*!
     * \brief Set this Environment object's parent Environment pointer.
     *
     * \param enclosing_env The parent (or enclosing) Environment of this
     *                      child Environment object.
     */
    void SetEnclosingEnv(std::shared_ptr<Environment> enclosing_env)
        { enclosing_ = enclosing_env; }

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
     * \brief Return the value associated with \a name \a distance envs back.
     *
     * GetAt() walks back through the Environment list \a distance nodes and
     * retrieves the value of \a name.
     */
    std::any GetAt(int distance, const std::string& name)
        { return Ancestor(distance)->env_[name]; }

    /*!
     * \brief Assign \a value to the variable with name \a name.
     *
     * If no variable name \a name exists in this or any enclosing environment,
     * a RuntimeError is thrown.
     */
    void Assign(const Token& name, const std::any& value);

    /*!
     * \brief Assign \a value to \a name at the Environment \a distance nodes
     * back in the env list.
     */
    void AssignAt(int distance, const Token& name, const std::any& value)
        { Ancestor(distance)->env_[name.GetLexeme()] = value; }

private:
    using EnvPtr = std::shared_ptr<Environment>;
    using EnvMap = std::unordered_map<std::string, std::any>;

    /*!
     * \brief Return a pointer to this \a distance'th Environment's ancestor.
     *
     * \return Ancestor() will return a pointer to the Environment \a distance
     *         Environment's back in the Environment list.
     */
    EnvPtr Ancestor(int distance);

    EnvPtr enclosing_; /*!< Pointer to parent Environment. */
    EnvMap env_;       /*!< Map of variable names to their values. */
}; // end Environment
} // end lox
