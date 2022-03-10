/* Autogenerated using generate_ast.py, DO NOT EDIT. */
#pragma once

#include <string>
#include <memory>

#include "Scanner.h"

namespace lox
{
template <typename VisitorImpl, typename VisitablePtr, typename ResultType>
class ValueGetter
{
public:
    static ResultType GetValue(VisitablePtr n)
    {
        VisitorImpl visitor;
        n->Accept(visitor);
        return visitor.value_;
    }

    void Return(ResultType value)
    {
        value_ = value;
    }
private:
    ResultType value_;
}; // end ValueGetter

class Binary;
class Grouping;
class Literal;
class Unary;

class Visitor
{
public:
    virtual void VisitBinaryExpr(Binary& expr) = 0;
    virtual void VisitGroupingExpr(Grouping& expr) = 0;
    virtual void VisitLiteralExpr(Literal& expr) = 0;
    virtual void VisitUnaryExpr(Unary& expr) = 0;
}; // end Visitor

class Expr
{
public:
    virtual ~Expr() = default;
    virtual void Accept(Visitor& visitor) = 0;
}; // end Expr

class Binary : public Expr
{
public:
    Binary() = default;
    ~Binary() = default;
    Binary(Binary&) = default;
    Binary& operator=(Binary&) = default;
    Binary(Binary&&) = default;
    Binary& operator=(Binary&&) = default;

    Binary(std::shared_ptr<Expr> left_, Token op_, std::shared_ptr<Expr> right_) : 
        left(left_),
        op(op_),
        right(right_)
    {

    }

    void Accept(Visitor& visitor) final
    {
        visitor.VisitBinaryExpr(*this);
    }

    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
}; // end Binary

class Grouping : public Expr
{
public:
    Grouping() = default;
    ~Grouping() = default;
    Grouping(Grouping&) = default;
    Grouping& operator=(Grouping&) = default;
    Grouping(Grouping&&) = default;
    Grouping& operator=(Grouping&&) = default;

    Grouping(std::shared_ptr<Expr> expression_) : 
        expression(expression_)
    {

    }

    void Accept(Visitor& visitor) final
    {
        visitor.VisitGroupingExpr(*this);
    }

    std::shared_ptr<Expr> expression;
}; // end Grouping

class Literal : public Expr
{
public:
    Literal() = default;
    ~Literal() = default;
    Literal(Literal&) = default;
    Literal& operator=(Literal&) = default;
    Literal(Literal&&) = default;
    Literal& operator=(Literal&&) = default;

    Literal(std::string value_) : 
        value(value_)
    {

    }

    void Accept(Visitor& visitor) final
    {
        visitor.VisitLiteralExpr(*this);
    }

    std::string value;
}; // end Literal

class Unary : public Expr
{
public:
    Unary() = default;
    ~Unary() = default;
    Unary(Unary&) = default;
    Unary& operator=(Unary&) = default;
    Unary(Unary&&) = default;
    Unary& operator=(Unary&&) = default;

    Unary(Token op_, std::shared_ptr<Expr> right_) : 
        op(op_),
        right(right_)
    {

    }

    void Accept(Visitor& visitor) final
    {
        visitor.VisitUnaryExpr(*this);
    }

    Token op;
    std::shared_ptr<Expr> right;
}; // end Unary
} // end lox