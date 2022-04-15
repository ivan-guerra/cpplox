#include <cstdio>
#include <climits>
#include <string>
#include <memory>

#include "Scanner.h"
#include "Compiler.h"

namespace lox
{
std::unordered_map<Token::TokenType, Compiler::ParseRule> Compiler::rules_ =
{
    {Token::TokenType::kLeftParen,
        {&Compiler::Grouping, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kRightParen,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kLeftBrace,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kRightBrace,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kComma,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kDot,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kMinus,
        {&Compiler::Unary, &Compiler::Binary, Precedence::kPrecTerm}},
    {Token::TokenType::kPlus,
        {nullptr, &Compiler::Binary, Precedence::kPrecTerm}},
    {Token::TokenType::kSemicolon,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kSlash,
        {nullptr, &Compiler::Binary, Precedence::kPrecFactor}},
    {Token::TokenType::kStar,
        {nullptr, &Compiler::Binary, Precedence::kPrecFactor}},
    {Token::TokenType::kBang,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kBangEqual,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kEqual,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kEqualEqual,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kGreater,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kGreaterEqual,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kLess,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kLessEqual,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kIdentifier,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kString,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kNumber,
        {&Compiler::Number, nullptr,  Precedence::kPrecNone}},
    {Token::TokenType::kAnd,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kClass,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kElse,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kFalse,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kFun,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kFor,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kIf,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kNil,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kOr,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kPrint,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kReturn,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kSuper,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kThis,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kTrue,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kVar,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kWhile,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kError,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kEof,
        {nullptr, nullptr, Precedence::kPrecNone}}
};

void Compiler::ParsePrecedence(Precedence precedence)
{
    Advance();
    ParseFn prefix_rule = rules_[parser_.previous.GetType()].prefix;
    if (!prefix_rule) {
        Error("Expect expression.");
        return;
    }
    prefix_rule(this);

    while (precedence <= rules_[parser_.current.GetType()].precedence) {
        Advance();
        ParseFn infix_rule = rules_[parser_.previous.GetType()].infix;
        infix_rule(this);
    }
}

void Compiler::Advance()
{
    parser_.previous = parser_.current;
    while (true) {
        parser_.current = scanner_.ScanToken();
        if (parser_.current.GetType() != Token::TokenType::kError)
            break;

        ErrorAtCurrent(parser_.current.GetLexeme());
    }
}

void Compiler::Consume(Token::TokenType type, const std::string& message)
{
    if (parser_.current.GetType() == type) {
        Advance();
        return;
    }
    ErrorAtCurrent(message);
}

uint8_t Compiler::MakeConstant(value::value_t value)
{
    int constant = chunk_->AddConstant(value);
    if (constant > UINT8_MAX) {
        Error("Too many constants in one chunk.");
        return 0;
    }
    return static_cast<uint8_t>(constant);
}

void Compiler::ErrorAt(const Token& error, const std::string& message)
{
    if (parser_.panic_mode)
        return;

    parser_.panic_mode = true;

    std::fprintf(stderr, "[line %d] Error", error.GetLine());

    if (error.GetType() == Token::TokenType::kEof) {
        fprintf(stderr, " at end");
    } else if (error.GetType() == Token::TokenType::kError) {
        /* Do nothing. */
    } else {
        fprintf(stderr, " at %s", error.GetLexeme().c_str());
    }
    fprintf(stderr, ": %s\n", message.c_str());

    parser_.had_error = true;
}

void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2)
{
    EmitByte(byte1);
    EmitByte(byte2);
}

void Compiler::EndCompiler()
{
    EmitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser_.had_error)
        chunk_->Disassemble("code");
#endif
}

void Compiler::Number()
{
    double value = std::stod(parser_.previous.GetLexeme());
    EmitConstant(value);
}

void Compiler::Grouping()
{
    Expression();
    Consume(Token::TokenType::kRightParen, "Expect ')' after expression.");
}

void Compiler::Unary()
{
    Token::TokenType operator_type = parser_.previous.GetType();

    /* Compile the operand. */
    ParsePrecedence(Precedence::kPrecUnary);

    /* Emit the operator instruction. */
    switch (operator_type) {
        case Token::TokenType::kMinus:
            EmitByte(Chunk::OpCode::kOpNegate);
            break;
        default:
            /* Unreachable */
            return;
    }
}

void Compiler::Binary()
{
    Token::TokenType operator_type = parser_.previous.GetType();
    ParsePrecedence(
        static_cast<Precedence>(rules_[operator_type].precedence + 1));

    switch (operator_type) {
        case Token::TokenType::kPlus:
            EmitByte(Chunk::OpCode::kOpAdd);
            break;
        case Token::TokenType::kMinus:
            EmitByte(Chunk::OpCode::kOpSubtract);
            break;
        case Token::TokenType::kStar:
            EmitByte(Chunk::OpCode::kOpMultiply);
            break;
        case Token::TokenType::kSlash:
            EmitByte(Chunk::OpCode::kOpDivide);
            break;
        default:
            /* Unreachable */
            return;
    }
}

Compiler::Compiler() :
    scanner_(""),
    chunk_(nullptr)
{
    parser_.had_error  = false;
    parser_.panic_mode = false;
}

bool Compiler::Compile(const std::string& source,
                       std::shared_ptr<Chunk> chunk)
{
    chunk_ = chunk;

    scanner_ = lox::Scanner(source);
    Advance();
    Expression();
    Consume(Token::TokenType::kEof, "Expect end of expression.");

    EndCompiler();

    return !parser_.had_error;
}
} // end lox
