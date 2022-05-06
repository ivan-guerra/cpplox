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
namespace cl
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
     * \brief Compile \a source code to bytecode.
     *
     * \param source Lox source text.
     * \param strings Pointer to a map containing all interned strings.
     * \return A pointer to the compiled Lox function object.
     */
    std::shared_ptr<obj::ObjFunction> Compile(
        const std::string& source,
        InternedStrings strings);

private:
    using Token     = lox::scanr::Token;
    using TokenType = lox::scanr::Token::TokenType;
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

    using ParseFn = std::function<void(Compiler*, bool)>;
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
        bool  is_captured;
    }; // end Local

    /*!
     * \struct Upvalue
     * \brief The Upvalue represents a closure upvalue.
     *
     * Closure upvalues are local variables refrenced by the closure that are
     * defined in one of the enclosing functions or global scope.
     */
    struct Upvalue
    {
        uint8_t index;    /*!< Tracks the closed-over local variables's slot index. */
        bool    is_local; /*!< Tracks whether this upvalue was resolved locally. */
    }; // end Upvalue

    /*!
     * \enum FunctionType
     * \brief The FunctionType enum defines the type of a Lox function object.
     */
    enum FunctionType
    {
        kTypeFunction, /*!< User defined function. */
        kTypeScript    /*!< Indicates a pseudo function representing the top level script. */
    }; // end FunctionType

    /*!
     * \struct CompilerData
     * \brief The CompilerData struct tracks local variable info.
     */
    struct CompilerData
    {
        std::shared_ptr<CompilerData> enclosing; /*!< Metadata of the next compiler on the compiler stack. */
        std::shared_ptr<obj::ObjFunction> function; /*!< Function being compiled. */
        FunctionType type;                    /*!< FunctionType of #function. */
        Local        locals[UINT8_MAX +1];    /*!< Array of local variable data. */
        int          local_count;             /*!< Length of locals array. */
        int          scope_depth;             /*!< Active scope depth (global=0). */
        Upvalue      upvalues[UINT8_MAX + 1]; /*!< Array of closure upvalues. */
    }; // end CompilerData
    using CompilerDataPtr = std::shared_ptr<CompilerData>;

    static std::unordered_map<TokenType, ParseRule> rules_; /*!< Lookup table mapping TokenType to a corresponding ParseRule. */

    /*!
     * \brief Initialize #compiler_.
     *
     * InitCompiler() enables us to simulate a stack of CompilerData structs.
     * Each Lox function call creates a new CompilerData instance. That
     * CompilerData instance must point back to the current CompilerData
     * instance (i.e., whatever #compiler_ points to at the time InitCompiler()
     * is called). InitCompiler() sets \a compiler to point back to
     * \a enclosing. InitCompiler() also initializes the other fields of
     * \a compiler appropriately.
     *
     * \param enclosing Pointer to the CompilerData struct at the top of the
     *                  CompilerData stack.
     * \param compiler  Pointer to the CompilerData struct being added to the
     *                  top of the stack.
     * \param type Type of the function being compiled.
     */
    void InitCompiler(
        CompilerDataPtr enclosing,
        CompilerDataPtr compiler,
        FunctionType type);

    /*!
     * \brief Return a reference to the Chunk of the function being compiled.
     */
    Chunk& CurrentChunk()
        { return compiler_->function->chunk; }

    /*!
     * \brief Parse statements at the current precedence level or higher.
     */
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

    /*!
     * \brief Compile an if statement.
     */
    void IfStatement();

    /*!
     * \brief Compile a while statement.
     */
    void WhileStatement();

    /*!
     * \brief Compile a for statement.
     */
    void ForStatement();

    /*!
     * \brief Replace the operand of the jump instruction at \a offset.
     *
     * A jump instruction takes an operand telling it where to jump.
     * PatchJump() is used to patch the jump instruction's operand at
     * \a offset. The operand is updated using the current location in
     * the code.
     */
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
     * \brief Compile a return statement.
     */
    void ReturnStatement();

    /*!
     * \brief Top level parse rule for parsing declarations.
     */
    void Declaration();

    /*!
     * \brief Parse any variable declaration.
     */
    void VarDeclaration();

    /*!
     * \brief Parse a function declaration.
     */
    void FunDeclaration();

    /*!
     * \brief Method called by the parser to handle variables.
     *
     * \param can_assign Flag indicating whether the variable in question
     *                   can be assigned to.
     */
    void Variable(bool can_assign)
        { NamedVariable(parser_.previous, can_assign); }

    /*!
     * \brief Return \c true if \a a and \a b have identical lexemes.
     */
    bool IdentifiersEqual(const Token& a, const Token& b) const
        { return (a.GetLexeme() == b.GetLexeme()); }

    /*!
     * \brief Adds a new local variable with name \a name to the current scope.
     */
    void AddLocal(const Token& name);

    /*!
     * \brief Mark a local variable as initialized.
     */
    void MarkInitialized();

    /*!
     * \brief Attempt to resolve a local variable by name.
     * \return The index of local variable's value in the stack is returned
     *         if \a name could be resolved. -1 is returned if the variable
     *         could not be resolved.
     */
    int ResolveLocal(CompilerDataPtr compiler, const Token& name);

    /*!
     * \brief Attempt to resolve the upvalue referenced by \a name.
     * \return The index of the upvalue in the stack is returned if \a name
     *         can be resolved in the one of the enclosing scopes. -1 is
     *         returned if the upvalue could not be resolved.
     */
    int ResolveUpvalue(CompilerDataPtr compiler, const Token& name);

    /*!
     * \brief Add an upvalue to the parameter compiler's upvalue array.
     */
    int AddUpvalue(CompilerDataPtr compiler, uint8_t index, bool is_local);

    /*!
     * \brief Declare a variable (local or global).
     */
    void DeclareVariable();

    /*!
     * \brief Method called by the parser to handle expressions.
     */
    void Expression()
        { ParsePrecedence(Precedence::kPrecAssignment); }

    /*!
     * \brief Open a new block or scope.
     */
    void BeginScope()
        { compiler_->scope_depth++; }

    /*!
     * \brief Teardown a block/scope.
     */
    void EndScope();

    /*!
     * \brief Compile a code block.
     */
    void Block();

    /*!
     * \brief Compile a function.
     */
    void Function(FunctionType type);

    /*!
     * \brief Compile a function or method's argument list.
     */
    uint8_t ArgumentList();

    /*!
     * \brief Advance the parser to the next Token.
     */
    void Advance();

    /*!
     * \brief Return \c true if the #parser_'s current token type equals \a type.
     */
    bool Check(TokenType type) const
        { return (parser_.current.GetType() == type); }

    /*!
     * \brief Return \c true if \a type matches the #parser_'s current token type.
     *
     * Match() has the side effect of advancing to the parser to the next token
     * via a call to Advance() if the types match.
     */
    bool Match(TokenType type);

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
    void Consume(TokenType type, const std::string& message);

    /*!
     * \brief Compile value into a bytecode constant.
     * \return The index of \a value in the current Chunk's constant array.
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
     * \brief Write \a byte to the current Chunk.
     */
    void EmitByte(uint8_t byte)
        { CurrentChunk().Write(byte, parser_.previous.GetLine()); }

    /*!
     * \brief Write \a byte1 and \a byte2 in sequence to the current Chunk.
     */
    void EmitBytes(uint8_t byte1, uint8_t byte2);

    /*!
     * \brief Write a return instruction to the current Chunk.
     */
    void EmitReturn();

    /*!
     * \brief Write a jump instruction to the current Chunk.
     */
    int EmitJump(uint8_t instruction);

    /*!
     * \brief Write a loop instruction to the current Chunk.
     */
    void EmitLoop(int loop_start);

    /*!
     * \brief Write a constant instruction to the current Chunk.
     */
    void EmitConstant(const val::Value& value)
        { EmitBytes(Chunk::OpCode::kOpConstant, MakeConstant(value)); }

    /*!
     * \brief End compilation.
     */
    std::shared_ptr<obj::ObjFunction> EndCompiler();

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

    /*!
     * \brief Compile a logical and statement.
     */
    void And([[maybe_unused]]bool can_assign);

    /*!
     * \brief Compile a logical or statement.
     */
    void Or([[maybe_unused]]bool can_assign);

    /*!
     * \brief Compile a function, method, etc. call.
     */
    void Call([[maybe_unused]]bool can_assign);

    lox::scanr::Scanner scanner_;  /*!< Token scanner. */
    Parser              parser_;   /*!< Handle to the Parser. */
    InternedStrings     strings_;  /*!< Collection of interned strings. */
    CompilerDataPtr     compiler_; /*!< Compiler metadata list. */
}; // end Compiler
} // end cl
} // end lox
