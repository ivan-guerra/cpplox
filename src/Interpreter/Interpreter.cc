#include <any>
#include <memory>
#include <string>
#include <iostream>

#include "Scanner.h"
#include "Interpreter.h"
#include "ErrorLogger.h"
#include "RuntimeError.h"

namespace lox
{
void Interpreter::ExecuteBlock(
    const std::vector<std::shared_ptr<ast::Stmt>>& statements,
    std::shared_ptr<Environment> env)
{
    std::shared_ptr<Environment> previous = this->environment_;
    try {
        this->environment_ = env;
        for (const auto& stmt : statements)
            Execute(stmt);
    } catch (const std::exception& e) {
        /* If something goes wrong when processing statements, always revert
           the environment. */
        this->environment_ = previous;
    }

    /* Revert the environment after processing the block. */
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

std::string Interpreter::Stringify(const std::any& object) const
{
    if (typeid(nullptr) == object.type())
        return "nil";

    if (typeid(bool) == object.type())
        return (std::any_cast<bool>(object) ? "true" : "false");

    if (typeid(double) == object.type()) {
        std::string double_str = std::to_string(std::any_cast<double>(object));
        double_str.erase(double_str.find_first_of('.'), std::string::npos);
        return double_str;
    }
    return std::any_cast<std::string>(object);
}

void Interpreter::VisitPrintStmt(ast::Print& stmt)
{
    std::any value = Evaluate(stmt.expression);
    std::cout << Stringify(value) << std::endl;
}

void Interpreter::VisitVarStmt(ast::Var& stmt)
{
    std::any value(nullptr);
    if (stmt.initializer)
        value = Evaluate(stmt.initializer);

    environment_->Define(stmt.name.GetLexeme(), value);
}

void Interpreter::VisitBlockStmt(ast::Block& stmt)
{
    ExecuteBlock(stmt.statements,
                 std::make_shared<Environment>(this->environment_));
}

void Interpreter::VisitIfStmt(ast::If& stmt)
{
    if (IsTruth(Evaluate(stmt.condition)))
        Execute(stmt.then_branch);
    else
        Execute(stmt.else_branch);
}

void Interpreter::VisitWhileStmt(ast::While& stmt)
{
    while (IsTruth(Evaluate(stmt.condition)))
        Execute(stmt.body);
}

std::any Interpreter::VisitBinaryExpr(ast::Binary& expr)
{
    std::any left  = Evaluate(expr.left);
    std::any right = Evaluate(expr.right);

    Token::TokenType type = expr.op.GetType();
    if (Token::TokenType::kPlus == type) {
        if ((typeid(double) == left.type()) &&
                (typeid(double) == right.type()))
            return (std::any_cast<double>(left) + std::any_cast<double>(right));
        if ((typeid(std::string) == left.type()) &&
                (typeid(std::string) == right.type()))
            return (std::any_cast<std::string>(left) + std::any_cast<std::string>(right));

        throw RuntimeError(expr.op,
                           "Operands must be two numbers or two strings.");
    } else if (Token::TokenType::kMinus == type) {
        CheckNumberOperands(expr.op, left, right);
        return (std::any_cast<double>(left) - std::any_cast<double>(right));
    } else if (Token::TokenType::kSlash == type) {
        CheckNumberOperands(expr.op, left, right);
        return (std::any_cast<double>(left) / std::any_cast<double>(right));
    } else if (Token::TokenType::kStar == type) {
        CheckNumberOperands(expr.op, left, right);
        return (std::any_cast<double>(left) * std::any_cast<double>(right));
    } else if (Token::TokenType::kGreater == type) {
        CheckNumberOperands(expr.op, left, right);
        return (std::any_cast<double>(left) > std::any_cast<double>(right));
    } else if (Token::TokenType::kGreaterEqual == type) {
        CheckNumberOperands(expr.op, left, right);
        return (std::any_cast<double>(left) >= std::any_cast<double>(right));
    } else if (Token::TokenType::kLess == type) {
        CheckNumberOperands(expr.op, left, right);
        return (std::any_cast<double>(left) < std::any_cast<double>(right));
    } else if (Token::TokenType::kLessEqual == type) {
        CheckNumberOperands(expr.op, left, right);
        return (std::any_cast<double>(left) <= std::any_cast<double>(right));
    } else if (Token::TokenType::kBangEqual == type) {
        return !IsEqual(left, right);
    } else if (Token::TokenType::kEqualEqual == type) {
        return IsEqual(left, right);
    }

    /* Unreachable */
    return nullptr;
}

std::any Interpreter::VisitUnaryExpr(ast::Unary& expr)
{
    std::any right = Evaluate(expr.right);

    Token::TokenType type = expr.op.GetType();
    if (Token::TokenType::kMinus == type) {
        CheckNumberOperand(expr.op, right);
        return -(std::any_cast<double>(right));
    } else if (Token::TokenType::kBang == type) {
        return !IsTruth(right);
    }

    /* Unreachable */
    return nullptr;
}

std::any Interpreter::VisitAssignExpr(ast::Assign& expr)
{
    std::any value = Evaluate(expr.value);
    environment_->Assign(expr.name, value);

    return value;
}

std::any Interpreter::VisitLogicalExpr(ast::Logical& expr)
{
    std::any left = Evaluate(expr.left);

    if (expr.op.GetType() == Token::TokenType::kOr) {
        if (IsTruth(left))
            return left;
    } else {
        if (!IsTruth(left))
            return left;
    }
    return Evaluate(expr.right);
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
