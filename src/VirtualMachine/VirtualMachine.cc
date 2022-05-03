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

    for (int i = frame_count - 1; i >= 0; --i) {
        CallFrame* frame = &frames_[i];
        std::shared_ptr<obj::ObjFunction> function = frame->function;
        std::size_t instruction = frame->ip - 1;

        std::fprintf(stderr, "[line %d] in ",
                     function->chunk.GetLines()[instruction]);
        if (!function->name)
            std::fprintf(stderr, "script\n");
        else
            std::fprintf(stderr, "%s()\n", function->name->chars.c_str());
    }
    ResetStack(&vm_stack);
}

bool VirtualMachine::Call(
    std::shared_ptr<obj::ObjFunction> function,
    int arg_count)
{
    if (arg_count != function->arity) {
        RuntimeError("Expected %d arguments but got %d.",
                     function->arity, arg_count);
        return false;
    }

    if (frame_count == kFramesMax) {
        RuntimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &frames_[frame_count++];
    frame->function = function;
    frame->ip       = 0;
    frame->slots    = vm_stack.stack_top - arg_count - 1;

    return true;
}

bool VirtualMachine::CallValue(const val::Value& callee, int arg_count)
{
    if (obj::IsObject(callee)) {
        switch (obj::GetType(callee)) {
            case obj::ObjType::kObjFunction:
                return Call(obj::AsFunction(callee), arg_count);
                break;
            default:
                /* Non-callable object type. */
                break;
        }
    }
    RuntimeError("Can only call functions and classes.");
    return false;
}

uint16_t VirtualMachine::ReadShort(CallFrame* frame)
{
    frame->ip += 2;
    return ((frame->function->chunk.GetInstruction(frame->ip - 2) << 8) |
             frame->function->chunk.GetInstruction(frame->ip - 1));
}

void VirtualMachine::Concatenate()
{
    std::shared_ptr<obj::ObjString> b = obj::AsString(Pop(&vm_stack));
    std::shared_ptr<obj::ObjString> a = obj::AsString(Pop(&vm_stack));

    std::shared_ptr<obj::ObjString> result = std::make_shared<obj::ObjString>();
    result->type = obj::ObjType::kObjString;
    result->chars = a->chars + b->chars;
    Push(&vm_stack, ObjVal(result));
}

VirtualMachine::InterpretResult VirtualMachine::Run()
{
    CallFrame* frame = &frames_[frame_count - 1];

    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        PrintStack(&vm_stack);
        frame->function->chunk.Disassemble(frame->ip);
#endif
        uint8_t instruction = ReadByte(frame);
        switch (instruction) {
            case Chunk::OpCode::kOpConstant:
                Push(&vm_stack, ReadConstant(frame));
                break;
            case Chunk::OpCode::kOpNil:
                Push(&vm_stack, val::NilVal());
                break;
            case Chunk::OpCode::kOpTrue:
                Push(&vm_stack, val::BoolVal(true));
                break;
            case Chunk::OpCode::kOpFalse:
                Push(&vm_stack, val::BoolVal(false));
                break;
            case Chunk::OpCode::KOpEqual: {
                val::Value b = Pop(&vm_stack);
                val::Value a = Pop(&vm_stack);
                Push(&vm_stack, val::BoolVal(val::ValuesEqual(a, b)));
                break;
            }
            case Chunk::OpCode::kOpGreater:
            case Chunk::OpCode::kOpLess:
                BinaryOp<bool>(val::BoolVal,
                               static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpNot: {
                bool is_falsey = IsFalsey(Pop(&vm_stack));
                Push(&vm_stack, val::BoolVal(is_falsey));
                break;
            }
            case Chunk::OpCode::kOpNegate: {
                val::Value val = Peek(&vm_stack, 0);
                if (!val::IsNumber(val)) {
                    RuntimeError("Operand must be a number.");
                    return InterpretResult::kInterpretRuntimeError;
                }
                Pop(&vm_stack);
                Push(&vm_stack, val::NumberVal(-val::AsNumber(val)));
                break;
            }
            case Chunk::OpCode::kOpAdd: {
                val::Value b = Peek(&vm_stack, 0);
                val::Value a = Peek(&vm_stack, 1);
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
                PrintValue(Pop(&vm_stack));
                std::printf("\n");
                break;
            case Chunk::OpCode::kOpPop:
                Pop(&vm_stack);
                break;
            case Chunk::OpCode::kOpDefineGlobal: {
                LoxString name = obj::AsString(ReadConstant(frame));
                globals_[name] = Pop(&vm_stack);
                break;
            }
            case Chunk::OpCode::kOpGetGlobal: {
                LoxString name = obj::AsString(ReadConstant(frame));
                if (globals_.find(name) == globals_.end()) {
                    RuntimeError(
                        "Undefined variable '%s'.",
                        name->chars.c_str());
                    return InterpretResult::kInterpretRuntimeError;
                }
                Push(&vm_stack, globals_[name]);
                break;
            }
            case Chunk::OpCode::kOpSetGlobal: {
                LoxString name = obj::AsString(ReadConstant(frame));
                if (globals_.find(name) == globals_.end()) {
                    RuntimeError(
                        "Undefined variable '%s'.",
                        name->chars.c_str());
                    return InterpretResult::kInterpretRuntimeError;
                }
                globals_[name] = Peek(&vm_stack, 0);
                break;
            }
            case Chunk::OpCode::kOpGetLocal: {
                uint8_t slot = ReadByte(frame);
                Push(&vm_stack, frame->slots[slot]);
                break;
            }
            case Chunk::OpCode::kOpSetLocal: {
                uint8_t slot = ReadByte(frame);
                frame->slots[slot] = Peek(&vm_stack, 0);
                break;
            }
            case Chunk::OpCode::kOpJumpIfFalse: {
                uint16_t offset = ReadShort(frame);
                if (IsFalsey(Peek(&vm_stack, 0)))
                    frame->ip += offset;
                break;
            }
            case Chunk::OpCode::kOpJump: {
                uint16_t offset = ReadShort(frame);
                frame->ip += offset;
                break;
            }
            case Chunk::OpCode::kOpLoop: {
                uint16_t offset = ReadShort(frame);
                frame->ip -= offset;
                break;
            }
            case Chunk::OpCode::kOpCall: {
                int arg_count = ReadByte(frame);
                if (!CallValue(Peek(&vm_stack, arg_count), arg_count))
                    return InterpretResult::kInterpretRuntimeError;

                frame = &frames_[frame_count - 1];
                break;
            }
            case Chunk::OpCode::kOpReturn: {
                val::Value result = Pop(&vm_stack);
                frame_count--;
                if (0 == frame_count) {
                    Pop(&vm_stack);
                    return InterpretResult::kInterpretOk;
                }

                vm_stack.stack_top = frame->slots;
                Push(&vm_stack, result);
                frame = &frames_[frame_count - 1];
                break;
            }
        }
    }
}

VirtualMachine::VirtualMachine() :
    strings_(std::make_shared<LoxStringMap>()),
    frame_count(0)
{
    ResetStack(&vm_stack);
}

VirtualMachine::InterpretResult VirtualMachine::Interpret(
    const std::string& source)
{
    Compiler compiler;
    std::shared_ptr<obj::ObjFunction> function =
        compiler.Compile(source, strings_);

    if (!function)
        return InterpretResult::kInterpretCompileError;

    Push(&vm_stack, obj::ObjVal(function));
    Call(function, 0);

    return Run();
}
} // end lox
