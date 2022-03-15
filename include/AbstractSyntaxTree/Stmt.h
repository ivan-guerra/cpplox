/* Autogenerated using generate_ast.py, DO NOT EDIT. */
#pragma once

#include <memory>
#include "Expr.h"

namespace lox
{
namespace ast
{
class Expression;
class Print;
class Var;
class Block;
class If;
class While;

class StmtVisitor
{
public:
    virtual void VisitExpressionStmt(Expression& stmt) = 0;
    virtual void VisitPrintStmt(Print& stmt) = 0;
    virtual void VisitVarStmt(Var& stmt) = 0;
    virtual void VisitBlockStmt(Block& stmt) = 0;
    virtual void VisitIfStmt(If& stmt) = 0;
    virtual void VisitWhileStmt(While& stmt) = 0;
}; // end Visitor

class Stmt
{
public:
    virtual ~Stmt() = default;
    virtual void Accept(StmtVisitor& visitor) = 0;
}; // end Stmt

class Expression : public Stmt
{
public:
    Expression() = default;
    ~Expression() = default;
    Expression(const Expression&) = default;
    Expression& operator=(const Expression&) = default;
    Expression(Expression&&) = default;
    Expression& operator=(Expression&&) = default;

    Expression(std::shared_ptr<Expr> expression_) : 
        expression(expression_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitExpressionStmt(*this);
    }

    std::shared_ptr<Expr> expression;
}; // end Expression

class Print : public Stmt
{
public:
    Print() = default;
    ~Print() = default;
    Print(const Print&) = default;
    Print& operator=(const Print&) = default;
    Print(Print&&) = default;
    Print& operator=(Print&&) = default;

    Print(std::shared_ptr<Expr> expression_) : 
        expression(expression_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitPrintStmt(*this);
    }

    std::shared_ptr<Expr> expression;
}; // end Print

class Var : public Stmt
{
public:
    Var() = default;
    ~Var() = default;
    Var(const Var&) = default;
    Var& operator=(const Var&) = default;
    Var(Var&&) = default;
    Var& operator=(Var&&) = default;

    Var(Token name_, std::shared_ptr<Expr> initializer_) : 
        name(name_),
        initializer(initializer_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitVarStmt(*this);
    }

    Token name;
    std::shared_ptr<Expr> initializer;
}; // end Var

class Block : public Stmt
{
public:
    Block() = default;
    ~Block() = default;
    Block(const Block&) = default;
    Block& operator=(const Block&) = default;
    Block(Block&&) = default;
    Block& operator=(Block&&) = default;

    Block(std::vector<std::shared_ptr<Stmt>> statements_) : 
        statements(statements_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitBlockStmt(*this);
    }

    std::vector<std::shared_ptr<Stmt>> statements;
}; // end Block

class If : public Stmt
{
public:
    If() = default;
    ~If() = default;
    If(const If&) = default;
    If& operator=(const If&) = default;
    If(If&&) = default;
    If& operator=(If&&) = default;

    If(std::shared_ptr<Expr> condition_, std::shared_ptr<Stmt> then_branch_, std::shared_ptr<Stmt> else_branch_) : 
        condition(condition_),
        then_branch(then_branch_),
        else_branch(else_branch_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitIfStmt(*this);
    }

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> then_branch;
    std::shared_ptr<Stmt> else_branch;
}; // end If

class While : public Stmt
{
public:
    While() = default;
    ~While() = default;
    While(const While&) = default;
    While& operator=(const While&) = default;
    While(While&&) = default;
    While& operator=(While&&) = default;

    While(std::shared_ptr<Expr> condition_, std::shared_ptr<Stmt> body_) : 
        condition(condition_),
        body(body_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitWhileStmt(*this);
    }

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
}; // end While
} // end ast
} // end lox