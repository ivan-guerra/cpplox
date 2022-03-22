#pragma once

#include <any>
#include <string>
#include <memory>
#include <initializer_list>

#include "Expr.h"

namespace lox
{
/*!
 * \class AstPrinter
 * \brief The AstPrinter class outputs an Expr AST as a Lisp like string.
 *
 * See https://craftinginterpreters.com/representing-code.html#a-not-very-pretty-printer
 * for more details.
 */
class AstPrinter :
    public ast::ExprVisitor
{
public:
    /*!
     * \brief Generate a binary expression output string.
     */
    std::any VisitBinaryExpr(std::shared_ptr<ast::Binary> expr) final
        { return Parenthesize(expr->op.GetLexeme(), {expr->left, expr->right}); }

    /*!
     * \brief Generate a grouping expression output string.
     */
    std::any VisitGroupingExpr(std::shared_ptr<ast::Grouping> expr) final
        { return Parenthesize("group", {expr->expression}); }

    /*!
     * \brief Generate a literal expression output string.
     */
    std::any VisitLiteralExpr(std::shared_ptr<ast::Literal> expr) final;

    /*!
     * \brief Generate a unary expression output string.
     */
    std::any VisitUnaryExpr(std::shared_ptr<ast::Unary> expr) final
        { return Parenthesize(expr->op.GetLexeme(), {expr->right}); }

    /*!
     * \brief Return the AST string represented using Lisp like syntax.
     */
    std::string Print(std::shared_ptr<ast::Expr> expr)
        { return std::any_cast<std::string>(expr->Accept(*this)); }

private:
    using ExprList = std::initializer_list<std::shared_ptr<ast::Expr>>;

    /*!
     * \brief Parenthesize \a name and all expressions in \a expressions.
     *
     * The strings produced by Parenthesize() are of the form (name (expr)*)
     */
    std::string Parenthesize(const std::string& name,
                             ExprList expressions);
}; // end AstPrinter
} // end lox
