#include <cstdio>
#include <climits>
#include <string>
#include <memory>

#include "Object.h"
#include "Scanner.h"
#include "Compiler.h"

namespace lox
{
namespace cl
{
std::unordered_map<lox::scanr::Token::TokenType, Compiler::ParseRule>
Compiler::rules_ =
{
    {TokenType::kLeftParen,
        {&Compiler::Grouping, &Compiler::Call, Precedence::kPrecCall}},
    {TokenType::kRightParen,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kLeftBrace,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kRightBrace,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kComma,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kDot,
        {nullptr, &Compiler::Dot, Precedence::kPrecCall}},
    {TokenType::kMinus,
        {&Compiler::Unary, &Compiler::Binary, Precedence::kPrecTerm}},
    {TokenType::kPlus,
        {nullptr, &Compiler::Binary, Precedence::kPrecTerm}},
    {TokenType::kSemicolon,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kSlash,
        {nullptr, &Compiler::Binary, Precedence::kPrecFactor}},
    {TokenType::kStar,
        {nullptr, &Compiler::Binary, Precedence::kPrecFactor}},
    {TokenType::kBang,
        {&Compiler::Unary, nullptr, Precedence::kPrecNone}},
    {TokenType::kBangEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecEquality}},
    {TokenType::kEqual,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kEqualEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecEquality}},
    {TokenType::kGreater,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {TokenType::kGreaterEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {TokenType::kLess,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {TokenType::kLessEqual,
        {nullptr, &Compiler::Binary, Precedence::kPrecComparison}},
    {TokenType::kIdentifier,
        {&Compiler::Variable, nullptr, Precedence::kPrecNone}},
    {TokenType::kString,
        {&Compiler::String, nullptr, Precedence::kPrecNone}},
    {TokenType::kNumber,
        {&Compiler::Number, nullptr,  Precedence::kPrecNone}},
    {TokenType::kAnd,
        {nullptr, &Compiler::And, Precedence::kPrecAnd}},
    {TokenType::kClass,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kElse,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kFalse,
        {&Compiler::Literal, nullptr, Precedence::kPrecNone}},
    {TokenType::kFun,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kFor,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kIf,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kNil,
        {&Compiler::Literal, nullptr, Precedence::kPrecNone}},
    {TokenType::kOr,
        {nullptr, &Compiler::Or, Precedence::kPrecOr}},
    {TokenType::kPrint,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kReturn,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kSuper,
        {&Compiler::Super, nullptr, Precedence::kPrecNone}},
    {TokenType::kThis,
        {&Compiler::This, nullptr, Precedence::kPrecNone}},
    {TokenType::kTrue,
        {&Compiler::Literal, nullptr, Precedence::kPrecNone}},
    {TokenType::kVar,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kWhile,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kError,
        {nullptr, nullptr, Precedence::kPrecNone}},
    {TokenType::kEof,
        {nullptr, nullptr, Precedence::kPrecNone}}
};

void Compiler::InitCompiler(
        CompilerDataPtr enclosing,
        CompilerDataPtr compiler,
        FunctionType type)
{
    compiler_ = compiler;

    compiler_->enclosing = enclosing;
    compiler_->function  = obj::NewFunction();
    compiler_->type      = type;
    if (type != FunctionType::kTypeScript) {
        compiler_->function->name =
            obj::CopyString(parser_.previous.GetLexeme(), strings_);
    }
    compiler_->locals[0].depth       = 0;
    compiler_->locals[0].is_captured = false;
    if (type != FunctionType::kTypeFunction) {
        compiler_->locals[0].name = scanr::Token(TokenType::kThis, "this", 0);
    }
    compiler_->local_count           = 1;
    compiler_->scope_depth           = 0;
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

    if (can_assign && Match(TokenType::kEqual))
        Error("Invalid assignment target.");
}

uint8_t Compiler::ParseVariable(const std::string& error_message)
{
    Consume(TokenType::kIdentifier, error_message);

    DeclareVariable();
    if (compiler_->scope_depth > 0)
        return 0;

    return IdentifierConstant(parser_.previous);
}

void Compiler::DefineVariable(uint8_t global)
{
    if (compiler_->scope_depth > 0) {
        MarkInitialized();
        return;
    }

    EmitBytes(Chunk::OpCode::kOpDefineGlobal, global);
}

void Compiler::Statement()
{
    if (Match(TokenType::kPrint)) {
        PrintStatement();
    } else if (Match(TokenType::kLeftBrace)) {
        BeginScope();
        Block();
        EndScope();
    } else if (Match(TokenType::kIf)) {
        IfStatement();
    } else if (Match(TokenType::kWhile)) {
        WhileStatement();
    } else if (Match(TokenType::kFor)) {
        ForStatement();
    } else if (Match(TokenType::kReturn)) {
        ReturnStatement();
    } else {
        ExpressionStatement();
    }
}

void Compiler::IfStatement()
{
    Consume(TokenType::kLeftParen, "Expect '(' after if.");
    Expression();
    Consume(TokenType::kRightParen, "Expect ')' after condition.");

    int then_jump = EmitJump(Chunk::OpCode::kOpJumpIfFalse);
    EmitByte(Chunk::OpCode::kOpPop);

    Statement();

    int else_jump = EmitJump(Chunk::OpCode::kOpJump);

    PatchJump(then_jump);
    EmitByte(Chunk::OpCode::kOpPop);

    if (Match(TokenType::kElse))
        Statement();

    PatchJump(else_jump);
}

void Compiler::WhileStatement()
{
    int loop_start = CurrentChunk().GetCode().size();
    Consume(TokenType::kLeftParen, "Expect '(' after 'while'.");
    Expression();
    Consume(TokenType::kRightParen, "Expect ')' after condition.");

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
    Consume(TokenType::kLeftParen, "Expect '(' after 'for'.");

    if (Match(TokenType::kSemicolon)) {
        /* No initializer. */
    } else if (Match(TokenType::kVar)) {
        VarDeclaration();
    } else {
        ExpressionStatement();
    }

    int loop_start = CurrentChunk().GetCode().size();
    int exit_jump  = -1;
    if (!Match(TokenType::kSemicolon)) {
        Expression();
        Consume(TokenType::kSemicolon,
                "Expect ';' after loop condition.");

        exit_jump = EmitJump(Chunk::OpCode::kOpJumpIfFalse);
        EmitByte(Chunk::OpCode::kOpPop);
    }

    if (!Match(TokenType::kRightParen)) {
        int body_jump       = EmitJump(Chunk::OpCode::kOpJump);
        int increment_start = CurrentChunk().GetCode().size();
        Expression();
        EmitByte(Chunk::OpCode::kOpPop);
        Consume(TokenType::kRightParen,
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
    Consume(TokenType::kSemicolon,
            "Expect ';' after value");
    EmitByte(Chunk::OpCode::kOpPrint);
}

void Compiler::ExpressionStatement()
{
    Expression();
    Consume(TokenType::kSemicolon,
            "Expect ';' after expression.");
    EmitByte(Chunk::OpCode::kOpPop);
}

void Compiler::ReturnStatement()
{
    if (compiler_->type == FunctionType::kTypeScript)
        Error("Can't return from top-level code.");

    if (Match(TokenType::kSemicolon)) {
        EmitReturn();
    } else {
        if (compiler_->type == FunctionType::kTypeInitializer)
            Error("Can't return a value from an initializer.");

        Expression();
        Consume(TokenType::kSemicolon, "Expect ';' after return value.");
        EmitByte(Chunk::OpCode::kOpReturn);
    }
}

void Compiler::Declaration()
{
    if (Match(TokenType::kFun))
        FunDeclaration();
    else if (Match(TokenType::kVar))
        VarDeclaration();
    else if (Match(TokenType::kClass))
        ClassDeclaration();
    else
        Statement();

    if (parser_.panic_mode)
        Synchronize();
}

void Compiler::VarDeclaration()
{
    uint8_t global = ParseVariable("Expect variable name.");
    if (Match(TokenType::kEqual))
        Expression();
    else
        EmitByte(Chunk::OpCode::kOpNil);

    Consume(TokenType::kSemicolon,
            "Expect ';' after variable declaration.");

    DefineVariable(global);
}

void Compiler::FunDeclaration()
{
    uint8_t global = ParseVariable("Expect function name.");
    MarkInitialized();
    Function(FunctionType::kTypeFunction);
    DefineVariable(global);
}

void Compiler::ClassDeclaration()
{
    Consume(TokenType::kIdentifier, "Expect class name.");
    Token   class_name    = parser_.previous;
    uint8_t name_constant = IdentifierConstant(parser_.previous);
    DeclareVariable();

    EmitBytes(Chunk::OpCode::kOpClass, name_constant);
    DefineVariable(name_constant);

    ClassCompiler class_compiler;
    class_compiler.enclosing      = current_class_;
    class_compiler.has_superclass = false;
    current_class_                = &class_compiler;

    if (Match(TokenType::kLess)) {
        Consume(TokenType::kIdentifier, "Expect superclass name.");
        Variable(false);
        if (IdentifiersEqual(class_name, parser_.previous))
            Error("A class can't inherit from itself.");

        BeginScope();
        AddLocal(scanr::Token(TokenType::kSuper, "super", 0));
        DefineVariable(0);

        NamedVariable(class_name, false);
        EmitByte(Chunk::OpCode::kOpInherit);
        class_compiler.has_superclass = true;
    }

    NamedVariable(class_name, false);
    Consume(TokenType::kLeftBrace, "Expect '{' before class body.");
    while (!Check(TokenType::kRightBrace) && !Check(TokenType::kEof))
        Method();
    Consume(TokenType::kRightBrace, "Expect '}' after class body.");
    EmitByte(Chunk::OpCode::kOpPop);

    if (class_compiler.has_superclass)
        EndScope();

    current_class_ = current_class_->enclosing;
}

void Compiler::Method()
{
    Consume(TokenType::kIdentifier, "Expect method name.");
    uint8_t constant = IdentifierConstant(parser_.previous);

    FunctionType type = FunctionType::kTypeMethod;
    static const std::string kInitStr("init");
    if (parser_.previous.GetLexeme() == kInitStr)
        type = FunctionType::kTypeInitializer;
    Function(type);

    EmitBytes(Chunk::OpCode::kOpMethod, constant);
}

void Compiler::AddLocal(const Token& name)
{
    if (compiler_->local_count == (UINT8_MAX + 1)) {
        Error("Too many local variables in function.");
        return;
    }

    Local local = {.name=name, .depth=-1, .is_captured=false};
    compiler_->locals[compiler_->local_count] = local;
    compiler_->local_count++;
}

void Compiler::MarkInitialized()
{
    if (0 == compiler_->scope_depth)
        return;

    compiler_->locals[compiler_->local_count - 1].depth =
        compiler_->scope_depth;
}

void Compiler::DeclareVariable()
{
    if (compiler_->scope_depth == 0)
        return;

    Token name = parser_.previous;
    for (int i = compiler_->local_count - 1; i >= 0; --i) {
        Local local = compiler_->locals[i];
        if ((local.depth != -1) && (local.depth < compiler_->scope_depth))
            break;

        if (IdentifiersEqual(name, local.name))
            Error("Already a variable with this name in this scope.");
    }
    AddLocal(name);
}

int Compiler::ResolveLocal(CompilerDataPtr compiler, const Token& name)
{
    for (int i = compiler->local_count - 1; i >= 0; --i) {
        Local local = compiler->locals[i];
        if (IdentifiersEqual(name, local.name)) {
            if (local.depth == -1)
                Error("Can't read local variable in its own intializer.");
            return i;
        }
    }
    return -1;
}

int Compiler::ResolveUpvalue(CompilerDataPtr compiler, const Token& name)
{
    if (!compiler->enclosing)
        return -1;

    int local = ResolveLocal(compiler->enclosing, name);
    if (-1 != local) {
        compiler->enclosing->locals[local].is_captured = true;
        return AddUpvalue(compiler, static_cast<uint8_t>(local), true);
    }

    int upvalue = ResolveUpvalue(compiler->enclosing, name);
    if (-1 != upvalue)
        return AddUpvalue(compiler, static_cast<uint8_t>(upvalue), false);

    return -1;
}

int Compiler::AddUpvalue(
    CompilerDataPtr compiler,
    uint8_t index,
    bool is_local)
{
    int upvalue_count = compiler->function->upvalue_count;
    for (int i = 0; i < upvalue_count; ++i) {
        Upvalue* upvalue = &compiler->upvalues[i];
        if ((upvalue->index == index) && (upvalue->is_local == is_local))
            return i;
    }

    if ((UINT8_MAX + 1) == upvalue_count) {
        Error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalue_count].is_local = is_local;
    compiler->upvalues[upvalue_count].index    = index;

    return compiler->function->upvalue_count++;
}

void Compiler::EndScope()
{
    compiler_->scope_depth--;

    while ((compiler_->local_count > 0) &&
           (compiler_->locals[compiler_->local_count - 1].depth >
            compiler_->scope_depth)) {
        if (compiler_->locals[compiler_->local_count - 1].is_captured)
            EmitByte(Chunk::OpCode::kOpCloseUpvalue);
        else
            EmitByte(Chunk::OpCode::kOpPop);

        compiler_->local_count--;
    }
}

void Compiler::Block()
{
    while (!Check(TokenType::kRightBrace) &&
           !Check(TokenType::kEof)) {
        Declaration();
    }
    Consume(TokenType::kRightBrace, "Expect '}' after block.");
}

void Compiler::Function(FunctionType type)
{
    CompilerDataPtr compiler = std::make_shared<CompilerData>();
    InitCompiler(compiler_, compiler, type);
    BeginScope();

    Consume(TokenType::kLeftParen, "Expect '(' after function name.");
    if (!Check(TokenType::kRightParen)) {
        do {
            compiler_->function->arity++;
            if (compiler_->function->arity > 255)
                ErrorAtCurrent("Can't have more than 255 parameters.");

            uint8_t constant = ParseVariable("Expect paramater name.");
            DefineVariable(constant);
        } while (Match(TokenType::kComma));
    }
    Consume(TokenType::kRightParen, "Expect ')' after parameters.");
    Consume(TokenType::kLeftBrace, "Expect '{' before function body");
    Block();

    std::shared_ptr<obj::ObjFunction> function = EndCompiler();
    EmitBytes(Chunk::OpCode::kOpClosure, MakeConstant(obj::ObjVal(function)));

    for (int i = 0; i < function->upvalue_count; ++i) {
        EmitByte(compiler->upvalues[i].is_local ?  1 : 0);
        EmitByte(compiler->upvalues[i].index);
    }
}

uint8_t Compiler::ArgumentList()
{
    uint8_t arg_count = 0;
    if (!Check(TokenType::kRightParen)) {
        do {
            Expression();
            if (255 == arg_count)
                Error("Can't have more than 255 arguments.");

            arg_count++;
        } while (Match(TokenType::kComma));
    }
    Consume(TokenType::kRightParen, "Expect ')' after arguments.");
    return arg_count;
}

void Compiler::Advance()
{
    parser_.previous = parser_.current;
    while (true) {
        parser_.current = scanner_.ScanToken();
        if (parser_.current.GetType() != TokenType::kError)
            break;

        ErrorAtCurrent(parser_.current.GetLexeme());
    }
}

bool Compiler::Match(TokenType type)
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
    int arg = ResolveLocal(compiler_, name);
    if (arg != -1) {
        get_op = Chunk::OpCode::kOpGetLocal;
        set_op = Chunk::OpCode::kOpSetLocal;
    } else if (-1 != (arg = ResolveUpvalue(compiler_, name))) {
        get_op = Chunk::OpCode::kOpGetUpvalue;
        set_op = Chunk::OpCode::kOpSetUpvalue;
    } else {
        arg = IdentifierConstant(name);
        get_op = Chunk::OpCode::kOpGetGlobal;
        set_op = Chunk::OpCode::kOpSetGlobal;
    }

    if (can_assign && Match(TokenType::kEqual)) {
        Expression();
        EmitBytes(set_op, arg);
    } else {
        EmitBytes(get_op, arg);
    }
}

void Compiler::Consume(TokenType type, const std::string& message)
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

    if (error.GetType() == TokenType::kEof) {
        fprintf(stderr, " at end");
    } else if (error.GetType() == TokenType::kError) {
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

    while (parser_.current.GetType() != TokenType::kEof) {
        if (parser_.previous.GetType() ==
            TokenType::kSemicolon)
            return;
        switch(parser_.current.GetType()) {
            case TokenType::kClass:
            case TokenType::kFun:
            case TokenType::kVar:
            case TokenType::kFor:
            case TokenType::kIf:
            case TokenType::kWhile:
            case TokenType::kPrint:
            case TokenType::kReturn:
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

void Compiler::EmitReturn()
{
    if (compiler_->type == FunctionType::kTypeInitializer)
        EmitBytes(Chunk::OpCode::kOpGetLocal, 0);
    else
        EmitByte(Chunk::OpCode::kOpNil);

    EmitByte(Chunk::OpCode::kOpReturn);
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
    std::shared_ptr<obj::ObjFunction> function = compiler_->function;
#ifdef DEBUG_PRINT_CODE
    if (!parser_.had_error) {
        CurrentChunk().Disassemble(
            function->name ? function->name->chars : "<script>");
    }
#endif
    compiler_ = compiler_->enclosing;
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
    Consume(TokenType::kRightParen, "Expect ')' after expression.");
}

void Compiler::Unary([[maybe_unused]]bool can_assign)
{
    TokenType operator_type = parser_.previous.GetType();

    /* Compile the operand. */
    ParsePrecedence(Precedence::kPrecUnary);

    /* Emit the operator instruction. */
    switch (operator_type) {
        case TokenType::kBang:
            EmitByte(Chunk::OpCode::kOpNot);
            break;
        case TokenType::kMinus:
            EmitByte(Chunk::OpCode::kOpNegate);
            break;
        default:
            /* Unreachable */
            return;
    }
}

void Compiler::Binary([[maybe_unused]]bool can_assign)
{
    TokenType operator_type = parser_.previous.GetType();
    ParsePrecedence(
        static_cast<Precedence>(rules_[operator_type].precedence + 1));

    switch (operator_type) {
        case TokenType::kBangEqual:
            EmitBytes(Chunk::OpCode::KOpEqual, Chunk::OpCode::kOpNot);
            break;
        case TokenType::kEqualEqual:
            EmitByte(Chunk::OpCode::KOpEqual);
            break;
        case TokenType::kGreater:
            EmitByte(Chunk::OpCode::kOpGreater);
            break;
        case TokenType::kGreaterEqual:
            EmitBytes(Chunk::OpCode::kOpLess, Chunk::OpCode::kOpNot);
            break;
        case TokenType::kLess:
            EmitByte(Chunk::OpCode::kOpLess);
            break;
        case TokenType::kLessEqual:
            EmitBytes(Chunk::OpCode::kOpGreater, Chunk::OpCode::kOpNot);
            break;
        case TokenType::kPlus:
            EmitByte(Chunk::OpCode::kOpAdd);
            break;
        case TokenType::kMinus:
            EmitByte(Chunk::OpCode::kOpSubtract);
            break;
        case TokenType::kStar:
            EmitByte(Chunk::OpCode::kOpMultiply);
            break;
        case TokenType::kSlash:
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
        case TokenType::kFalse:
            EmitByte(Chunk::OpCode::kOpFalse);
            break;
        case TokenType::kTrue:
            EmitByte(Chunk::OpCode::kOpTrue);
            break;
        case TokenType::kNil:
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

void Compiler::Call([[maybe_unused]]bool can_assign)
{
    uint8_t arg_count = ArgumentList();
    EmitBytes(Chunk::OpCode::kOpCall, arg_count);
}

void Compiler::Dot(bool can_assign)
{
    Consume(TokenType::kIdentifier, "Expect property name after '.'.");
    uint8_t name = IdentifierConstant(parser_.previous);

    if (can_assign && Match(TokenType::kEqual)) {
        Expression();
        EmitBytes(Chunk::OpCode::kOpSetProperty, name);
    } else if (Match(TokenType::kLeftParen)) {
        uint8_t arg_count = ArgumentList();
        EmitBytes(Chunk::OpCode::kOpInvoke, name);
        EmitByte(arg_count);
    } else {
        EmitBytes(Chunk::OpCode::kOpGetProperty, name);
    }
}

void Compiler::This([[maybe_unused]]bool can_assign)
{
    if (!current_class_) {
        Error("Can't use 'this' outside of a class.");
        return;
    }
    Variable(false);
}

void Compiler::Super([[maybe_unused]]bool can_assign)
{
    if (!current_class_)
        Error("Can't use 'super' outside of a class.");
    else if (!current_class_->has_superclass)
        Error("Can't use 'super' in a class with no superclass.");

    Consume(TokenType::kDot, "Expect '.' after 'super'.");
    Consume(TokenType::kIdentifier, "Expect superclass method name.");
    uint8_t name = IdentifierConstant(parser_.previous);

    NamedVariable(scanr::Token(TokenType::kThis, "this", 0), false);
    if (Match(TokenType::kLeftParen)) {
        uint8_t arg_count = ArgumentList();
        NamedVariable(scanr::Token(TokenType::kSuper, "super", 0), false);
        EmitBytes(Chunk::OpCode::kOpSuperInvoke, name);
        EmitByte(arg_count);
    } else {
        NamedVariable(scanr::Token(TokenType::kSuper, "super", 0), false);
        EmitBytes(Chunk::OpCode::kOpGetSuper, name);
    }
}

Compiler::Compiler() :
    scanner_(""),
    compiler_(std::make_shared<CompilerData>()),
    current_class_(nullptr)
{
    parser_.had_error  = false;
    parser_.panic_mode = false;

    InitCompiler(nullptr, compiler_, FunctionType::kTypeScript);
}

std::shared_ptr<obj::ObjFunction> Compiler::Compile(
    const std::string& source,
    InternedStrings strings)
{
    scanner_ = lox::scanr::Scanner(source);
    strings_ = strings;

    Advance();
    while (!Match(TokenType::kEof))
        Declaration();

    std::shared_ptr<obj::ObjFunction> function = EndCompiler();
    return (parser_.had_error ? nullptr : function);
}
} // end cl
} // end lox
