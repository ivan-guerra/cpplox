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
class Class;
class If;
class While;
class Function;
class Return;

class StmtVisitor
{
public:
    virtual void VisitExpressionStmt(std::shared_ptr<Expression> stmt) = 0;
    virtual void VisitPrintStmt(std::shared_ptr<Print> stmt) = 0;
    virtual void VisitVarStmt(std::shared_ptr<Var> stmt) = 0;
    virtual void VisitBlockStmt(std::shared_ptr<Block> stmt) = 0;
    virtual void VisitClassStmt(std::shared_ptr<Class> stmt) = 0;
    virtual void VisitIfStmt(std::shared_ptr<If> stmt) = 0;
    virtual void VisitWhileStmt(std::shared_ptr<While> stmt) = 0;
    virtual void VisitFunctionStmt(std::shared_ptr<Function> stmt) = 0;
    virtual void VisitReturnStmt(std::shared_ptr<Return> stmt) = 0;
}; // end Visitor

class Stmt
{
public:
    virtual ~Stmt() = default;
    virtual void Accept(StmtVisitor& visitor) = 0;
}; // end Stmt

class Expression : 
    public Stmt,
    public std::enable_shared_from_this<Expression>
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
        visitor.VisitExpressionStmt(shared_from_this());
    }

    std::shared_ptr<Expr> expression;
}; // end Expression

class Print : 
    public Stmt,
    public std::enable_shared_from_this<Print>
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
        visitor.VisitPrintStmt(shared_from_this());
    }

    std::shared_ptr<Expr> expression;
}; // end Print

class Var : 
    public Stmt,
    public std::enable_shared_from_this<Var>
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
        visitor.VisitVarStmt(shared_from_this());
    }

    Token name;
    std::shared_ptr<Expr> initializer;
}; // end Var

class Block : 
    public Stmt,
    public std::enable_shared_from_this<Block>
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
        visitor.VisitBlockStmt(shared_from_this());
    }

    std::vector<std::shared_ptr<Stmt>> statements;
}; // end Block

class Class : 
    public Stmt,
    public std::enable_shared_from_this<Class>
{
public:
    Class() = default;
    ~Class() = default;
    Class(const Class&) = default;
    Class& operator=(const Class&) = default;
    Class(Class&&) = default;
    Class& operator=(Class&&) = default;

    Class(Token name_, std::shared_ptr<Variable> superclass_, std::vector<std::shared_ptr<Function>> methods_) : 
        name(name_),
        superclass(superclass_),
        methods(methods_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitClassStmt(shared_from_this());
    }

    Token name;
    std::shared_ptr<Variable> superclass;
    std::vector<std::shared_ptr<Function>> methods;
}; // end Class

class If : 
    public Stmt,
    public std::enable_shared_from_this<If>
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
        visitor.VisitIfStmt(shared_from_this());
    }

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> then_branch;
    std::shared_ptr<Stmt> else_branch;
}; // end If

class While : 
    public Stmt,
    public std::enable_shared_from_this<While>
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
        visitor.VisitWhileStmt(shared_from_this());
    }

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
}; // end While

class Function : 
    public Stmt,
    public std::enable_shared_from_this<Function>
{
public:
    Function() = default;
    ~Function() = default;
    Function(const Function&) = default;
    Function& operator=(const Function&) = default;
    Function(Function&&) = default;
    Function& operator=(Function&&) = default;

    Function(Token name_, std::vector<Token> params_, std::vector<std::shared_ptr<Stmt>> body_) : 
        name(name_),
        params(params_),
        body(body_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitFunctionStmt(shared_from_this());
    }

    Token name;
    std::vector<Token> params;
    std::vector<std::shared_ptr<Stmt>> body;
}; // end Function

class Return : 
    public Stmt,
    public std::enable_shared_from_this<Return>
{
public:
    Return() = default;
    ~Return() = default;
    Return(const Return&) = default;
    Return& operator=(const Return&) = default;
    Return(Return&&) = default;
    Return& operator=(Return&&) = default;

    Return(Token keyword_, std::shared_ptr<Expr> value_) : 
        keyword(keyword_),
        value(value_)
    {

    }

    void Accept(StmtVisitor& visitor) final
    {
        visitor.VisitReturnStmt(shared_from_this());
    }

    Token keyword;
    std::shared_ptr<Expr> value;
}; // end Return
} // end ast
} // end lox