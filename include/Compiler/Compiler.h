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
    using InternedStrings = std::shared_ptr<std::unordered_map<std::string,
                                            std::shared_ptr<obj::ObjString>>>;

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
    std::shared_ptr<obj::ObjFunction>
    Compile(const std::string& source, InternedStrings strings);

private:
    using Token     = lox::scanr::Token;
    using TokenType = lox::scanr::Token::TokenType;
    using ParseFn   = std::function<void(Compiler*, bool)>;

    /*!
     * \enum Precedence
     * \brief The Precedence emum defines token precedence.
     */
    enum Precedence
    {
        kPrecNone,       /*!< Lowest or no precedence. */
        kPrecAssignment, /*!< = */
        kPrecOr,         /*!< or */
        kPrecAnd,        /*!< and */
        kPrecEquality,   /*!< == != */
        kPrecComparison, /*!< < > <= >= */
        kPrecTerm,       /*!< + - */
        kPrecFactor,     /*!< * / */
        kPrecUnary,      /*!< ! - */
        kPrecCall,       /*!< . () */
        kPrecPrimary     /*!< Primary expression precedence. */
    }; // end Precedence

    /*!
     * \struct Parser
     * \brief The Parser struct tracks parse state by Token.
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
        bool  is_captured;
    }; // end Local

    /*!
     * \struct Upvalue
     * \brief The Upvalue struct represents a closure upvalue.
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
        kTypeFunction,   /*!< User defined function. */
        kTypeScript,     /*!< Pseudo function representing the top level script. */
        kTypeMethod,     /*!< Class method. */
        kTypeInitializer /*!< Class initializer method. */
    }; // end FunctionType

    /*!
     * \struct CompilerData
     * \brief The CompilerData struct tracks local variable info.
     */
    struct CompilerData
    {
        std::shared_ptr<CompilerData>     enclosing; /*!< Metadata of the next compiler on the compiler stack. */
        std::shared_ptr<obj::ObjFunction> function;  /*!< Function being compiled. */
        FunctionType type;                    /*!< FunctionType of #function. */
        Local        locals[UINT8_MAX +1];    /*!< Array of local variable data. */
        int          local_count;             /*!< Length of locals array. */
        int          scope_depth;             /*!< Active scope depth (global=0). */
        Upvalue      upvalues[UINT8_MAX + 1]; /*!< Array of closure upvalues. */
    }; // end CompilerData

    using CompilerDataPtr = std::shared_ptr<CompilerData>;
    /*!
     * \struct ClassCompiler
     * \brief The ClassCompiler struct captures the nearest enclosing class.
     */
    struct ClassCompiler
    {
        ClassCompiler* enclosing;      /*!< Pointer the enclosing class. */
        bool           has_superclass; /*!< Superclass flag. */
    }; // end ClassCompiler

    static std::unordered_map<TokenType, ParseRule>
    rules_; /*!< Lookup table mapping TokenType to a ParseRule. */

    /*!
     * \brief Initialize \a compiler.
     *
     * InitCompiler() enables us to simulate a stack of CompilerData structs.
     * Each Lox function call creates a new CompilerData instance.
     *
     * \param enclosing Pointer to the CompilerData struct at the top of the
     *                  CompilerData stack.
     * \param compiler  Pointer to the CompilerData struct being added to the
     *                  top of the stack.
     * \param type      Type of the function being compiled.
     */
    void
    InitCompiler(CompilerDataPtr enclosing,
                 CompilerDataPtr compiler,
                 FunctionType type);

    /*!
     * \brief Return a reference to the Chunk of the function being compiled.
     */
    Chunk&
    CurrentChunk() { return current_->function->chunk; }

    /*!
     * \brief Parse statements at the current precedence level or higher.
     */
    void
    ParsePrecedence(Precedence precedence);

    /*!
     * \brief Parse a variable.
     */
    uint8_t
    ParseVariable(const std::string& error_message);

    /*!
     * \brief Emit bytecode for a variable definition.
     */
    void
    DefineVariable(uint8_t global);

    /*!
     * \brief Compile a Lox statement.
     */
    void
    Statement();

    /*!
     * \brief Compile an if statement.
     */
    void
    IfStatement();

    /*!
     * \brief Compile a while statement.
     */
    void
    WhileStatement();

    /*!
     * \brief Compile a for statement.
     */
    void
    ForStatement();

    /*!
     * \brief Replace the operand of the jump instruction at \a offset.
     *
     * A jump instruction takes an operand telling it where to jump.
     * PatchJump() is used to patch the jump instruction's operand at
     * \a offset. The operand is updated using the current location in
     * the code.
     */
    void
    PatchJump(int offset);

    /*!
     * \brief Compile a Lox print statement.
     */
    void
    PrintStatement();

    /*!
     * \brief Compile an expression statement.
     */
    void
    ExpressionStatement();

    /*!
     * \brief Compile a return statement.
     */
    void
    ReturnStatement();

    /*!
     * \brief Top level rule for compiling declarations.
     */
    void
    Declaration();

    /*!
     * \brief Compile a variable declaration.
     */
    void
    VarDeclaration();

    /*!
     * \brief Compile a function declaration.
     */
    void
    FunDeclaration();

    /*!
     * \brief Compile a class declaration.
     */
    void
    ClassDeclaration();

    /*!
     * \brief Compile a class method.
     */
    void
    Method();

    /*!
     * \brief Return \c true if \a a and \a b have identical lexemes.
     */
    bool
    IdentifiersEqual(const Token& a, const Token& b) const
        { return (a.GetLexeme() == b.GetLexeme()); }

    /*!
     * \brief Adds a new local variable with name \a name to the current scope.
     */
    void
    AddLocal(const Token& name);

    /*!
     * \brief Mark a local variable as initialized.
     */
    void
    MarkInitialized();

    /*!
     * \brief Attempt to resolve a local variable by name.
     * \return The index of local variable's value in the stack is returned
     *         if \a name could be resolved. -1 is returned if the variable
     *         could not be resolved.
     */
    int
    ResolveLocal(CompilerDataPtr compiler, const Token& name);

    /*!
     * \brief Resolve the upvalue referenced by \a name.
     * \return The index of the upvalue in the stack is returned if \a name
     *         can be resolved in the one of the enclosing scopes. -1 is
     *         returned if the upvalue could not be resolved.
     */
    int
    ResolveUpvalue(CompilerDataPtr compiler, const Token& name);

    /*!
     * \brief Add an upvalue to the parameter compiler's upvalue array.
     */
    int
    AddUpvalue(CompilerDataPtr compiler, uint8_t index, bool is_local);

    /*!
     * \brief Declare a variable (local or global).
     */
    void
    DeclareVariable();

    /*!
     * \brief Method called by the parser to handle expressions.
     */
    void
    Expression() { ParsePrecedence(Precedence::kPrecAssignment); }

    /*!
     * \brief Open a new block or scope.
     */
    void
    BeginScope() { current_->scope_depth++; }

    /*!
     * \brief Teardown a block/scope.
     */
    void
    EndScope();

    /*!
     * \brief Compile a code block.
     */
    void
    Block();

    /*!
     * \brief Compile a function.
     */
    void
    Function(FunctionType type);

    /*!
     * \brief Compile a function or method's argument list.
     */
    uint8_t
    ArgumentList();

    /*!
     * \brief Advance the parser to the next Token.
     */
    void
    Advance();

    /*!
     * \brief Return \c true if the #parser_'s current token type equals \a type.
     */
    bool
    Check(TokenType type) const
        { return (parser_.current.GetType() == type); }

    /*!
     * \brief Return \c true if \a type matches the current parse token type.
     *
     * Match() has the side effect of advancing to the parser to the next token
     * via a call to Advance() if the types match.
     */
    bool
    Match(TokenType type);

    /*!
     * \brief Compile the identifier represented by \a name.
     */
    uint8_t
    IdentifierConstant(const Token& name);

    /*!
     * \brief Consume the current Token if its type matches \a type.
     *
     * If there is a type mismatch between \a type and the TokenType of the
     * Token currently being processed by the parser, ErrorAt() is called
     * with message \a message.
     */
    void
    Consume(TokenType type, const std::string& message);

    /*!
     * \brief Compile \a value into a bytecode constant.
     * \return The index of \a value in the current Chunk's constant array.
     */
    uint8_t
    MakeConstant(const val::Value& value);

    /*!
     * \brief Synchronize the parser on the next statement following an error.
     */
    void
    Synchronize();

    /*!
     * \brief Print error info to STDERR.
     */
    void
    ErrorAt(const Token& error, const std::string& message);

    /*!
     * \brief Print error info for the current token.
     */
    void
    ErrorAtCurrent(const std::string& message)
        { ErrorAt(parser_.current, message); }

    /*!
     * \brief Print error info for the previous token.
     */
    void
    Error(const std::string& message) { ErrorAt(parser_.previous, message); }

    /*!
     * \brief Write \a byte to the current Chunk.
     */
    void
    EmitByte(uint8_t byte)
        { CurrentChunk().Write(byte, parser_.previous.GetLine()); }

    /*!
     * \brief Write \a byte1 and \a byte2 in sequence to the current Chunk.
     */
    void
    EmitBytes(uint8_t byte1, uint8_t byte2);

    /*!
     * \brief Write a return instruction to the current Chunk.
     */
    void
    EmitReturn();

    /*!
     * \brief Write a jump instruction to the current Chunk.
     */
    int
    EmitJump(uint8_t instruction);

    /*!
     * \brief Write a loop instruction to the current Chunk.
     */
    void
    EmitLoop(int loop_start);

    /*!
     * \brief Write a constant instruction to the current Chunk.
     */
    void
    EmitConstant(const val::Value& value)
        { EmitBytes(Chunk::OpCode::kOpConstant, MakeConstant(value)); }

    /*!
     * \brief End compilation.
     */
    std::shared_ptr<obj::ObjFunction>
    EndCompiler();

    /* Parser action functions. */
    void
    Number([[maybe_unused]]bool can_assign);

    void
    Grouping([[maybe_unused]]bool can_assign);

    void
    Unary([[maybe_unused]]bool can_assign);

    void
    Binary([[maybe_unused]]bool can_assign);

    void
    Literal([[maybe_unused]]bool can_assign);

    void
    String([[maybe_unused]]bool can_assign);

    void
    And([[maybe_unused]]bool can_assign);

    void
    Or([[maybe_unused]]bool can_assign);

    void
    Variable(bool can_assign) { NamedVariable(parser_.previous, can_assign); }

    void
    NamedVariable(const Token& name, bool can_assign);

    void
    Call([[maybe_unused]]bool can_assign);

    void
    Dot(bool can_assign);

    void
    This([[maybe_unused]]bool can_assign);

    void
    Super([[maybe_unused]]bool can_assign);

    lox::scanr::Scanner scanner_;       /*!< Token scanner. */
    Parser              parser_;        /*!< Handle to the Parser. */
    InternedStrings     interned_strs_; /*!< Collection of interned strings. */
    CompilerDataPtr     current_;       /*!< Compiler metadata. */
    ClassCompiler*      current_class_; /*!< Current class under compilation. */
}; // end Compiler
} // end cl
} // end lox
