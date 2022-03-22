#pragma once

#include <any>
#include <vector>
#include <memory>
#include <string>
#include <stack>
#include <unordered_map>

#include "Expr.h"
#include "Stmt.h"
#include "Scanner.h"
#include "Interpreter.h"

namespace lox
{
/*!
 * \class Resolver
 * \brief The Resolver class resolves variable environment depth.
 *
 * When the Interpreter goes to resolve a variable reference, it performs a
 * linear search through a linked list of Environments until it finds the
 * variable. The Resolver tells the interpreter for a given variable reference
 * at some node in the AST how far back the Interpreter needs to search in its
 * list of Environments in order to resolve the variable.
 */
class Resolver :
    public ast::ExprVisitor,
    public ast::StmtVisitor
{
public:
    Resolver() = delete;

    /*!
     * \brief Construct the Resolver.
     *
     * It is important that the Resolver has an instance of Interpreter. As
     * the Resolver walks the AST, it will hand off variable resolution info
     * to the Interpreter.
     *
     * \param interpreter Pointer to an Interpreter object.
     */
    Resolver(std::shared_ptr<lox::Interpreter> interpreter) :
        interpreter_(interpreter),
        current_function_(FunctionType::kNone)
        { }

    ~Resolver() = default;

    Resolver(const Resolver&) = default;
    Resolver& operator=(const Resolver&) = default;

    Resolver(Resolver&&) = default;
    Resolver& operator=(Resolver&&) = default;

    std::any VisitBinaryExpr(std::shared_ptr<ast::Binary> expr) final;

    std::any VisitGroupingExpr(std::shared_ptr<ast::Grouping> expr) final;

    std::any VisitLiteralExpr(
        [[maybe_unused]]std::shared_ptr<ast::Literal> expr) final
        { return nullptr; }

    std::any VisitUnaryExpr(std::shared_ptr<ast::Unary> expr) final;

    std::any VisitVariableExpr(std::shared_ptr<ast::Variable> expr) final;

    std::any VisitAssignExpr(std::shared_ptr<ast::Assign> expr) final;

    std::any VisitLogicalExpr(std::shared_ptr<ast::Logical> expr) final;

    std::any VisitCallExpr(std::shared_ptr<ast::Call> expr) final;

    void VisitExpressionStmt(std::shared_ptr<ast::Expression> stmt) final
        { Resolve(stmt->expression); }

    void VisitPrintStmt(std::shared_ptr<ast::Print> stmt) final
        { Resolve(stmt->expression); }

    void VisitVarStmt(std::shared_ptr<ast::Var> stmt) final;

    void VisitBlockStmt(std::shared_ptr<ast::Block> stmt) final;

    void VisitIfStmt(std::shared_ptr<ast::If> stmt) final;

    void VisitWhileStmt(std::shared_ptr<ast::While> stmt) final;

    void VisitFunctionStmt(std::shared_ptr<ast::Function> stmt) final;

    void VisitReturnStmt(std::shared_ptr<ast::Return> stmt) final;

    void Resolve(const std::vector<std::shared_ptr<ast::Stmt>>& statements);

private:
    using ExprPtr     = std::shared_ptr<ast::Expr>;
    using StmtPtr     = std::shared_ptr<ast::Stmt>;
    using StmtPtrList = std::vector<StmtPtr>;
    using ScopeStack  = std::stack<std::unordered_map<std::string, bool>>;

    /*!
     * \enum FunctionType
     * \brief The FunctionType enum is used to help us detect return errors.
     */
    enum class FunctionType
    {
        kNone,    /*!< Not a function. */
        kFunction /*!< Standard Lox function. */
    }; // end FunctionType

    /*!
     * \brief Add a scope to #scopes_.
     */
    void BeginScope()
        { scopes_.push(std::unordered_map<std::string, bool>()); }

    /*!
     * \brief Remove a scope from #scopes_.
     */
    void EndScope()
        { scopes_.pop(); }

    /*!
     * \brief Declare a variable within the current scope.
     */
    void Declare(const Token& name);

    /*!
     * \brief Define a variable within the current scope.
     */
    void Define(const Token& name);

    /*!
     * \brief Resolve a local variable.
     *
     * ResolveLocal() walks back through #scopes_ searching for \a name. A
     * side effect of this method is registration of the depth of the variable
     * in Interpreter's Environment chain.
     */
    void ResolveLocal(std::shared_ptr<ast::Expr> expr, const Token& name);

    /*!
     * \brief Resolve the parameters and body of a function.
     */
    void ResolveFunction(std::shared_ptr<ast::Function> function,
                         FunctionType function_type);

    void Resolve(ExprPtr expr)
        { expr->Accept(*this); }

    void Resolve(StmtPtr stmt)
        { stmt->Accept(*this); }

    std::shared_ptr<lox::Interpreter> interpreter_;      /*!< Interpret which this Resolver is registering its variable resolutions. */
    ScopeStack                        scopes_;           /*!< Stack of active scopes. */
    FunctionType                      current_function_; /*!< Flag used to track the function type if any. */
}; // end Resolver
} // end lox
