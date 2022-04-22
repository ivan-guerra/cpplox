#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

#include "Value.h"
#include "Chunk.h"
#include "Scanner.h"

namespace lox
{
/*!
 * \class Compiler
 * \brief Compile source text to bytecode.
 */
class Compiler
{
public:
    Compiler();
    ~Compiler() = default;

    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    Compiler(Compiler&&) = delete;
    Compiler& operator=(Compiler&&) = delete;

    /*!
     * \brief Compile \a source to bytecode stored in \a chunk.
     *
     * \param source Lox source text.
     * \param chunk Chunk object used to store the bytecode representation of
     *              \a source.
     * \return \c true if \a source compiles without error.
     */
    bool Compile(const std::string& source,
                 std::shared_ptr<Chunk> chunk);

private:
    using ParseFn = std::function<void(Compiler*)>;

    /*!
     * \enum Precedence
     * \brief The Precedence emum defines token precedence.
     */
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

    /*!
     * \struct Parser
     * \brief The Parser struct tracks parse state by Token and error info.
     */
    struct Parser
    {
        Token current;    /*!< Current token being processed by the parser. */
        Token previous;   /*!< Previous Token scanned by the parser. */
        bool  had_error;  /*!< Flag indicating an error has occurred. */
        bool  panic_mode; /*!< Flag for handling cascading errors. */
    }; // end Parser

    /*!
     * \struct ParseRule
     * \brief The ParseRule struct handles lookup into the parser action table.
     */
    struct ParseRule
    {
        ParseFn    prefix;     /*!< Prefix parse function. */
        ParseFn    infix;      /*!< Infix parse function. */
        Precedence precedence; /*!< Parse rule precedence */
    }; // end ParseRule

    static std::unordered_map<Token::TokenType, ParseRule> rules_; /*!< Lookup table mapping TokenType to a corresponding ParseRule. */

    void ParsePrecedence(Precedence precedence);

    void Expression()
        { ParsePrecedence(Precedence::kPrecAssignment); }

    /*!
     * \brief Advance the parser to the next Token.
     */
    void Advance();

    /*!
     * \brief Consume the current Token if its type matches \a type.
     *
     * If there is a type mismatch between \a type and the TokenType of the
     * Token currently being processed by the parser, ErrorAt() is called
     * with message \a message.
     */
    void Consume(Token::TokenType type, const std::string& message);

    /*!
     * \brief Compile value into a bytecode constant.
     * \return The index of \a value in #chunk_'s constant array.
     */
    uint8_t MakeConstant(const val::Value& value);

    /*!
     * \brief Print error info to STDOUT.
     */
    void ErrorAt(const Token& error, const std::string& message);

    /*!
     * \brief Print error info for the current token.
     */
    void ErrorAtCurrent(const std::string& message)
        { ErrorAt(parser_.current, message); }

    /*!
     * \brief Print error info for the previous token.
     */
    void Error(const std::string& message)
        { ErrorAt(parser_.previous, message); }

    /*!
     * \brief Write \a byte to #chunk_.
     */
    void EmitByte(uint8_t byte)
        { chunk_->Write(byte, parser_.previous.GetLine()); }

    /*!
     * \brief Write \a byte1 and \a byte2 in sequence to #chunk_.
     */
    void EmitBytes(uint8_t byte1, uint8_t byte2);

    /*!
     * \brief Write a return instruction to #chunk_.
     */
    void EmitReturn()
        { EmitByte(Chunk::OpCode::kOpReturn); }

    /*!
     * \brief Wrtie a constant instruction to #chunk_.
     */
    void EmitConstant(const val::Value& value)
        { EmitBytes(Chunk::OpCode::kOpConstant, MakeConstant(value)); }

    /*!
     * \brief End compilation.
     */
    void EndCompiler();

    /*!
     * \brief Compile a number.
     */
    void Number();

    /*!
     * \brief Consume grouping tokens and compile the inner expression.
     */
    void Grouping();

    /*!
     * \brief Compile a unary expression.
     */
    void Unary();

    /*!
     * \brief Compile a binary expression.
     */
    void Binary();

    /*!
     * \brief Compile a literal expression.
     */
    void Literal();

    /*!
     * \brief Compile a string literal.
     */
    void String();

    Scanner                scanner_; /*!< Token scanner. */
    std::shared_ptr<Chunk> chunk_;   /*!< Chunk storing compiled bytecode. */
    Parser                 parser_;  /*!< Handle to the Parser. */
}; // end Compiler
} // end lox
