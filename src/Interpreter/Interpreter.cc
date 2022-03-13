#include <any>
#include <memory>
#include <string>
#include <iostream>

#include "Scanner.h"
#include "Interpreter.h"
#include "ErrorLogger.h"

namespace lox
{
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

std::any Interpreter::VisitBinaryExpr(Binary& expr)
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

std::any Interpreter::VisitUnaryExpr(Unary& expr)
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

void Interpreter::Interpret(std::shared_ptr<Expr> expression)
{
    try {
        std::any value = Evaluate(expression);
        std::cout << Stringify(value) << std::endl;
    } catch (const RuntimeError& e) {
        LOG_RUNTIME_ERROR(e.token.GetLine(), e.message);
    }
}
} // end lox