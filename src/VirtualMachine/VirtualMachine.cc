#include <memory>
#include <cstdio>
#include <cstdarg>
#include <cstdint>

#include "Chunk.h"
#include "Value.h"
#include "Object.h"
#include "VirtualMachine.h"

namespace lox
{
void VirtualMachine::RuntimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    CallFrame& frame = frames_.Top();
    size_t instruction = frame.ip - 1;
    int line = frame.function->chunk.GetLines().at(instruction);
    std::fprintf(stderr, "[line %d] in script\n", line);

    stack_.Reset();
}


uint8_t VirtualMachine::ReadByte()
{
    CallFrame& frame = frames_.Top();
    return frame.function->chunk.GetInstruction(frame.ip++);
}


val::Value VirtualMachine::ReadConstant()
{
    CallFrame& frame = frames_.Top();
    return frame.function->chunk.GetConstants()[ReadByte()];
}

uint16_t VirtualMachine::ReadShort()
{
    CallFrame& frame = frames_.Top();
    frame.ip += 2;
    return ((frame.function->chunk.GetInstruction(frame.ip - 2) << 8) |
             frame.function->chunk.GetInstruction(frame.ip - 1));
}

void VirtualMachine::Concatenate()
{
    std::shared_ptr<obj::ObjString> b = obj::AsString(stack_.Pop());
    std::shared_ptr<obj::ObjString> a = obj::AsString(stack_.Pop());

    std::shared_ptr<obj::ObjString> result = std::make_shared<obj::ObjString>();
    result->type = obj::ObjType::kObjString;
    result->chars = a->chars + b->chars;
    stack_.Push(ObjVal(result));
}

VirtualMachine::InterpretResult VirtualMachine::Run()
{
    CallFrame& frame = frames_.Top();

    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        stack_.Print();
        frame.function->chunk.Disassemble(frame.ip);
#endif
        uint8_t instruction = ReadByte();
        switch (instruction) {
            case Chunk::OpCode::kOpConstant:
                stack_.Push(ReadConstant());
                break;
            case Chunk::OpCode::kOpNil:
                stack_.Push(val::NilVal());
                break;
            case Chunk::OpCode::kOpTrue:
                stack_.Push(val::BoolVal(true));
                break;
            case Chunk::OpCode::kOpFalse:
                stack_.Push(val::BoolVal(false));
                break;
            case Chunk::OpCode::KOpEqual: {
                val::Value b = stack_.Pop();
                val::Value a = stack_.Pop();
                stack_.Push(val::BoolVal(val::ValuesEqual(a, b)));
                break;
            }
            case Chunk::OpCode::kOpGreater:
            case Chunk::OpCode::kOpLess:
                BinaryOp<bool>(val::BoolVal,
                               static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpNot: {
                bool is_falsey = IsFalsey(stack_.Pop());
                stack_.Push(val::BoolVal(is_falsey));
                break;
            }
            case Chunk::OpCode::kOpNegate: {
                val::Value val = stack_.Top();
                if (!val::IsNumber(val)) {
                    RuntimeError("Operand must be a number.");
                    return InterpretResult::kInterpretRuntimeError;
                }
                stack_.Pop();
                stack_.Push(val::NumberVal(-val::AsNumber(val)));
                break;
            }
            case Chunk::OpCode::kOpAdd: {
                val::Value b = stack_.Peek(0);
                val::Value a = stack_.Peek(1);
                if (obj::IsString(a) && obj::IsString(b)) {
                    Concatenate();
                } else if (val::IsNumber(a) && val::IsNumber(b)) {
                    BinaryOp<double>(val::NumberVal,
                                     static_cast<Chunk::OpCode>(instruction));
                } else {
                    RuntimeError(
                        "Operands must be two numbers or two strings.");
                    return InterpretResult::kInterpretRuntimeError;
                }
                break;
            }
            case Chunk::OpCode::kOpSubtract:
            case Chunk::OpCode::kOpMultiply:
            case Chunk::OpCode::kOpDivide:
                BinaryOp<double>(val::NumberVal,
                                 static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpPrint:
                PrintValue(stack_.Pop());
                std::printf("\n");
                break;
            case Chunk::OpCode::kOpPop:
                stack_.Pop();
                break;
            case Chunk::OpCode::kOpDefineGlobal: {
                LoxString name = obj::AsString(ReadConstant());
                globals_[name] = stack_.Pop();
                break;
            }
            case Chunk::OpCode::kOpGetGlobal: {
                LoxString name = obj::AsString(ReadConstant());
                if (globals_.find(name) == globals_.end()) {
                    RuntimeError(
                        "Undefined variable '%s'.",
                        name->chars.c_str());
                    return InterpretResult::kInterpretRuntimeError;
                }
                stack_.Push(globals_[name]);
                break;
            }
            case Chunk::OpCode::kOpSetGlobal: {
                LoxString name = obj::AsString(ReadConstant());
                if (globals_.find(name) == globals_.end()) {
                    RuntimeError(
                        "Undefined variable '%s'.",
                        name->chars.c_str());
                    return InterpretResult::kInterpretRuntimeError;
                }
                globals_[name] = stack_.Peek(0);
                break;
            }
            case Chunk::OpCode::kOpGetLocal: {
                uint8_t slot = ReadByte();
                stack_.Push(frame.slots[slot]);
                break;
            }
            case Chunk::OpCode::kOpSetLocal: {
                uint8_t slot = ReadByte();
                frame.slots[slot] = stack_.Peek(0);
                break;
            }
            case Chunk::OpCode::kOpJumpIfFalse: {
                uint16_t offset = ReadShort();
                if (IsFalsey(stack_.Top()))
                    frame.ip += offset;
                break;
            }
            case Chunk::OpCode::kOpJump: {
                uint16_t offset = ReadShort();
                frame.ip += offset;
                break;
            }
            case Chunk::OpCode::kOpLoop: {
                uint16_t offset = ReadShort();
                frame.ip -= offset;
                break;
            }
            case Chunk::OpCode::kOpReturn:
                return InterpretResult::kInterpretOk;
        }
    }
}

VirtualMachine::VirtualMachine() :
    strings_(std::make_shared<LoxStringMap>()),
    frames_(kFramesMax),
    stack_(kStackMax)
{

}

VirtualMachine::InterpretResult VirtualMachine::Interpret(
    const std::string& source)
{
    Compiler compiler;
    std::shared_ptr<obj::ObjFunction> function =
        compiler.Compile(source, strings_);

    if (!function)
        return InterpretResult::kInterpretCompileError;

    stack_.Push(obj::ObjVal(function));
    CallFrame frame = {.function=function, .ip=0, .slots=stack_.Data()};
    frames_.Push(frame);

    return Run();
}
} // end lox
