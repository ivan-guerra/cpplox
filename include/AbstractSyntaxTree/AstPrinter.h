#pragma once

#include <string>
#include <memory>
#include <initializer_list>

#include "Expr.h"

namespace lox
{
/*!
 * \class AstPrinter
 * \brief The AstPrinter class outputs the AST as a Lisp like string.
 *
 * See https://craftinginterpreters.com/representing-code.html#a-not-very-pretty-printer
 * for more details.
 */
class AstPrinter :
    public ValueGetter<AstPrinter, std::shared_ptr<Expr>, std::string>,
    public Visitor
{
public:
    /*!
     * \brief Generate a binary expression output string.
     */
    void VisitBinaryExpr(Binary& expr) final
        { Return(Parenthesize(expr.op.GetLexeme(), {expr.left, expr.right})); }

    /*!
     * \brief Generate a grouping expression output string.
     */
    void VisitGroupingExpr(Grouping& expr) final
        { Return(Parenthesize("group", {expr.expression})); }

    /*!
     * \brief Generate a literal expression output string.
     */
    void VisitLiteralExpr(Literal& expr) final
        { (expr.value.empty() ? Return("nil") : Return(expr.value)); }

    /*!
     * \brief Generate a unary expression output string.
     */
    void VisitUnaryExpr(Unary& expr) final
        { Return(Parenthesize(expr.op.GetLexeme(), {expr.right})); }

    /*!
     * \brief Return the AST string represented using Lisp like syntax.
     */
    std::string Print(std::shared_ptr<Expr> expr)
        { return GetValue(expr); }

private:
    using ExprList = std::initializer_list<std::shared_ptr<Expr>>;

    /*!
     * \brief Parenthesize \a name and all expressions in \a expressions.
     *
     * The strings produced by Parenthesize() are of the form (name (expr)*)
     */
    std::string Parenthesize(const std::string& name,
                             ExprList expressions) const;
}; // end AstPrinter
} // end lox
