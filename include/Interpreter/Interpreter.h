#pragma once

#include <any>
#include <memory>
#include <vector>
#include <unordered_map>

#include "Expr.h"
#include "Stmt.h"
#include "Scanner.h"
#include "Environment.h"
#include "RuntimeError.h"

namespace lox
{
/*!
 * \class Interpreter
 * \brief The Interpreter class implements the Lox lang interpreter.
 */
class Interpreter :
    public ast::ExprVisitor,
    public ast::StmtVisitor
{
public:
    Interpreter();
    ~Interpreter() = default;

    /* Default copy construction and assignment is valid. */
    Interpreter(const Interpreter&) = default;
    Interpreter& operator=(const Interpreter&) = default;

    /* Default move construction and assignment is valid. */
    Interpreter(Interpreter&&) = default;
    Interpreter& operator=(Interpreter&&) = default;

    void VisitExpressionStmt(std::shared_ptr<ast::Expression> stmt) final
        { Evaluate(stmt->expression); }

    void VisitPrintStmt(std::shared_ptr<ast::Print> stmt) final;

    void VisitVarStmt(std::shared_ptr<ast::Var> stmt) final;

    void VisitBlockStmt(std::shared_ptr<ast::Block> stmt) final;

    void VisitIfStmt(std::shared_ptr<ast::If> stmt) final;

    void VisitWhileStmt(std::shared_ptr<ast::While> stmt) final;

    void VisitFunctionStmt(std::shared_ptr<ast::Function> stmt) final;

    void VisitReturnStmt(std::shared_ptr<ast::Return> stmt) final;

    std::any VisitBinaryExpr(std::shared_ptr<ast::Binary> expr) final;

    std::any VisitGroupingExpr(std::shared_ptr<ast::Grouping> expr) final
        { return Evaluate(expr->expression); }

    std::any VisitLiteralExpr(std::shared_ptr<ast::Literal> expr) final
        { return expr->value; }

    std::any VisitUnaryExpr(std::shared_ptr<ast::Unary> expr) final;

    std::any VisitVariableExpr(std::shared_ptr<ast::Variable> expr) final
        { return LookupVariable(expr->name, expr); }

    std::any VisitAssignExpr(std::shared_ptr<ast::Assign> expr) final;

    std::any VisitLogicalExpr(std::shared_ptr<ast::Logical> expr) final;

    std::any VisitCallExpr(std::shared_ptr<ast::Call> expr) final;

    /*!
     * \brief Execute each statement in \a statements.
     */
    void Interpret(const std::vector<std::shared_ptr<ast::Stmt>>& statements);

    /*!
     * \brief Method used by the Resolver to register variable resolution info.
     *
     * \param expr Node in the AST assoicated with resolution info.
     * \param depth Number of Environments to walk back in the Environment
     *              chain when attempting to resolve a variable reference
     *              in \a expr at runtime.
     */
    void Resolve(std::shared_ptr<ast::Expr> expr, int depth)
        { locals_[expr] = depth; }

private:
    using EnvPtr    = std::shared_ptr<Environment>;
    using LocalsMap = std::unordered_map<std::shared_ptr<ast::Expr>, int>;

    /*!
     * \class LoxCallable
     * \brief The LoxCallable class defines the interface for any callable Lox object.
     */
    class LoxCallable
    {
    public:
        /*!
         * \brief Action performed by the object when it is called.
         *
         * \param interpreter Instance of this Interpreter.
         * \param arguments Call arguments.
         *
         * \return The result of the object action.
         */
        virtual std::any Call(Interpreter& interpreter,
                              std::vector<std::any>& arguments) = 0;

        /*!
         * \brief Return number of arguments expected by the callable object.
         */
        virtual std::size_t Arity() const = 0;
    }; // end LoxCallable

    /*!
     * \class Clock
     * \brief The Clock class implements the builtin Lox clock function.
     *
     * The Clock class implements a Lox built-in clock function. The Clock
     * class is handy when benchmarking code. It allows you to generate a
     * timestamp in seconds using the system clock as its source.
     */
    class Clock :
        public LoxCallable
    {
        public:
            /*!
             * \brief Return the current system time in seconds.
             */
            std::any Call([[maybe_unused]]Interpreter& interpreter,
                [[maybe_unused]]std::vector<std::any>& arguments) final;

            std::size_t Arity() const final
                { return 0; }
    }; // end Clock

    /*!
     * \class LoxFunction
     * \brief The LoxFunction class implements Lox function calls.
     */
    class LoxFunction :
        public LoxCallable
    {
        public:
            LoxFunction() = delete;
            LoxFunction(std::shared_ptr<ast::Function> decl,
                        std::shared_ptr<Environment> cls) :
                declaration(decl),
                closure(cls)
                { }

            /*!
             * \brief Perform the function call and return the result.
             * \return If the function called has no return value, nil is
             *         returned.
             */
            std::any Call(Interpreter& interpreter,
                          std::vector<std::any>& arguments) final;

            std::size_t Arity() const final
                { return declaration->params.size(); }

            std::shared_ptr<ast::Function> declaration; /*!< Function declaration statement. */
            std::shared_ptr<Environment>   closure;     /*!< Closure environment. */
    }; // end LoxFunction

    /*!
     * \brief Execute the code represented by \a stmt.
     */
    void Execute(std::shared_ptr<ast::Stmt> stmt)
        { if (stmt) stmt->Accept(*this); }

    /*!
     * \brief Call Execute() on all statements in \a statements.
     *
     * \param statements Vector of zero or more statements within a code block.
     * \param env Environment of the block being executed.
     */
    void ExecuteBlock(const std::vector<std::shared_ptr<ast::Stmt>>& statements,
                      std::shared_ptr<Environment> env);

    /*!
     * \brief Return the result of the evaluating \a expr.
     */
    std::any Evaluate(std::shared_ptr<ast::Expr> expr)
        { return expr->Accept(*this); }

    /*!
     * \brief Return \c true if \a object evaluates to true.
     *
     * In lox, we define \c false as \a nil or the literal \c false. All other
     * values are \c true.
     */
    bool IsTruth(const std::any& object) const;

    /*!
     * \brief Determine if \a left and \a right are equal.
     *
     * We only check for equality for string, number, boolean, and nil types.
     * An equality check of any other type or mismatched types has an automatic
     * return value of \c false.
     */
    bool IsEqual(const std::any& left, const std::any& right) const;

    /*!
     * \brief Throw a RuntimeError if \a operand is not a lox number.
     */
    void CheckNumberOperand(const Token& op, const std::any& operand) const;

    /*!
     * \brief Throw a RuntimeError if either \a left or \a right is not a lox number.
     */
    void CheckNumberOperands(const Token& op,
                             const std::any& left,
                             const std::any& right) const;

    /*!
     * \brief Return the string representation of \a object.
     */
    std::string Stringify(const std::any& object);

    /*!
     * \brief Return the value associated with \a name.
     *
     * LookupVariable() walks backwards through the Environment list searching
     * for reference to \a name. The exact number of steps taken in the
     * Environment chain is stored in #locals_ and is used during traversal
     * to ensure the "right" reference to \a name is found.
     *
     * \param name Token that is searched for in the Environment.
     * \param expr Pointer to the AST node used to find the depth of \a name
     *             in the Environment list (see #locals_).
     *
     * \return Resolution of \a name.
     */
    std::any LookupVariable(const Token& name,
                            std::shared_ptr<ast::Expr> expr);

    EnvPtr    globals_;     /*!< Global scope Environment. */
    EnvPtr    environment_; /*!< Active Environment. */
    LocalsMap locals_;      /*!< Map of local variables and their depth in the Environment list. */
}; // end Interpreter
} // end lox
