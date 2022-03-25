#include <memory>
#include <unordered_set>
#include <initializer_list>
#include <sstream>

#include "Expr.h"
#include "Stmt.h"
#include "Parser.h"
#include "Scanner.h"
#include "ErrorLogger.h"

namespace lox
{
Parser::ParserException Parser::Error(const Token& token,
                                      const std::string& message) const
{
    LOG_STATIC_ERROR(token.GetLine(), message);
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

Parser::StmtPtr Parser::Declaration()
{
    try {
        if (Match({Token::TokenType::kClass}))
            return ClassDeclaration();

        if (Match({Token::TokenType::kFun}))
            return Function("function");

        if (Match({Token::TokenType::kVar}))
            return VarDeclaration();

        return Statement();
    } catch (const ParserException& e) {
        Synchronize();
        return nullptr;
    }
}

Parser::StmtPtr Parser::ClassDeclaration()
{
    Token name = Consume(Token::TokenType::kIdentifier, "Expect class name.");
    Consume(Token::TokenType::kLeftBrace, "Expect '{' before class body.");

    std::vector<std::shared_ptr<ast::Function>> methods;
    while (!Check(Token::TokenType::kRightBrace) && !IsAtEnd()) {
        methods.push_back(
            std::static_pointer_cast<ast::Function>(Function("method")));
    }

    Consume(Token::TokenType::kRightBrace, "Expect '}' after class body.");

    return std::make_shared<ast::Class>(name, methods);
}

Parser::StmtPtr Parser::Function(const std::string& kind)
{
    Token name =
        Consume(Token::TokenType::kIdentifier,
                std::string("Expect ") + kind + std::string(" name."));
    Consume(Token::TokenType::kLeftParen,
            std::string("Expect '(' after ") + kind + std::string(" name."));

    std::vector<Token> parameters;
    if (!Check(Token::TokenType::kRightParen)) {
        do {
            if (parameters.size() >= 255)
                Error(Peek(), "Can't have more than 255 parameters.");

            parameters.push_back(Consume(Token::TokenType::kIdentifier,
                                         "Expect parameter name."));
        } while (Match({Token::TokenType::kComma}));
    }
    Consume(Token::TokenType::kRightParen, "Expect ')' after parameters.");

    Consume(Token::TokenType::kLeftBrace,
            std::string("Expect '{' before ") + kind + std::string(" body."));
    std::vector<StmtPtr> body = Block();

    return std::make_shared<ast::Function>(name, parameters, body);
}

Parser::StmtPtr Parser::VarDeclaration()
{
    Token name = Consume(Token::TokenType::kIdentifier,
                         "Expect variable name.");

    ExprPtr initializer = nullptr;
    if (Match({Token::TokenType::kEqual}))
        initializer = Expression();

    Consume(Token::TokenType::kSemicolon, "Expect ';' after variable declaration.");
    return std::make_shared<ast::Var>(name, initializer);
}

Parser::StmtPtr Parser::Statement()
{
    if (Match({Token::TokenType::kIf}))
        return IfStatement();

    if (Match({Token::TokenType::kPrint}))
        return PrintStatement();

    if (Match({Token::TokenType::kFor}))
        return ForStatement();

    if (Match({Token::TokenType::kReturn}))
        return ReturnStatement();

    if (Match({Token::TokenType::kWhile}))
        return WhileStatement();

    if (Match({Token::TokenType::kLeftBrace}))
        return std::make_shared<ast::Block>(Parser::Block());

    return ExpressionStatement();
}

Parser::StmtPtr Parser::IfStatement()
{
    Consume(Token::TokenType::kLeftParen, "Expect '(' after 'if'.");
    ExprPtr condition = Expression();
    Consume(Token::TokenType::kRightParen, "Expect ')' after if condition.'");

    StmtPtr then_branch = Statement();
    StmtPtr else_branch = nullptr;
    if (Match({Token::TokenType::kElse}))
        else_branch = Statement();

    return std::make_shared<ast::If>(condition, then_branch, else_branch);
}

Parser::StmtPtr Parser::PrintStatement()
{
    ExprPtr value = Expression();
    Consume(Token::TokenType::kSemicolon, "Expect ';' after value.");

    return std::make_shared<ast::Print>(value);
}

Parser::StmtPtr Parser::ForStatement()
{
    Consume(Token::TokenType::kLeftParen, "Expect '(' after 'for'.");

    StmtPtr initializer = nullptr;
    if (Match({Token::TokenType::kSemicolon}))
        initializer = nullptr;
    else if (Match({Token::TokenType::kVar}))
        initializer = VarDeclaration();
    else
        initializer = ExpressionStatement();

    ExprPtr condition = nullptr;
    if (!Check(Token::TokenType::kSemicolon))
        condition = Expression();
    Consume(Token::TokenType::kSemicolon, "Expect ';' after loop condition.");

    ExprPtr increment = nullptr;
    if (!Check(Token::TokenType::kRightParen))
        increment = Expression();
    Consume(Token::TokenType::kRightParen, "Expect ')' after for clauses.");

    StmtPtr body = Statement();

    using StmtPtrList = std::vector<StmtPtr>;
    if (increment) {
        body = std::make_shared<ast::Block>(
            StmtPtrList{body, std::make_shared<ast::Expression>(increment)}
        );
    }

    if (!condition)
        condition = std::make_shared<ast::Literal>(true);
    body = std::make_shared<ast::While>(condition, body);

    if (initializer)
        body = std::make_shared<ast::Block>(StmtPtrList{initializer, body});

    return body;
}

Parser::StmtPtr Parser::ReturnStatement()
{
    Token keyword = Previous();

    ExprPtr value = nullptr;
    if (!Check(Token::TokenType::kSemicolon))
        value = Expression();

    Consume(Token::TokenType::kSemicolon, "Expect ';' after return value");
    return std::make_shared<ast::Return>(keyword, value);
}

Parser::StmtPtr Parser::WhileStatement()
{
    Consume(Token::TokenType::kLeftParen, "Expect '(' after 'while'.");
    ExprPtr condition = Expression();
    Consume(Token::TokenType::kRightParen, "Expect ')' after condition.");

    StmtPtr body = Statement();

    return std::make_shared<ast::While>(condition, body);
}

Parser::StmtPtr Parser::ExpressionStatement()
{
    ExprPtr expr = Expression();
    Consume(Token::TokenType::kSemicolon, "Expect ';' after expression.");

    return std::make_shared<ast::Expression>(expr);
}

std::vector<Parser::StmtPtr> Parser::Block()
{
    std::vector<StmtPtr> statements;

    while (!Check(Token::TokenType::kRightBrace) && !IsAtEnd())
        statements.push_back(Declaration());

    Consume(Token::TokenType::kRightBrace, "Expect '}' after block.");
    return statements;
}

Parser::ExprPtr Parser::Or()
{
    ExprPtr expr = And();

    while (Match({Token::TokenType::kOr})) {
        Token op = Previous();
        ExprPtr right = And();
        expr = std::make_shared<ast::Logical>(expr, op, right);
    }
    return expr;
}

Parser::ExprPtr Parser::And()
{
    ExprPtr expr = Equality();

    while (Match({Token::TokenType::kAnd})) {
        Token op = Previous();
        ExprPtr right = Equality();
        expr = std::make_shared<ast::Logical>(expr, op, right);
    }
    return expr;
}

Parser::ExprPtr Parser::Assignment()
{
    ExprPtr expr = Or();

    if (Match({Token::TokenType::kEqual})) {
        Token   equals = Previous();
        ExprPtr value  = Assignment();

        if (typeid(*expr) == typeid(ast::Variable)) {
            Token name = std::static_pointer_cast<ast::Variable>(expr)->name;
            return std::make_shared<ast::Assign>(name, value);
        } else if (typeid(*expr) == typeid(ast::Get)) {
            std::shared_ptr<ast::Get> get =
                std::static_pointer_cast<ast::Get>(expr);
            return std::make_shared<ast::Set>(get->object, get->name, value);
        }
        LOG_STATIC_ERROR(equals.GetLine(), "Invalid assignment target.");
    }
    return expr;
}

Parser::ExprPtr Parser::Equality()
{
    ExprPtr expr = Comparison();

    static const std::initializer_list<Token::TokenType> kEqualityTokens = {
        Token::TokenType::kBangEqual,
        Token::TokenType::kEqualEqual
    };
    while (Match(kEqualityTokens)) {
        Token op = Previous();
        ExprPtr right = Comparison();
        expr = std::make_shared<ast::Binary>(expr, op, right);
    }
    return expr;
}

Parser::ExprPtr Parser::Comparison()
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
        expr = std::make_shared<ast::Binary>(expr, op, right);
    }
    return expr;
}

Parser::ExprPtr Parser::Term()
{
    ExprPtr expr = Factor();

    static const std::initializer_list<Token::TokenType> kTermTokens = {
        Token::TokenType::kPlus,
        Token::TokenType::kMinus
    };
    while (Match(kTermTokens)) {
        Token op = Previous();
        ExprPtr right = Factor();
        expr = std::make_shared<ast::Binary>(expr, op, right);
    }
    return expr;
}

Parser::ExprPtr Parser::Factor()
{
    ExprPtr expr = Unary();

    static const std::initializer_list<Token::TokenType> kFactorTokens = {
        Token::TokenType::kSlash,
        Token::TokenType::kStar
    };
    while (Match(kFactorTokens)) {
        Token op = Previous();
        ExprPtr right = Unary();
        expr = std::make_shared<ast::Binary>(expr, op, right);
    }
    return expr;
}

Parser::ExprPtr Parser::Unary()
{
    static const std::initializer_list<Token::TokenType> kUnaryTokens = {
        Token::TokenType::kBang,
        Token::TokenType::kMinus
    };
    if (Match(kUnaryTokens)) {
        Token op = Previous();
        ExprPtr right = Unary();
        return std::make_shared<ast::Unary>(op, right);
    }
    return Call();
}

Parser::ExprPtr Parser::FinishCall(ExprPtr callee)
{
    std::vector<Parser::ExprPtr> arguments;
    if (!Check(Token::TokenType::kRightParen)) {
        do {
            if (arguments.size() >= 255)
                Error(Peek(), "Can't have more than 255 arguments.");

            arguments.push_back(Expression());
        } while (Match({Token::TokenType::kComma}));
    }

    Token paren = Consume(Token::TokenType::kRightParen,
                          "Expect ')' after arguments.");

    return std::make_shared<ast::Call>(callee, paren, arguments);
}

Parser::ExprPtr Parser::Call()
{
    ExprPtr expr = Primary();

    while (true) {
        if (Match({Token::TokenType::kLeftParen})) {
            expr = FinishCall(expr);
        } else if (Match({Token::TokenType::kDot})) {
            Token name = Consume(Token::TokenType::kIdentifier,
                                 "Expect property name after '.'.");
            expr = std::make_shared<ast::Get>(expr, name);
        } else {
            break;
        }
    }
    return expr;
}

Parser::ExprPtr Parser::Primary()
{
    if (Match({Token::TokenType::kFalse}))
        return std::make_shared<ast::Literal>(false);
    if (Match({Token::TokenType::kTrue}))
        return std::make_shared<ast::Literal>(true);
    if (Match({Token::TokenType::kNil}))
        return std::make_shared<ast::Literal>(nullptr);

    if (Match({Token::TokenType::kNumber, Token::TokenType::kString}))
        return std::make_shared<ast::Literal>(Previous().GetLiteral());

    if (Match({Token::TokenType::kThis}))
        return std::make_shared<ast::This>(Previous());

    if (Match({Token::TokenType::kIdentifier}))
        return std::make_shared<ast::Variable>(Previous());

    if (Match({Token::TokenType::kLeftParen})) {
        ExprPtr expr = Expression();
        Consume(Token::TokenType::kRightParen,
                "expected ')' after expression");
        return std::make_shared<ast::Grouping>(expr);
    }

    throw Error(Peek(), "expected expression");
}

std::vector<std::shared_ptr<ast::Stmt>> Parser::Parse()
{
    std::vector<StmtPtr> statements;
    try {
        while (!IsAtEnd())
            statements.push_back(Declaration());

        return statements;
    } catch (ParserException& e) {
        return statements;
    }
}
} // end lox
