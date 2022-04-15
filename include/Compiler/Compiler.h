#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#include "Value.h"
#include "Chunk.h"
#include "Scanner.h"

namespace lox
{
class Compiler
{
public:
    Compiler();
    ~Compiler() = default;

    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    Compiler(Compiler&&) = delete;
    Compiler& operator=(Compiler&&) = delete;

    bool Compile(const std::string& source,
                 std::shared_ptr<Chunk> chunk);

private:
    using ParseFn = std::function<void(Compiler*)>;

    enum Precedence
    {
        kPrecNone,
        kPrecAssignment, /* = */
        kPrecOr,         /* or */
        kPrecAnd,        /* and */
        kPrecEquality,   /* == != */
        kPrecComparison, /* < > <= >= */
        kPrecTerm,       /* + - */
        kPrecFactor,     /* * / */
        kPrecUnary,      /* ! - */
        kPrecCall,       /* . () */
        kPrecPrimary
    }; // end Precedence

    struct Parser
    {
        Token current;
        Token previous;
        bool  had_error;
        bool  panic_mode;
    }; // end Parser

    struct ParseRule
    {
        ParseFn    prefix;
        ParseFn    infix;
        Precedence precedence;
    }; // end ParseRule

    static std::unordered_map<Token::TokenType, ParseRule> rules_;

    void ParsePrecedence(Precedence precedence);

    void Expression()
        { ParsePrecedence(Precedence::kPrecAssignment); }

    void Advance();

    void Consume(Token::TokenType type, const std::string& message);

    uint8_t MakeConstant(value::value_t value);

    void ErrorAt(const Token& error, const std::string& message);

    void ErrorAtCurrent(const std::string& message)
        { ErrorAt(parser_.current, message); }

    void Error(const std::string& message)
        { ErrorAt(parser_.previous, message); }

    void EmitByte(uint8_t byte)
        { chunk_->Write(byte, parser_.previous.GetLine()); }

    void EmitBytes(uint8_t byte1, uint8_t byte2);

    void EmitReturn()
        { EmitByte(Chunk::OpCode::kOpReturn); }

    void EmitConstant(value::value_t value)
        { EmitBytes(Chunk::OpCode::kOpConstant, MakeConstant(value)); }

    void EndCompiler();

    void Number();

    void Grouping();

    void Unary();

    void Binary();

    Scanner                scanner_;
    std::shared_ptr<Chunk> chunk_;
    Parser                 parser_;
}; // end Compiler
} // end lox
