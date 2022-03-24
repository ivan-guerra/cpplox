#include <memory>
#include <unordered_map>

#include "Resolver.h"
#include "Scanner.h"
#include "ErrorLogger.h"
#include "Expr.h"
#include "Stmt.h"

namespace lox
{
void Resolver::Declare(const Token& name)
{
    if (scopes_.empty())
        return;

    std::unordered_map<std::string, bool>& scope = scopes_.top();
    if (scope.find(name.GetLexeme()) != scope.end()) {
        LOG_STATIC_ERROR(name.GetLine(),
                         "Already a variable with this name in this scope.");
    }
    scopes_.top()[name.GetLexeme()] = false;
}

void Resolver::Define(const Token& name)
{
    if (scopes_.empty())
        return;

    scopes_.top()[name.GetLexeme()] = true;
}

void Resolver::ResolveLocal(ExprPtr expr, const Token& name)
{
    ScopeStack aux = scopes_;
    for (int i = scopes_.size() - 1; i >= 0; i--) {
        if (scopes_.top().find(name.GetLexeme()) != scopes_.top().end()) {
            interpreter_->Resolve(expr, aux.size() - i - 1);
            break;
        }
        scopes_.pop();
    }
    scopes_ = aux;
}

void Resolver::ResolveFunction(std::shared_ptr<ast::Function> function,
                               FunctionType function_type)
{
    FunctionType enclosing_function = current_function_;
    current_function_ = function_type;

    BeginScope();
    for (const Token& param : function->params) {
        Declare(param);
        Define(param);
    }
    Resolve(function->body);
    EndScope();

    current_function_ = enclosing_function;
}

std::any Resolver::VisitVariableExpr(std::shared_ptr<ast::Variable> expr)
{
    std::string name_lexeme = expr->name.GetLexeme();
    if (!scopes_.empty() &&
        ((scopes_.top().find(name_lexeme) != scopes_.top().end()) &&
        !scopes_.top()[name_lexeme])) {
        LOG_STATIC_ERROR(expr->name.GetLine(),
                         "Can't read local variable in its own initializer.");
    }
    ResolveLocal(expr, expr->name);

    return nullptr;
}

std::any Resolver::VisitUnaryExpr(std::shared_ptr<ast::Unary> expr)
{
    Resolve(expr->right);
    return nullptr;
}

std::any Resolver::VisitBinaryExpr(std::shared_ptr<ast::Binary> expr)
{
    Resolve(expr->left);
    Resolve(expr->right);
    return nullptr;
}

std::any Resolver::VisitGroupingExpr(std::shared_ptr<ast::Grouping> expr)
{
    Resolve(expr->expression);
    return nullptr;
}

std::any Resolver::VisitAssignExpr(std::shared_ptr<ast::Assign> expr)
{
    Resolve(expr->value);
    ResolveLocal(expr, expr->name);
    return nullptr;
}

std::any Resolver::VisitCallExpr(std::shared_ptr<ast::Call> expr)
{
    Resolve(expr->callee);

    for (ExprPtr argument : expr->arguments)
        Resolve(argument);
    return nullptr;
}

std::any Resolver::VisitLogicalExpr(std::shared_ptr<ast::Logical> expr)
{
    Resolve(expr->left);
    Resolve(expr->right);
    return nullptr;
}

void Resolver::VisitVarStmt(std::shared_ptr<ast::Var> stmt)
{
    Declare(stmt->name);
    if (stmt->initializer)
        Resolve(stmt->initializer);
    Define(stmt->name);
}

void Resolver::VisitBlockStmt(std::shared_ptr<ast::Block> stmt)
{
    BeginScope();
    Resolve(stmt->statements);
    EndScope();
}

void Resolver::VisitIfStmt(std::shared_ptr<ast::If> stmt)
{
    Resolve(stmt->condition);
    Resolve(stmt->then_branch);
    if (stmt->else_branch)
        Resolve(stmt->else_branch);
}

void Resolver::VisitFunctionStmt(std::shared_ptr<ast::Function> stmt)
{
    Declare(stmt->name);
    Define(stmt->name);

    ResolveFunction(stmt, FunctionType::kFunction);
}

void Resolver::VisitWhileStmt(std::shared_ptr<ast::While> stmt)
{
    Resolve(stmt->condition);
    Resolve(stmt->body);
}

void Resolver::VisitReturnStmt(std::shared_ptr<ast::Return> stmt)
{
    if (current_function_ == FunctionType::kNone) {
        LOG_STATIC_ERROR(stmt->keyword.GetLine(),
                         "Can't return from top-level code.");
    }

    if (stmt->value)
        Resolve(stmt->value);
}

void Resolver::Resolve(
    const std::vector<std::shared_ptr<ast::Stmt>>& statements)
{
    for (StmtPtr statement : statements)
        Resolve(statement);
}
} // end lox
