/* Autogenerated using generate_ast.py, DO NOT EDIT. */
#pragma once

#include <any>
#include <memory>
#include "Scanner.h"

namespace lox
{
namespace ast
{
class Binary;
class Grouping;
class Literal;
class Unary;
class Variable;
class Assign;
class Logical;
class Call;

class ExprVisitor
{
public:
    virtual std::any VisitBinaryExpr(std::shared_ptr<Binary> expr) = 0;
    virtual std::any VisitGroupingExpr(std::shared_ptr<Grouping> expr) = 0;
    virtual std::any VisitLiteralExpr(std::shared_ptr<Literal> expr) = 0;
    virtual std::any VisitUnaryExpr(std::shared_ptr<Unary> expr) = 0;
    virtual std::any VisitVariableExpr(std::shared_ptr<Variable> expr) = 0;
    virtual std::any VisitAssignExpr(std::shared_ptr<Assign> expr) = 0;
    virtual std::any VisitLogicalExpr(std::shared_ptr<Logical> expr) = 0;
    virtual std::any VisitCallExpr(std::shared_ptr<Call> expr) = 0;
}; // end Visitor

class Expr
{
public:
    virtual ~Expr() = default;
    virtual std::any Accept(ExprVisitor& visitor) = 0;
}; // end Expr

class Binary : 
    public Expr,
    public std::enable_shared_from_this<Binary>
{
public:
    Binary() = default;
    ~Binary() = default;
    Binary(const Binary&) = default;
    Binary& operator=(const Binary&) = default;
    Binary(Binary&&) = default;
    Binary& operator=(Binary&&) = default;

    Binary(std::shared_ptr<Expr> left_, Token op_, std::shared_ptr<Expr> right_) : 
        left(left_),
        op(op_),
        right(right_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitBinaryExpr(shared_from_this());
    }

    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
}; // end Binary

class Grouping : 
    public Expr,
    public std::enable_shared_from_this<Grouping>
{
public:
    Grouping() = default;
    ~Grouping() = default;
    Grouping(const Grouping&) = default;
    Grouping& operator=(const Grouping&) = default;
    Grouping(Grouping&&) = default;
    Grouping& operator=(Grouping&&) = default;

    Grouping(std::shared_ptr<Expr> expression_) : 
        expression(expression_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitGroupingExpr(shared_from_this());
    }

    std::shared_ptr<Expr> expression;
}; // end Grouping

class Literal : 
    public Expr,
    public std::enable_shared_from_this<Literal>
{
public:
    Literal() = default;
    ~Literal() = default;
    Literal(const Literal&) = default;
    Literal& operator=(const Literal&) = default;
    Literal(Literal&&) = default;
    Literal& operator=(Literal&&) = default;

    Literal(std::any value_) : 
        value(value_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitLiteralExpr(shared_from_this());
    }

    std::any value;
}; // end Literal

class Unary : 
    public Expr,
    public std::enable_shared_from_this<Unary>
{
public:
    Unary() = default;
    ~Unary() = default;
    Unary(const Unary&) = default;
    Unary& operator=(const Unary&) = default;
    Unary(Unary&&) = default;
    Unary& operator=(Unary&&) = default;

    Unary(Token op_, std::shared_ptr<Expr> right_) : 
        op(op_),
        right(right_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitUnaryExpr(shared_from_this());
    }

    Token op;
    std::shared_ptr<Expr> right;
}; // end Unary

class Variable : 
    public Expr,
    public std::enable_shared_from_this<Variable>
{
public:
    Variable() = default;
    ~Variable() = default;
    Variable(const Variable&) = default;
    Variable& operator=(const Variable&) = default;
    Variable(Variable&&) = default;
    Variable& operator=(Variable&&) = default;

    Variable(Token name_) : 
        name(name_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitVariableExpr(shared_from_this());
    }

    Token name;
}; // end Variable

class Assign : 
    public Expr,
    public std::enable_shared_from_this<Assign>
{
public:
    Assign() = default;
    ~Assign() = default;
    Assign(const Assign&) = default;
    Assign& operator=(const Assign&) = default;
    Assign(Assign&&) = default;
    Assign& operator=(Assign&&) = default;

    Assign(Token name_, std::shared_ptr<Expr> value_) : 
        name(name_),
        value(value_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitAssignExpr(shared_from_this());
    }

    Token name;
    std::shared_ptr<Expr> value;
}; // end Assign

class Logical : 
    public Expr,
    public std::enable_shared_from_this<Logical>
{
public:
    Logical() = default;
    ~Logical() = default;
    Logical(const Logical&) = default;
    Logical& operator=(const Logical&) = default;
    Logical(Logical&&) = default;
    Logical& operator=(Logical&&) = default;

    Logical(std::shared_ptr<Expr> left_, Token op_, std::shared_ptr<Expr> right_) : 
        left(left_),
        op(op_),
        right(right_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitLogicalExpr(shared_from_this());
    }

    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
}; // end Logical

class Call : 
    public Expr,
    public std::enable_shared_from_this<Call>
{
public:
    Call() = default;
    ~Call() = default;
    Call(const Call&) = default;
    Call& operator=(const Call&) = default;
    Call(Call&&) = default;
    Call& operator=(Call&&) = default;

    Call(std::shared_ptr<Expr> callee_, Token paren_, std::vector<std::shared_ptr<Expr>> arguments_) : 
        callee(callee_),
        paren(paren_),
        arguments(arguments_)
    {

    }

    std::any Accept(ExprVisitor& visitor) final
    {
        return visitor.VisitCallExpr(shared_from_this());
    }

    std::shared_ptr<Expr> callee;
    Token paren;
    std::vector<std::shared_ptr<Expr>> arguments;
}; // end Call
} // end ast
} // end lox