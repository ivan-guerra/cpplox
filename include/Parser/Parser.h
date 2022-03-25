#pragma once

#include <vector>
#include <memory>
#include <exception>
#include <initializer_list>

#include "Expr.h"
#include "Stmt.h"
#include "Scanner.h"

namespace lox
{
/*!
 * \class Parser
 * \brief The Parser class implements the Lox lang parser.
 *
 * Parser implements a recursive decent parsing strategy. You can find the
 * complete Lox lang grammar
 * <a href="https://craftinginterpreters.com/appendix-i.html">here</a>.
 */
class Parser
{
public:
    Parser() = delete;

    ~Parser() = default;

    /*!
     * \brief Construct a Parser parsing the token stream \a tokens.
     *
     * \param tokens A vector of Token objects obtained by the Scanner.
     */
    Parser(const std::vector<Token> tokens) : current_(0), tokens_(tokens) { }

    /* Default copy construction and assignment is valid. */
    Parser(const Parser&) = default;
    Parser& operator=(const Parser&) = default;

    /* Default move construction and assignment is valid. */
    Parser(Parser&&) = default;
    Parser& operator=(Parser&&) = default;

    /*!
     * \brief Parse the Stmt ASTs contained within the token stream.
     *
     * \return A vector of pointers to the root node of zero or more Stmt ASTs.
     */
    std::vector<std::shared_ptr<ast::Stmt>> Parse();

private:
    using ExprPtr = std::shared_ptr<ast::Expr>;
    using StmtPtr = std::shared_ptr<ast::Stmt>;

    /*!
     * \class ParserException
     * \brief ParserException is a sentinel class used to unwind the parser.
     *
     * See
     * https://craftinginterpreters.com/parsing-expressions.html#syntax-errors
     * for a clear description of how ParserException is used.
     */
    class ParserException : public std::exception { }; // end ParserException

    /*!
     * \brief Print an error message and return a ParserException.
     *
     * Error() makes use of the ErrorLogger to print the \a token line number
     * and \a message to stderr.
     *
     * \param token A Token.
     * \param message An error message.
     *
     * \return A ParserException object.
     */
    ParserException Error(const Token& token,
                          const std::string& message) const;

    /*!
     * \brief Unwind the parser to a known state.
     *
     * See https://craftinginterpreters.com/parsing-expressions.html#synchronizing-a-recursive-descent-parser
     * for details.
     */
    void Synchronize();

    /*!
     * \brief Return the current token.
     */
    Token Peek() const
        { return tokens_.at(current_); }

    /*!
     * Return the previously consumed token.
     */
    Token Previous() const
        { return tokens_.at(current_ - 1); }

    /*!
     * \brief Return \c true if we have reached EOF.
     */
    bool IsAtEnd() const
        { return (Peek().GetType() == Token::TokenType::kEof); }

    /*!
     * \brief Consume a token and return it.
     */
    Token Advance();

    /*!
     * \brief Return \c true if the current token's type matches \a token_type.
     */
    bool Check(Token::TokenType token_type) const
        { return (IsAtEnd()) ? false : (Peek().GetType() == token_type); }

    /*!
     * \brief Return \c true if any of the argument types match the current type.
     */
    bool Match(const std::initializer_list<Token::TokenType>& token_types);

    /*!
     * \brief Consume a token if the current token type matches \a type.
     *
     * \return The consumed Token object.
     */
    Token Consume(Token::TokenType type, const std::string& message);

    /* === Statement Productions === */

    StmtPtr Declaration();

    StmtPtr ClassDeclaration();

    StmtPtr Function(const std::string& kind);

    StmtPtr VarDeclaration();

    StmtPtr Statement();

    StmtPtr IfStatement();

    StmtPtr PrintStatement();

    StmtPtr ForStatement();

    StmtPtr ReturnStatement();

    StmtPtr WhileStatement();

    StmtPtr ExpressionStatement();

    std::vector<StmtPtr> Block();

    /* === Expression Productions === */

    ExprPtr Expression()
        { return Assignment(); }

    ExprPtr Or();

    ExprPtr And();

    ExprPtr Assignment();

    ExprPtr Equality();

    ExprPtr Comparison();

    ExprPtr Term();

    ExprPtr Factor();

    ExprPtr Unary();

    ExprPtr FinishCall(ExprPtr callee);

    ExprPtr Call();

    ExprPtr Primary();

    int                current_; /*!< Index of the current parse token. */
    std::vector<Token> tokens_;  /*!< Token stream. */
}; // end Parser
} // end lox
