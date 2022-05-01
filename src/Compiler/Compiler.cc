#include <cstdio>
#include <climits>
#include <string>
#include <memory>

#include "Object.h"
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
        {&Compiler::Unary, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kBangEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecEquality}},
    {Token::TokenType::kEqual,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kEqualEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecEquality}},
    {Token::TokenType::kGreater,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {Token::TokenType::kGreaterEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {Token::TokenType::kLess,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {Token::TokenType::kLessEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {Token::TokenType::kIdentifier,
        {&Compiler::Variable, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kString,
        {&Compiler::String, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kNumber,
        {&Compiler::Number, nullptr,  Precedence::kPrecNone}},
    {Token::TokenType::kAnd,
        {nullptr, &Compiler::And, Precedence::kPrecAnd}},
    {Token::TokenType::kClass,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kElse,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kFalse,
        {&Compiler::Literal, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kFun,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kFor,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kIf,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kNil,
        {&Compiler::Literal, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kOr,
        {nullptr, &Compiler::Or, Precedence::kPrecOr}},
    {Token::TokenType::kPrint,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kReturn,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kSuper,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kThis,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kTrue,
        {&Compiler::Literal, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kVar,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kWhile,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kError,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {Token::TokenType::kEof,
        {nullptr, nullptr, Precedence::kPrecNone}}
};


void Compiler::InitCompiler(FunctionType type)
{
    compiler_.function        = obj::NewFunction();
    compiler_.type            = type;
    compiler_.locals[0].depth = 0;
    compiler_.local_count     = 1;
    compiler_.scope_depth     = 0;
}

void Compiler::ParsePrecedence(Precedence precedence)
{
    Advance();

    ParseFn prefix_rule = rules_[parser_.previous.GetType()].prefix;
    if (!prefix_rule) {
        Error("Expect expression.");
        return;
    }
    bool can_assign = precedence <= Precedence::kPrecAssignment;
    prefix_rule(this, can_assign);

    while (precedence <= rules_[parser_.current.GetType()].precedence) {
        Advance();
        ParseFn infix_rule = rules_[parser_.previous.GetType()].infix;
        infix_rule(this, can_assign);
    }

    if (can_assign && Match(Token::TokenType::kEqual))
        Error("Invalid assignment target.");
}

uint8_t Compiler::ParseVariable(const std::string& error_message)
{
    Consume(Token::TokenType::kIdentifier, error_message);

    DeclareVariable();
    if (compiler_.scope_depth > 0)
        return 0;

    return IdentifierConstant(parser_.previous);
}

void Compiler::DefineVariable(uint8_t global)
{
    if (compiler_.scope_depth > 0) {
        MarkInitialized();
        return;
    }

    EmitBytes(Chunk::OpCode::kOpDefineGlobal, global);
}

void Compiler::Statement()
{
    if (Match(Token::TokenType::kPrint)) {
        PrintStatement();
    } else if (Match(Token::TokenType::kLeftBrace)) {
        BeginScope();
        Block();
        EndScope();
    } else if (Match(Token::TokenType::kIf)) {
        IfStatement();
    } else if (Match(Token::TokenType::kWhile)) {
        WhileStatement();
    } else if (Match(Token::TokenType::kFor)) {
        ForStatement();
    } else {
        ExpressionStatement();
    }
}

void Compiler::IfStatement()
{
    Consume(Token::TokenType::kLeftParen, "Expect '(' after if.");
    Expression();
    Consume(Token::TokenType::kRightParen, "Expect ')' after condition.");

    int then_jump = EmitJump(Chunk::OpCode::kOpJumpIfFalse);
    EmitByte(Chunk::OpCode::kOpPop);

    Statement();

    int else_jump = EmitJump(Chunk::OpCode::kOpJump);

    PatchJump(then_jump);
    EmitByte(Chunk::OpCode::kOpPop);

    if (Match(Token::TokenType::kElse))
        Statement();

    PatchJump(else_jump);
}

void Compiler::WhileStatement()
{
    int loop_start = CurrentChunk().GetCode().size();
    Consume(Token::TokenType::kLeftParen, "Expect '(' after 'while'.");
    Expression();
    Consume(Token::TokenType::kRightParen, "Expect ')' after condition.");

    int exit_jump = EmitJump(Chunk::OpCode::kOpJumpIfFalse);
    EmitByte(Chunk::OpCode::kOpPop);
    Statement();
    EmitLoop(loop_start);

    PatchJump(exit_jump);
    EmitByte(Chunk::OpCode::kOpPop);
}

void Compiler::ForStatement()
{
    BeginScope();
    Consume(Token::TokenType::kLeftParen, "Expect '(' after 'for'.");

    if (Match(Token::TokenType::kSemicolon)) {
        /* No initializer. */
    } else if (Match(Token::TokenType::kVar)) {
        VarDeclaration();
    } else {
        ExpressionStatement();
    }

    int loop_start = CurrentChunk().GetCode().size();
    int exit_jump  = -1;
    if (!Match(Token::TokenType::kSemicolon)) {
        Expression();
        Consume(Token::TokenType::kSemicolon,
                "Expect ';' after loop condition.");

        exit_jump = EmitJump(Chunk::OpCode::kOpJumpIfFalse);
        EmitByte(Chunk::OpCode::kOpPop);
    }

    if (!Match(Token::TokenType::kRightParen)) {
        int body_jump       = EmitJump(Chunk::OpCode::kOpJump);
        int increment_start = CurrentChunk().GetCode().size();
        Expression();
        EmitByte(Chunk::OpCode::kOpPop);
        Consume(Token::TokenType::kRightParen,
                "Expect ')' after for clauses.");

        EmitLoop(loop_start);
        loop_start = increment_start;
        PatchJump(body_jump);
    }

    Statement();
    EmitLoop(loop_start);

    if (exit_jump != -1) {
        PatchJump(exit_jump);
        EmitByte(Chunk::OpCode::kOpPop);
    }
    EndScope();
}

void Compiler::PatchJump(int offset)
{
    int jump = static_cast<int>(CurrentChunk().GetCode().size()) - offset - 2;
    if (jump > UINT16_MAX)
        Error("Too much code to jump over.");

    CurrentChunk().SetInstruction(offset, (jump >> 8) & 0xFF);
    CurrentChunk().SetInstruction(offset + 1, jump & 0xff);
}

void Compiler::PrintStatement()
{
    Expression();
    Consume(Token::TokenType::kSemicolon,
            "Expect ';' after value");
    EmitByte(Chunk::OpCode::kOpPrint);
}

void Compiler::ExpressionStatement()
{
    Expression();
    Consume(Token::TokenType::kSemicolon,
            "Expect ';' after expression.");
    EmitByte(Chunk::OpCode::kOpPop);
}

void Compiler::Declaration()
{
    if (Match(Token::TokenType::kVar))
        VarDeclaration();
    else
        Statement();

    if (parser_.panic_mode)
        Synchronize();
}

void Compiler::VarDeclaration()
{
    uint8_t global = ParseVariable("Expect variable name.");
    if (Match(Token::TokenType::kEqual))
        Expression();
    else
        EmitByte(Chunk::OpCode::kOpNil);

    Consume(Token::TokenType::kSemicolon,
            "Expect ';' after variable declaration.");

    DefineVariable(global);
}

void Compiler::AddLocal(const Token& name)
{
    if (compiler_.local_count == (UINT8_MAX + 1)) {
        Error("Too many local variables in function.");
        return;
    }

    Local local = {.name=name, .depth=-1};
    compiler_.locals[compiler_.local_count] = local;
    compiler_.local_count++;
}

void Compiler::DeclareVariable()
{
    if (compiler_.scope_depth == 0)
        return;

    Token name = parser_.previous;
    for (int i = compiler_.local_count - 1; i >= 0; --i) {
        Local local = compiler_.locals[i];
        if ((local.depth != -1) && (local.depth < compiler_.scope_depth))
            break;

        if (IdentifiersEqual(name, local.name))
            Error("Already a variable with this name in this scope.");
    }
    AddLocal(name);
}

int Compiler::ResolveLocal(const Token& name)
{
    for (int i = compiler_.local_count - 1; i >= 0; --i) {
        Local local = compiler_.locals[i];
        if (IdentifiersEqual(name, local.name)) {
            if (local.depth == -1)
                Error("Can't read local variable in its own intializer.");
            return i;
        }
    }
    return -1;
}

void Compiler::EndScope()
{
    compiler_.scope_depth--;

    while ((compiler_.local_count > 0) &&
           (compiler_.locals[compiler_.local_count - 1].depth >
            compiler_.scope_depth)) {
        EmitByte(Chunk::OpCode::kOpPop);
        compiler_.local_count--;
    }
}

void Compiler::Block()
{
    while (!Check(Token::TokenType::kRightBrace) &&
           !Check(Token::TokenType::kEof)) {
        Declaration();
    }
    Consume(Token::TokenType::kRightBrace, "Expect '}' after block.");
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

bool Compiler::Match(Token::TokenType type)
{
    if (!Check(type))
        return false;

    Advance();
    return true;
}

uint8_t Compiler::IdentifierConstant(const Token& name)
{
    return MakeConstant(obj::ObjVal(
                obj::CopyString(name.GetLexeme(), strings_)));
}

void Compiler::NamedVariable(const Token& name, bool can_assign)
{
    uint8_t get_op = 0;
    uint8_t set_op = 0;
    int arg = ResolveLocal(name);
    if (arg != -1) {
        get_op = Chunk::OpCode::kOpGetLocal;
        set_op = Chunk::OpCode::kOpSetLocal;
    } else {
        arg = IdentifierConstant(name);
        get_op = Chunk::OpCode::kOpGetGlobal;
        set_op = Chunk::OpCode::kOpSetGlobal;
    }

    if (can_assign && Match(Token::TokenType::kEqual)) {
        Expression();
        EmitBytes(set_op, arg);
    } else {
        EmitBytes(get_op, arg);
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

uint8_t Compiler::MakeConstant(const val::Value& value)
{
    int constant = CurrentChunk().AddConstant(value);
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

void Compiler::Synchronize()
{
    parser_.panic_mode = false;

    while (parser_.current.GetType() != Token::TokenType::kEof) {
        if (parser_.previous.GetType() ==
            Token::TokenType::kSemicolon)
            return;
        switch(parser_.current.GetType()) {
            case Token::TokenType::kClass:
            case Token::TokenType::kFun:
            case Token::TokenType::kVar:
            case Token::TokenType::kFor:
            case Token::TokenType::kIf:
            case Token::TokenType::kWhile:
            case Token::TokenType::kPrint:
            case Token::TokenType::kReturn:
                return;
            default:
                /* Do nothing. */
                ;
        }
        Advance();
    }
}

void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2)
{
    EmitByte(byte1);
    EmitByte(byte2);
}

int Compiler::EmitJump(uint8_t instruction)
{
    EmitByte(instruction);
    EmitByte(0xFF);
    EmitByte(0xFF);

    return (static_cast<int>(CurrentChunk().GetCode().size()) - 2);
}

void Compiler::EmitLoop(int loop_start)
{
    EmitByte(Chunk::OpCode::kOpLoop);

    int offset =
        static_cast<int>(CurrentChunk().GetCode().size()) - loop_start + 2;
    if (offset > UINT16_MAX)
        Error("Loop body too large.");

    EmitByte((offset >> 8) & 0xFF);
    EmitByte(offset & 0xFF);
}

std::shared_ptr<obj::ObjFunction> Compiler::EndCompiler()
{
    EmitReturn();
    std::shared_ptr<obj::ObjFunction> function = compiler_.function;
#ifdef DEBUG_PRINT_CODE
    if (!parser_.had_error) {
        CurrentChunk().Disassemble(
            function->name ? function->name->chars : "<script>");
    }
#endif
    return function;
}

void Compiler::Number([[maybe_unused]]bool can_assign)
{
    double value = std::stod(parser_.previous.GetLexeme());
    EmitConstant(val::NumberVal(value));
}

void Compiler::Grouping([[maybe_unused]]bool can_assign)
{
    Expression();
    Consume(Token::TokenType::kRightParen, "Expect ')' after expression.");
}

void Compiler::Unary([[maybe_unused]]bool can_assign)
{
    Token::TokenType operator_type = parser_.previous.GetType();

    /* Compile the operand. */
    ParsePrecedence(Precedence::kPrecUnary);

    /* Emit the operator instruction. */
    switch (operator_type) {
        case Token::TokenType::kBang:
            EmitByte(Chunk::OpCode::kOpNot);
            break;
        case Token::TokenType::kMinus:
            EmitByte(Chunk::OpCode::kOpNegate);
            break;
        default:
            /* Unreachable */
            return;
    }
}

void Compiler::Binary([[maybe_unused]]bool can_assign)
{
    Token::TokenType operator_type = parser_.previous.GetType();
    ParsePrecedence(
        static_cast<Precedence>(rules_[operator_type].precedence + 1));

    switch (operator_type) {
        case Token::TokenType::kBangEqual:
            EmitBytes(Chunk::OpCode::KOpEqual, Chunk::OpCode::kOpNot);
            break;
        case Token::TokenType::kEqualEqual:
            EmitByte(Chunk::OpCode::KOpEqual);
            break;
        case Token::TokenType::kGreater:
            EmitByte(Chunk::OpCode::kOpGreater);
            break;
        case Token::TokenType::kGreaterEqual:
            EmitBytes(Chunk::OpCode::kOpLess, Chunk::OpCode::kOpNot);
            break;
        case Token::TokenType::kLess:
            EmitByte(Chunk::OpCode::kOpLess);
            break;
        case Token::TokenType::kLessEqual:
            EmitBytes(Chunk::OpCode::kOpGreater, Chunk::OpCode::kOpNot);
            break;
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

void Compiler::Literal([[maybe_unused]]bool can_assign)
{
    switch (parser_.previous.GetType()) {
        case Token::TokenType::kFalse:
            EmitByte(Chunk::OpCode::kOpFalse);
            break;
        case Token::TokenType::kTrue:
            EmitByte(Chunk::OpCode::kOpTrue);
            break;
        case Token::TokenType::kNil:
            EmitByte(Chunk::OpCode::kOpNil);
            break;
        default:
            return;
    }
}

void Compiler::String([[maybe_unused]]bool can_assign)
{
    std::string lexeme = parser_.previous.GetLexeme();

    /* Trim off the '"' marks on either end of the lexeme before copying. */
    std::shared_ptr<obj::Obj> str_obj =
        obj::CopyString(lexeme.substr(1, lexeme.size() - 2), strings_);

    EmitConstant(obj::ObjVal(str_obj));
}

void Compiler::And([[maybe_unused]]bool can_assign)
{
    int end_jump = EmitJump(Chunk::OpCode::kOpJumpIfFalse);

    EmitByte(Chunk::OpCode::kOpPop);
    ParsePrecedence(Precedence::kPrecAnd);

    PatchJump(end_jump);
}

void Compiler::Or([[maybe_unused]]bool can_assign)
{
    int else_jump = EmitJump(Chunk::OpCode::kOpJumpIfFalse);
    int end_jump  = EmitJump(Chunk::OpCode::kOpJump);

    PatchJump(else_jump);
    EmitByte(Chunk::OpCode::kOpPop);

    ParsePrecedence(Precedence::kPrecOr);
    PatchJump(end_jump);
}

Compiler::Compiler() :
    scanner_("")
{
    parser_.had_error     = false;
    parser_.panic_mode    = false;

    InitCompiler(FunctionType::kTypeScript);
}

std::shared_ptr<obj::ObjFunction> Compiler::Compile(
    const std::string& source,
    InternedStrings strings)
{
    scanner_ = lox::Scanner(source);
    strings_ = strings;

    Advance();
    while (!Match(Token::TokenType::kEof))
        Declaration();

    std::shared_ptr<obj::ObjFunction> function = EndCompiler();
    return (parser_.had_error ? nullptr : function);
}
} // end lox
