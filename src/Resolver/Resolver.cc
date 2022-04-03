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

std::any Resolver::VisitGetExpr(std::shared_ptr<ast::Get> expr)
{
    Resolve(expr->object);
    return nullptr;
}

std::any Resolver::VisitSetExpr(std::shared_ptr<ast::Set> expr)
{
    Resolve(expr->value);
    Resolve(expr->object);
    return nullptr;
}

std::any Resolver::VisitThisExpr(std::shared_ptr<ast::This> expr)
{
    if (ClassType::kNone == current_class_) {
        LOG_STATIC_ERROR(expr->keyword.GetLine(),
                         "Can't use 'this' outside of a class.");
        return nullptr;
    }

    ResolveLocal(expr, expr->keyword);

    return nullptr;
}

std::any Resolver::VisitSuperExpr(std::shared_ptr<ast::Super> expr)
{
    if (current_class_ == ClassType::kNone) {
        LOG_STATIC_ERROR(expr->keyword.GetLine(),
                         "Can't use 'super' outside of a class.");
    } else if (current_class_ != ClassType::kSubclass) {
        LOG_STATIC_ERROR(expr->keyword.GetLine(),
                         "Can't use 'super' in a class with no superclass.");
    }

    ResolveLocal(expr, expr->keyword);
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

void Resolver::VisitClassStmt(std::shared_ptr<ast::Class> stmt)
{
    ClassType enclosing_class = current_class_;
    current_class_ = ClassType::kClass;

    Declare(stmt->name);
    Define(stmt->name);

    if (stmt->superclass &&
        (stmt->superclass->name.GetLexeme() == stmt->name.GetLexeme())) {
        LOG_STATIC_ERROR(stmt->superclass->name.GetLine(),
                         "A class can't inherit from itself.");
    }

    if (stmt->superclass) {
        current_class_ = ClassType::kSubclass;
        Resolve(stmt->superclass);
    }

    if (stmt->superclass) {
        BeginScope();
        scopes_.top()["super"] = true;
    }

    BeginScope();
    scopes_.top()["this"] = true;

    for (auto& method : stmt->methods) {
        FunctionType declaration = FunctionType::kMethod;
        if (method->name.GetLexeme() == std::string("init"))
            declaration = FunctionType::kInitializer;

        ResolveFunction(method, declaration);
    }
    EndScope();

    if (stmt->superclass)
        EndScope();

    current_class_ = enclosing_class;
}

void Resolver::VisitReturnStmt(std::shared_ptr<ast::Return> stmt)
{
    if (current_function_ == FunctionType::kNone) {
        LOG_STATIC_ERROR(stmt->keyword.GetLine(),
                         "Can't return from top-level code.");
    }

    if (stmt->value) {
        if (current_function_ == FunctionType::kInitializer) {
            LOG_STATIC_ERROR(stmt->keyword.GetLine(),
                             "Can't return a value from an initializer.");
        }
        Resolve(stmt->value);
    }
}

void Resolver::Resolve(
    const std::vector<std::shared_ptr<ast::Stmt>>& statements)
{
    for (StmtPtr statement : statements)
        Resolve(statement);
}
} // end lox
