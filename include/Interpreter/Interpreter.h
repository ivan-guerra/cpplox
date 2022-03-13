#pragma once

#include <any>
#include <memory>
#include <exception>
#include <vector>

#include "Expr.h"
#include "Stmt.h"
#include "Scanner.h"
#include "Environment.h"

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
class Interpreter :
    public ast::ExprVisitor,
    public ast::StmtVisitor
{
public:
    Interpreter() : environment_(std::make_shared<Environment>()) { }
    ~Interpreter() = default;

    /* Default copy construction and assignment is valid. */
    Interpreter(const Interpreter&) = default;
    Interpreter& operator=(const Interpreter&) = default;

    /* Default move construction and assignment is valid. */
    Interpreter(Interpreter&&) = default;
    Interpreter& operator=(Interpreter&&) = default;

    void VisitExpressionStmt(ast::Expression& stmt) final
        { Evaluate(stmt.expression); }

    void VisitPrintStmt(ast::Print& stmt) final;

    void VisitVarStmt(ast::Var& stmt) final;

    void VisitBlockStmt(ast::Block& stmt) final;

    /*!
     * \brief Evaluate a binary expression.
     */
    std::any VisitBinaryExpr(ast::Binary& expr) final;

    /*!
     * \brief Evaluate a parenthesized expression.
     */
    std::any VisitGroupingExpr(ast::Grouping& expr) final
        { return Evaluate(expr.expression); }

    /*!
     * \brief Evaluate a literal expression.
     */
    std::any VisitLiteralExpr(ast::Literal& expr) final
        { return expr.value; }

    /*!
     * \brief Evaluate a unary expression.
     */
    std::any VisitUnaryExpr(ast::Unary& expr) final;

    std::any VisitVariableExpr(ast::Variable& expr) final
        { return environment_->Get(expr.name); }

    std::any VisitAssignExpr(ast::Assign& expr) final;

    /*!
     * \brief Evaluate \a expression.
     *
     * Interpret() will compute the value represented by \a expression. The
     * computed value is output to stdout.
     */
    void Interpret(const std::vector<std::shared_ptr<ast::Stmt>>& statements);
private:
    void Execute(std::shared_ptr<ast::Stmt> stmt)
        { stmt->Accept(*this); }

    void ExecuteBlock(const std::vector<std::shared_ptr<ast::Stmt>>& statements,
                      std::shared_ptr<Environment> env);

    /*!
     * \brief Return the result of the evaluating \a expr.
     */
    std::any Evaluate(std::shared_ptr<ast::Expr> expr)
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

    std::shared_ptr<Environment> environment_;
}; // end Interpreter
} // end lox
