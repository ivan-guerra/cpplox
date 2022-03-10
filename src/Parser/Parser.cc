#include <memory>
#include <unordered_set>
#include <initializer_list>

#include "Expr.h"
#include "Parser.h"
#include "Scanner.h"
#include "ErrorLogger.h"

namespace lox
{
Parser::ParserException Parser::Error(const Token& token,
                                      const std::string& message) const
{
    LOG_ERROR(token.GetLine(), message);
    return ParserException();
}

void Parser::Synchronize()
{
    Advance();

    static const std::unordered_set<Token::TokenType> kBoundaryMarker = {
        Token::TokenType::kClass,
        Token::TokenType::kFun,
        Token::TokenType::kVar,
        Token::TokenType::kFor,
        Token::TokenType::kIf,
        Token::TokenType::kWhile,
        Token::TokenType::kPrint,
        Token::TokenType::kReturn
    };
    while (!IsAtEnd()) {
        if ((Previous().GetType() == Token::TokenType::kSemicolon) ||
            kBoundaryMarker.count(Peek().GetType()))
            return;

        Advance();
    }
}

Token Parser::Advance()
{
    if (!IsAtEnd())
        current_++;

    return Previous();
}

bool Parser::Match(
    const std::initializer_list<Token::TokenType>& token_types)
{
    for (const auto& type : token_types) {
        if (Check(type)) {
            Advance();
            return true;
        }
    }
    return false;
}

Token Parser::Consume(Token::TokenType type, const std::string& message)
{
    if (Check(type))
        return Advance();

    throw Error(Peek(), message);
}

std::shared_ptr<Expr> Parser::Equality()
{
    ExprPtr expr = Comparison();

    static const std::initializer_list<Token::TokenType> kEqualityTokens = {
        Token::TokenType::kBangEqual,
        Token::TokenType::kEqualEqual
    };
    while (Match(kEqualityTokens)) {
        Token op = Previous();
        ExprPtr right = Comparison();
        expr = std::make_shared<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::Comparison()
{
    ExprPtr expr = Term();

    static const std::initializer_list<Token::TokenType> kTermTokens = {
        Token::TokenType::kGreater,
        Token::TokenType::kGreaterEqual,
        Token::TokenType::kLess,
        Token::TokenType::kLessEqual
    };
    while (Match(kTermTokens)) {
        Token op = Previous();
        ExprPtr right = Term();
        expr = std::make_shared<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::Term()
{
    ExprPtr expr = Factor();

    static const std::initializer_list<Token::TokenType> kTermTokens = {
        Token::TokenType::kPlus,
        Token::TokenType::kMinus
    };
    while (Match(kTermTokens)) {
        Token op = Previous();
        ExprPtr right = Factor();
        expr = std::make_shared<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::Factor()
{
    ExprPtr expr = Unary();

    static const std::initializer_list<Token::TokenType> kFactorTokens = {
        Token::TokenType::kSlash,
        Token::TokenType::kStar
    };
    while (Match(kFactorTokens)) {
        Token op = Previous();
        ExprPtr right = Unary();
        expr = std::make_shared<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::Unary()
{
    static const std::initializer_list<Token::TokenType> kUnaryTokens = {
        Token::TokenType::kBang,
        Token::TokenType::kMinus
    };
    if (Match(kUnaryTokens)) {
        Token op = Previous();
        ExprPtr right = Unary();
        return std::make_shared<lox::Unary>(op, right);
    }
    return Primary();
}

std::shared_ptr<Expr> Parser::Primary()
{
    if (Match({Token::TokenType::kFalse}))
        return std::make_shared<Literal>(std::string("false"));
    if (Match({Token::TokenType::kTrue}))
        return std::make_shared<Literal>(std::string("true"));
    if (Match({Token::TokenType::kNil}))
        return std::make_shared<Literal>(std::string("nil"));

    if (Match({Token::TokenType::kNumber, Token::TokenType::kString}))
        return std::make_shared<Literal>(Previous().GetLiteral());

    if (Match({Token::TokenType::kLeftParen})) {
        ExprPtr expr = Expression();
        Consume(Token::TokenType::kRightParen,
                "expected ')' after expression");
        return std::make_shared<Grouping>(expr);
    }

    throw Error(Peek(), "expected expression");
}

std::shared_ptr<Expr> Parser::Parse()
{
    try {
        return Expression();
    } catch (ParserException& e) {
        return nullptr;
    }
}
} // end lox
