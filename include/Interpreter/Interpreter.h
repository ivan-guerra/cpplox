#pragma once

#include <any>
#include <memory>
#include <exception>

#include "Expr.h"
#include "Scanner.h"

namespace lox
{
/*!
 * \class Interpreter
 * \brief The Interpreter class implements the Lox lang interpreter.
 *
 * The current state of the Interpreter allows us to evaluate Expressions.
 * Future revisions will implement the logic to evaluate additional lox code
 * at runtime.
 */
class Interpreter : public Visitor
{
public:
    Interpreter() = default;
    ~Interpreter() = default;

    /* Default copy construction and assignment is valid. */
    Interpreter(const Interpreter&) = default;
    Interpreter& operator=(const Interpreter&) = default;

    /* Default move construction and assignment is valid. */
    Interpreter(Interpreter&&) = default;
    Interpreter& operator=(Interpreter&&) = default;

    /*!
     * \brief Evaluate a binary expression.
     */
    std::any VisitBinaryExpr(Binary& expr) final;

    /*!
     * \brief Evaluate a parenthesized expression.
     */
    std::any VisitGroupingExpr(Grouping& expr) final
        { return Evaluate(expr.expression); }

    /*!
     * \brief Evaluate a literal expression.
     */
    std::any VisitLiteralExpr(Literal& expr) final
        { return expr.value; }

    /*!
     * \brief Evaluate a unary expression.
     */
    std::any VisitUnaryExpr(Unary& expr) final;

    /*!
     * \brief Evaluate \a expression.
     *
     * Interpret() will compute the value represented by \a expression. The
     * computed value is output to stdout.
     */
    void Interpret(std::shared_ptr<Expr> expression);
private:
    /*!
     * \class RuntimeError
     * \brief The RuntimeError class represents a lox runtime error.
     *
     * We don't want lox throwing C++ exceptions or crashing when faced with
     * erroneous runtime behavior. When the interpreter encounters an error
     * it will throw a RuntimeError that is handled by Interpret() gracefully.
     */
    class RuntimeError : public std::exception
    {
    public:
        RuntimeError(const Token& token_, const std::string& message_) :
            token(token_), message(message_) { }

        Token       token;   /*!< Last Token processed before this error was thrown. */
        std::string message; /*!< Message describing the runtime error. */
    }; // end RuntimeError

    /*!
     * \brief Return the result of the evaluating \a expr.
     */
    std::any Evaluate(std::shared_ptr<Expr> expr)
        { return expr->Accept(*this); }

    /*!
     * \brief Return \c true if \a object evaluates to true.
     *
     * In lox, we define \c false as \a nil or the literal \c false. All other
     * values are \c true.
     */
    bool IsTruth(const std::any& object) const;

    /*!
     * \brief Determine if \a left and \a right are equal.
     *
     * We only check for equality for string, number, boolean, and nil types.
     * An equality check of any other type or mismatched types has an automatic
     * return value of \c false.
     */
    bool IsEqual(const std::any& left, const std::any& right) const;

    /*!
     * \brief Throw a RuntimeError if \a operand is not a lox number.
     */
    void CheckNumberOperand(const Token& op, const std::any& operand) const;

    /*!
     * \brief Throw a RuntimeError if either \a left or \a right is not a lox number.
     */
    void CheckNumberOperands(const Token& op,
                             const std::any& left,
                             const std::any& right) const;

    /*!
     * \brief Return the string representation of \a object.
     */
    std::string Stringify(const std::any& object) const;
}; // end Interpreter
} // end lox
