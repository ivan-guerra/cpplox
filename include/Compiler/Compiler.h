#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

#include "Value.h"
#include "Object.h"
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
    using InternedStrings = std::shared_ptr<
        std::unordered_map<std::string, std::shared_ptr<obj::ObjString>>>;

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
     * \param strings Pointer to a map containing all interned strings.
     * \return \c true if \a source compiles without error.
     */
    bool Compile(const std::string& source,
                 std::shared_ptr<Chunk> chunk,
                 InternedStrings strings);

private:
    using ParseFn = std::function<void(Compiler*, bool)>;

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

    /*!
     * \struct Local
     * \brief The Local struct wraps local variable name and stack data.
     */
    struct Local
    {
        Token name;
        int   depth;
    }; // end Local

    /*!
     * \struct CompilerData
     * \brief The CompilerData struct tracks local variable info.
     */
    struct CompilerData
    {
        Local locals[UINT8_MAX + 1];
        int   local_count;
        int   scope_depth;
    }; // end CompilerData

    static std::unordered_map<Token::TokenType, ParseRule> rules_; /*!< Lookup table mapping TokenType to a corresponding ParseRule. */

    void ParsePrecedence(Precedence precedence);

    /*!
     * \brief Compile a variable statement.
     */
    uint8_t ParseVariable(const std::string& error_message);

    /*!
     * \brief Emit bytecode for a variable definition.
     */
    void DefineVariable(uint8_t global);

    /*!
     * \brief Parse a Lox statement.
     */
    void Statement();

    void IfStatement();

    void WhileStatement();

    void ForStatement();

    void PatchJump(int offset);

    /*!
     * \brief Compile a Lox print statement.
     */
    void PrintStatement();

    /*!
     * \brief Compile an expression statement.
     */
    void ExpressionStatement();

    /*!
     * \brief Top level parse rule for parsing declarations.
     */
    void Declaration();

    /*!
     * \brief Parse any variable declaration.
     */
    void VarDeclaration();

    /*!
     * \brief Method called by the parser to handle variables.
     *
     * \param can_assign Flag indicating whether the variable in question
     *                   can be assigned to.
     */
    void Variable(bool can_assign)
        { NamedVariable(parser_.previous, can_assign); }

    bool IdentifiersEqual(const Token& a, const Token& b) const
        { return (a.GetLexeme() == b.GetLexeme()); }

    void AddLocal(const Token& name);

    void DeclareVariable();

    /*!
     * \brief Method called by the parser to handle expressions.
     */
    void Expression()
        { ParsePrecedence(Precedence::kPrecAssignment); }

    /*!
     * \brief Mark a local variable as initialized.
     */
    void MarkInitialized()
        { compiler_.locals[compiler_.local_count - 1].depth = compiler_.scope_depth; }

    /*!
     * \brief Attempt to resolve a local variable by name.
     * \return The index of local variable's value in the stack is returned
     *         if \a name could be resolved. -1 is returned if the variable
     *         could not be resolved.
     */
    int ResolveLocal(const Token& name);

    /*!
     * \brief Open a new block or scope.
     */
    void BeginScope()
        { compiler_.scope_depth++; }

    /*!
     * \brief Teardown a block/scope.
     */
    void EndScope();

    /*!
     * \brief Compile a code block.
     */
    void Block();

    /*!
     * \brief Advance the parser to the next Token.
     */
    void Advance();

    /*!
     * \brief Return \c true if the #parser_'s current token type equals \a type.
     */
    bool Check(Token::TokenType type) const
        { return (parser_.current.GetType() == type); }

    /*!
     * \brief Return \c true if \a type matches the #parser_'s current token type.
     *
     * Match() has the side effect of advancing to the parser to the next token
     * via a call to Advance() if the types match.
     */
    bool Match(Token::TokenType type);

    /*!
     * \brief Compile the identifier represented by \a name.
     */
    uint8_t IdentifierConstant(const Token& name);

    /*!
     * \brief Reference or assign to a named variable.
     */
    void NamedVariable(const Token& name, bool can_assign);

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
     * \brief Synchronize the parser on the next statement following an error.
     */
    void Synchronize();

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

    int EmitJump(uint8_t instruction);

    void EmitLoop(int loop_start);

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
    void Number([[maybe_unused]]bool can_assign);

    /*!
     * \brief Consume grouping tokens and compile the inner expression.
     */
    void Grouping([[maybe_unused]]bool can_assign);

    /*!
     * \brief Compile a unary expression.
     */
    void Unary([[maybe_unused]]bool can_assign);

    /*!
     * \brief Compile a binary expression.
     */
    void Binary([[maybe_unused]]bool can_assign);

    /*!
     * \brief Compile a literal expression.
     */
    void Literal([[maybe_unused]]bool can_assign);

    /*!
     * \brief Compile a string literal.
     */
    void String([[maybe_unused]]bool can_assign);

    void And([[maybe_unused]]bool can_assign);

    void Or([[maybe_unused]]bool can_assign);

    Scanner                scanner_;  /*!< Token scanner. */
    std::shared_ptr<Chunk> chunk_;    /*!< Chunk storing compiled bytecode. */
    Parser                 parser_;   /*!< Handle to the Parser. */
    InternedStrings        strings_;  /*!< Collection of interned strings. */
    CompilerData           compiler_; /*!< Compiler local var data. */
}; // end Compiler
} // end lox
