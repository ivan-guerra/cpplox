#include <any>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <unordered_map>

#include "Expr.h"
#include "Stmt.h"
#include "Scanner.h"
#include "Interpreter.h"
#include "ErrorLogger.h"
#include "RuntimeError.h"

namespace lox
{
std::any Interpreter::Clock::Call([[maybe_unused]]Interpreter& interpreter,
    [[maybe_unused]]std::vector<std::any>& arguments)
{
    auto curr_time = std::chrono::system_clock::now();
    return std::chrono::duration<double>(curr_time.time_since_epoch()).count();
}

std::any Interpreter::LoxFunction::Call(Interpreter& interpreter,
                                        std::vector<std::any>& arguments)
{
    EnvPtr env = std::make_shared<Environment>(*interpreter.environment_);
    env->SetEnclosingEnv(closure);

    for (std::size_t i = 0; i < declaration->params.size(); ++i) {
        env->Define(declaration->params[i].GetLexeme(),
                arguments[i]);
    }
    try {
        interpreter.ExecuteBlock(declaration->body, env);
    } catch (const Return& e) {
        return (is_initializer) ? closure->GetAt(0, "this") : e.value;
    }

    if (is_initializer)
        return closure->GetAt(0, "this");

    return nullptr;
}

std::shared_ptr<Interpreter::LoxCallable> Interpreter::LoxFunction::Bind(
    std::shared_ptr<LoxInstance> instance)
{
    std::shared_ptr<Environment> env = std::make_shared<Environment>();
    env->SetEnclosingEnv(closure);
    env->Define("this", instance);

    std::shared_ptr<LoxCallable> binding =
        std::make_shared<LoxFunction>(declaration, env, is_initializer);

    return binding;
}

std::any Interpreter::LoxClass::Call(Interpreter& interpreter,
                                     std::vector<std::any>& arguments)
{
    std::shared_ptr<LoxInstance> instance =
        std::make_shared<LoxInstance>(shared_from_this());

    std::shared_ptr<LoxCallable> initializer = FindMethod("init");
    if (initializer) {
        std::static_pointer_cast<LoxFunction>(initializer)->Bind(instance)->Call(interpreter, arguments);
    }
    return instance;
}

std::size_t Interpreter::LoxClass::Arity() const
{
    std::shared_ptr<LoxCallable> initializer = FindMethod("init");
    if (!initializer)
        return 0;

    return initializer->Arity();
}

std::shared_ptr<Interpreter::LoxCallable> Interpreter::LoxClass::FindMethod(
    const std::string& name) const
{
    if (methods.find(name) != methods.end())
        return methods.at(name);

    return nullptr;
}

std::any Interpreter::LoxInstance::Get(const Token& name)
{
    if (fields.find(name.GetLexeme()) != fields.end())
        return fields.at(name.GetLexeme());

    std::shared_ptr<LoxFunction> method =
        std::static_pointer_cast<LoxFunction>(
            klass->FindMethod(name.GetLexeme()));
    if (method)
        return method->Bind(shared_from_this());

    std::stringstream err_message;
    err_message << "Undefined property '" << name.GetLexeme() << "'.";
    throw RuntimeError(name, err_message.str());
}

void Interpreter::ExecuteBlock(
    const std::vector<std::shared_ptr<ast::Stmt>>& statements,
    std::shared_ptr<Environment> env)
{
    EnvPtr previous = this->environment_;
    try {
        this->environment_ = env;
        for (const auto& stmt : statements)
            Execute(stmt);
    } catch (const std::runtime_error& e) {
        this->environment_ = previous;

        /* Important that we let the exception propagate up so that either
           error handling code or a LoxCallable::Call() method sees the
           exception and triggers the appropriate behavior. */
        throw;
    }

    /* We don't have finally in C++ so this assignment must live outside
       the catch statement. */
    this->environment_ = previous;
}

bool Interpreter::IsTruth(const std::any& object) const
{
    if (typeid(nullptr) == object.type())
        /* Return false if object is nil. */
        return false;

    if (typeid(bool) == object.type())
        return std::any_cast<bool>(object);

    return true;
}

bool Interpreter::IsEqual(const std::any& left, const std::any& right) const
{
    /* We only check for equality for string, number, boolean, and nil types.
       An equality check of any other type or mismatched types has an automatic
       return value of false. */
    if (left.type() == right.type()) {
        if (typeid(std::string) == left.type())
            return (std::any_cast<std::string>(left) == std::any_cast<std::string>(right));
        if (typeid(double) == left.type())
            return (std::any_cast<double>(left) == std::any_cast<double>(right));
        if (typeid(bool) == left.type())
            return (std::any_cast<bool>(left) == std::any_cast<bool>(right));
        if (typeid(nullptr) == left.type())
            return true;
    }
    return false;
}

void Interpreter::CheckNumberOperand(const Token& op,
                                     const std::any& operand) const
{
    if (typeid(double) == operand.type())
        return;

    throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::CheckNumberOperands(const Token& op,
                                      const std::any& left,
                                      const std::any& right) const
{
    if ((typeid(double) == left.type()) && (typeid(double) == right.type()))
        return;

    throw RuntimeError(op, "Operands must be numbers.");
}

std::string Interpreter::Stringify(const std::any& object)
{
    if (typeid(nullptr) == object.type())
        return "nil";

    if (typeid(bool) == object.type())
        return (std::any_cast<bool>(object) ? "true" : "false");

    if (typeid(double) == object.type())
        return std::to_string(std::any_cast<double>(object));

    using ClockPtr = std::shared_ptr<lox::Interpreter::Clock>;
    if (typeid(ClockPtr) == object.type()) {
        ClockPtr clock = std::any_cast<ClockPtr>(object);
        std::vector<std::any> dummy;
        double time = std::any_cast<double>(clock->Call(*this, dummy));
        return std::to_string(time);
    }

    return std::any_cast<std::string>(object);
}

std::any Interpreter::LookupVariable(const Token& name,
                                     std::shared_ptr<ast::Expr> expr)
{
    if (locals_.find(expr) != locals_.end()) {
        int distance = locals_[expr];
        return environment_->GetAt(distance, name.GetLexeme());
    }

    return globals_->Get(name);
}

void Interpreter::VisitPrintStmt(std::shared_ptr<ast::Print> stmt)
{
    std::any value = Evaluate(stmt->expression);
    std::cout << Stringify(value) << std::endl;
}

void Interpreter::VisitVarStmt(std::shared_ptr<ast::Var> stmt)
{
    std::any value(nullptr);
    if (stmt->initializer)
        value = Evaluate(stmt->initializer);

    environment_->Define(stmt->name.GetLexeme(), value);
}

void Interpreter::VisitBlockStmt(std::shared_ptr<ast::Block> stmt)
{
    EnvPtr env = std::make_shared<Environment>();
    env->SetEnclosingEnv(this->environment_);

    ExecuteBlock(stmt->statements, env);
}

void Interpreter::VisitIfStmt(std::shared_ptr<ast::If> stmt)
{
    if (IsTruth(Evaluate(stmt->condition)))
        Execute(stmt->then_branch);
    else
        Execute(stmt->else_branch);
}

void Interpreter::VisitWhileStmt(std::shared_ptr<ast::While> stmt)
{
    while (IsTruth(Evaluate(stmt->condition)))
        Execute(stmt->body);
}

void Interpreter::VisitFunctionStmt(std::shared_ptr<ast::Function> stmt)
{
    std::shared_ptr<LoxCallable> function =
        std::make_shared<LoxFunction>(stmt,
            std::make_shared<Environment>(*environment_), false);

    environment_->Define(stmt->name.GetLexeme(), function);
}

void Interpreter::VisitClassStmt(std::shared_ptr<ast::Class> stmt)
{
    environment_->Define(stmt->name.GetLexeme(), nullptr);

    std::unordered_map<std::string, std::shared_ptr<LoxCallable>> methods;
    for (auto& method : stmt->methods) {
        std::shared_ptr<LoxCallable> function =
            std::make_shared<LoxFunction>(method, environment_,
                method->name.GetLexeme() == std::string("init"));
            methods[method->name.GetLexeme()] = function;
    }

    std::shared_ptr<LoxCallable> klass =
        std::make_shared<LoxClass>(stmt->name.GetLexeme(), methods);
    environment_->Assign(stmt->name, klass);
}

void Interpreter::VisitReturnStmt(std::shared_ptr<ast::Return> stmt)
{
    std::any value = nullptr;
    if (stmt->value)
        value = Evaluate(stmt->value);

    throw Return(value);
}

std::any Interpreter::VisitBinaryExpr(std::shared_ptr<ast::Binary> expr)
{
    std::any left  = Evaluate(expr->left);
    std::any right = Evaluate(expr->right);

    Token::TokenType type = expr->op.GetType();
    if (Token::TokenType::kPlus == type) {
        if ((typeid(double) == left.type()) &&
                (typeid(double) == right.type()))
            return (std::any_cast<double>(left) + std::any_cast<double>(right));
        if ((typeid(std::string) == left.type()) &&
                (typeid(std::string) == right.type()))
            return (std::any_cast<std::string>(left) + std::any_cast<std::string>(right));

        throw RuntimeError(expr->op,
                           "Operands must be two numbers or two strings.");
    } else if (Token::TokenType::kMinus == type) {
        CheckNumberOperands(expr->op, left, right);
        return (std::any_cast<double>(left) - std::any_cast<double>(right));
    } else if (Token::TokenType::kSlash == type) {
        CheckNumberOperands(expr->op, left, right);
        return (std::any_cast<double>(left) / std::any_cast<double>(right));
    } else if (Token::TokenType::kStar == type) {
        CheckNumberOperands(expr->op, left, right);
        return (std::any_cast<double>(left) * std::any_cast<double>(right));
    } else if (Token::TokenType::kGreater == type) {
        CheckNumberOperands(expr->op, left, right);
        return (std::any_cast<double>(left) > std::any_cast<double>(right));
    } else if (Token::TokenType::kGreaterEqual == type) {
        CheckNumberOperands(expr->op, left, right);
        return (std::any_cast<double>(left) >= std::any_cast<double>(right));
    } else if (Token::TokenType::kLess == type) {
        CheckNumberOperands(expr->op, left, right);
        return (std::any_cast<double>(left) < std::any_cast<double>(right));
    } else if (Token::TokenType::kLessEqual == type) {
        CheckNumberOperands(expr->op, left, right);
        return (std::any_cast<double>(left) <= std::any_cast<double>(right));
    } else if (Token::TokenType::kBangEqual == type) {
        return !IsEqual(left, right);
    } else if (Token::TokenType::kEqualEqual == type) {
        return IsEqual(left, right);
    }

    /* Unreachable */
    return nullptr;
}

std::any Interpreter::VisitUnaryExpr(std::shared_ptr<ast::Unary> expr)
{
    std::any right = Evaluate(expr->right);

    Token::TokenType type = expr->op.GetType();
    if (Token::TokenType::kMinus == type) {
        CheckNumberOperand(expr->op, right);
        return -(std::any_cast<double>(right));
    } else if (Token::TokenType::kBang == type) {
        return !IsTruth(right);
    }

    /* Unreachable */
    return nullptr;
}

std::any Interpreter::VisitAssignExpr(std::shared_ptr<ast::Assign> expr)
{
    std::any value = Evaluate(expr->value);

    if (locals_.find(expr) != locals_.end()) {
        int distance = locals_[expr];
        environment_->AssignAt(distance, expr->name, value);
    } else {
        globals_->Assign(expr->name, value);
    }

    return value;
}

std::any Interpreter::VisitLogicalExpr(std::shared_ptr<ast::Logical> expr)
{
    std::any left = Evaluate(expr->left);

    if (expr->op.GetType() == Token::TokenType::kOr) {
        if (IsTruth(left))
            return left;
    } else {
        if (!IsTruth(left))
            return left;
    }
    return Evaluate(expr->right);
}

std::any Interpreter::VisitCallExpr(std::shared_ptr<ast::Call> expr)
{
    std::any callee = Evaluate(expr->callee);

    std::vector<std::any> arguments;
    for (const auto& argument : expr->arguments)
        arguments.push_back(Evaluate(argument));

    if (callee.type() != typeid(std::shared_ptr<LoxCallable>))
        throw RuntimeError(expr->paren, "Can only call functions and classes.");

    std::shared_ptr<LoxCallable> function =
        std::any_cast<std::shared_ptr<LoxCallable>>(callee);

    if (arguments.size() != function->Arity()) {
        std::stringstream err_message;
        err_message << "Expected " << function->Arity()
                    << " arguments but got " << arguments.size() << ".";
        throw RuntimeError(expr->paren, err_message.str());
    }

    return function->Call(*this, arguments);
}

std::any Interpreter::VisitGetExpr(std::shared_ptr<ast::Get> expr)
{
    std::any object = Evaluate(expr->object);

    using LoxInstancePtr = std::shared_ptr<LoxInstance>;
    if (object.type() == typeid(LoxInstancePtr))
        return std::any_cast<LoxInstancePtr>(object)->Get(expr->name);

    throw RuntimeError(expr->name, "Only instances have properties.");
}

std::any Interpreter::VisitSetExpr(std::shared_ptr<ast::Set> expr)
{
    std::any object = Evaluate(expr->object);

    using LoxInstancePtr = std::shared_ptr<LoxInstance>;
    if (object.type() != typeid(LoxInstancePtr))
        throw RuntimeError(expr->name, "Only instances have fields.");

    std::any value = Evaluate(expr->value);
    std::any_cast<LoxInstancePtr>(object)->Set(expr->name, value);
    return value;
}

Interpreter::Interpreter() :
    globals_(std::make_shared<Environment>()),
    environment_(globals_)
{
    globals_->Define("clock", std::make_shared<Clock>());
}

void Interpreter::Interpret(
    const std::vector<std::shared_ptr<ast::Stmt>>& statements)
{
    try {
        for (const auto& stmt : statements) {
            if (stmt)
                Execute(stmt);
        }
    } catch (const RuntimeError& e) {
        LOG_RUNTIME_ERROR(e.token.GetLine(), e.message);
    }
}
} // end lox
