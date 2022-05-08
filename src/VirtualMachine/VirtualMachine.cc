#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <memory>

#include "Chunk.h"
#include "Value.h"
#include "Object.h"
#include "VirtualMachine.h"

namespace lox
{
namespace vm
{
static val::Value ClockNative(
    [[maybe_unused]]int arg_count,
    [[maybe_unused]]val::Value* args)
{
    return val::NumberVal(static_cast<double>(clock()) / CLOCKS_PER_SEC);
}

void VirtualMachine::RuntimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = frame_count - 1; i >= 0; --i) {
        CallFrame* frame = &frames_[i];
        std::shared_ptr<obj::ObjFunction> function = frame->closure->function;
        std::size_t instruction = frame->ip - 1;

        std::fprintf(stderr, "[line %d] in ",
                     function->chunk.GetLines()[instruction]);
        if (!function->name)
            std::fprintf(stderr, "script\n");
        else
            std::fprintf(stderr, "%s()\n", function->name->chars.c_str());
    }
    ResetStack();
}

bool VirtualMachine::Call(
    std::shared_ptr<obj::ObjClosure> closure,
    int arg_count)
{
    if (arg_count != closure->function->arity) {
        RuntimeError("Expected %d arguments but got %d.",
                     closure->function->arity, arg_count);
        return false;
    }

    if (frame_count == kFramesMax) {
        RuntimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &frames_[frame_count++];
    frame->closure = closure;
    frame->ip      = 0;
    frame->slots   = vm_stack.stack_top - arg_count - 1;

    return true;
}

bool VirtualMachine::CallValue(const val::Value& callee, int arg_count)
{
    if (obj::IsObject(callee)) {
        switch (obj::GetType(callee)) {
            case obj::ObjType::kObjClosure:
                return Call(obj::AsClosure(callee), arg_count);
                break;
            case obj::ObjType::kObjNative: {
                obj::NativeFn native = obj::AsNative(callee);
                val::Value result = native(arg_count,
                                           vm_stack.stack_top - arg_count);
                vm_stack.stack_top -= arg_count + 1;
                Push(result);
                return true;
                break;
            }
            case obj::ObjType::kObjClass: {
                std::shared_ptr<obj::ObjClass> klass = obj::AsClass(callee);
                vm_stack.stack_top[-arg_count - 1] =
                    obj::ObjVal(obj::NewInstance(klass));

                if (klass->methods.find(init_string_) != klass->methods.end()) {
                    return Call(obj::AsClosure(klass->methods[init_string_]),
                                arg_count);
                } else if (arg_count != 0) {
                    RuntimeError("Expected 0 arguments but got %d.",
                                 arg_count);
                    return false;
                }
                return true;
                break;
            }
            case obj::ObjType::kObjBoundMethod: {
                std::shared_ptr<obj::ObjBoundMethod> bound =
                    obj::AsBoundMethod(callee);
                vm_stack.stack_top[-arg_count - 1] = bound->receiver;
                return Call(bound->method, arg_count);
            }
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
    return ((frame->closure->function->chunk.GetInstruction(frame->ip - 2) << 8) |
             frame->closure->function->chunk.GetInstruction(frame->ip - 1));
}

void VirtualMachine::Concatenate()
{
    std::shared_ptr<obj::ObjString> b = obj::AsString(Pop());
    std::shared_ptr<obj::ObjString> a = obj::AsString(Pop());

    std::shared_ptr<obj::ObjString> result = std::make_shared<obj::ObjString>();
    result->type = obj::ObjType::kObjString;
    result->chars = a->chars + b->chars;
    Push(ObjVal(result));
}

void VirtualMachine::DefineNative(
    const std::string& name,
    obj::NativeFn function)
{
    Push(obj::ObjVal(obj::CopyString(name, strings_)));
    Push(obj::ObjVal(obj::NewNative(function)));
    globals_[obj::AsString(vm_stack.stack[0])] = vm_stack.stack[1];
    Pop();
    Pop();
}

void VirtualMachine::DefineMethod(std::shared_ptr<obj::ObjString> name)
{
    val::Value method = Peek(0);
    std::shared_ptr<obj::ObjClass> klass = obj::AsClass(Peek(1));
    klass->methods[name] = method;
    Pop();
}

bool VirtualMachine::BindMethod(
    std::shared_ptr<obj::ObjClass> klass,
    std::shared_ptr<obj::ObjString> name)
{
    if (klass->methods.find(name) == klass->methods.end()) {
        RuntimeError("Undefined property '%s'.", name->chars.c_str());
        return false;
    }

    std::shared_ptr<obj::ObjBoundMethod> bound =
        obj::NewBoundMethod(Peek(0), obj::AsClosure(klass->methods[name]));

    Pop();
    Push(obj::ObjVal(bound));
    return true;
}

bool VirtualMachine::InvokeFromClass(
    std::shared_ptr<obj::ObjClass> klass,
    std::shared_ptr<obj::ObjString> name,
    int arg_count)
{
    if (klass->methods.find(name) == klass->methods.end()) {
        RuntimeError("Undefined property '%s'.", name->chars.c_str());
        return false;
    }
    return Call(obj::AsClosure(klass->methods[name]), arg_count);
}

bool VirtualMachine::Invoke(
    std::shared_ptr<obj::ObjString> name,
    int arg_count)
{
    val::Value receiver = Peek(arg_count);
    if (!obj::IsInstance(receiver)) {
        RuntimeError("Only instances have methods.");
        return false;
    }

    std::shared_ptr<obj::ObjInstance> instance = obj::AsInstance(receiver);
    if (instance->fields.find(name) != instance->fields.end()) {
        vm_stack.stack_top[-arg_count - 1] = instance->fields[name];
        return CallValue(instance->fields[name], arg_count);
    }
    return InvokeFromClass(instance->klass, name, arg_count);
}

void VirtualMachine::CloseUpvalues(val::Value* last)
{
    while (open_upvalues_ && (open_upvalues_->location >= last)) {
        UpvaluePtr upvalue = open_upvalues_;
        upvalue->closed    = *upvalue->location;
        upvalue->location  = &upvalue->closed;
        open_upvalues_     = upvalue->next;
    }
}

std::shared_ptr<obj::ObjUpvalue> VirtualMachine::CaptureUpvalue(
    val::Value* local)
{
    UpvaluePtr prev_upvalue = nullptr;
    UpvaluePtr upvalue = open_upvalues_;
    while (upvalue && (upvalue->location > local)) {
        prev_upvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue && (upvalue->location == local))
        return upvalue;

    UpvaluePtr created_upvalue = obj::NewUpvalue(local);
    created_upvalue->next = upvalue;
    if (!prev_upvalue)
        open_upvalues_ = created_upvalue;
    else
        prev_upvalue->next = created_upvalue;

    return created_upvalue;
}

VirtualMachine::InterpretResult VirtualMachine::Run()
{
    CallFrame* frame = &frames_[frame_count - 1];

    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        PrintStack();
        frame->closure->function->chunk.Disassemble(frame->ip);
#endif
        uint8_t instruction = ReadByte(frame);
        switch (instruction) {
            case Chunk::OpCode::kOpConstant:
                Push(ReadConstant(frame));
                break;
            case Chunk::OpCode::kOpNil:
                Push(val::NilVal());
                break;
            case Chunk::OpCode::kOpTrue:
                Push(val::BoolVal(true));
                break;
            case Chunk::OpCode::kOpFalse:
                Push(val::BoolVal(false));
                break;
            case Chunk::OpCode::KOpEqual: {
                val::Value b = Pop();
                val::Value a = Pop();
                Push(val::BoolVal(val::ValuesEqual(a, b)));
                break;
            }
            case Chunk::OpCode::kOpGreater:
            case Chunk::OpCode::kOpLess:
                BinaryOp<bool>(val::BoolVal,
                               static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpNot: {
                bool is_falsey = IsFalsey(Pop());
                Push(val::BoolVal(is_falsey));
                break;
            }
            case Chunk::OpCode::kOpNegate: {
                val::Value val = Peek(0);
                if (!val::IsNumber(val)) {
                    RuntimeError("Operand must be a number.");
                    return InterpretResult::kInterpretRuntimeError;
                }
                Pop();
                Push(val::NumberVal(-val::AsNumber(val)));
                break;
            }
            case Chunk::OpCode::kOpAdd: {
                val::Value b = Peek(0);
                val::Value a = Peek(1);
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
                PrintValue(Pop());
                std::printf("\n");
                break;
            case Chunk::OpCode::kOpPop:
                Pop();
                break;
            case Chunk::OpCode::kOpDefineGlobal: {
                LoxString name = obj::AsString(ReadConstant(frame));
                globals_[name] = Pop();
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
                Push(globals_[name]);
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
                globals_[name] = Peek(0);
                break;
            }
            case Chunk::OpCode::kOpGetLocal: {
                uint8_t slot = ReadByte(frame);
                Push(frame->slots[slot]);
                break;
            }
            case Chunk::OpCode::kOpSetLocal: {
                uint8_t slot = ReadByte(frame);
                frame->slots[slot] = Peek(0);
                break;
            }
            case Chunk::OpCode::kOpJumpIfFalse: {
                uint16_t offset = ReadShort(frame);
                if (IsFalsey(Peek(0)))
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
                if (!CallValue(Peek(arg_count), arg_count))
                    return InterpretResult::kInterpretRuntimeError;

                frame = &frames_[frame_count - 1];
                break;
            }
            case Chunk::OpCode::kOpReturn: {
                val::Value result = Pop();
                CloseUpvalues(frame->slots);
                frame_count--;
                if (0 == frame_count) {
                    Pop();
                    return InterpretResult::kInterpretOk;
                }

                vm_stack.stack_top = frame->slots;
                Push(result);
                frame = &frames_[frame_count - 1];
                break;
            }
            case Chunk::OpCode::kOpClosure: {
                std::shared_ptr<obj::ObjFunction> function =
                    obj::AsFunction(ReadConstant(frame));
                std::shared_ptr<obj::ObjClosure> closure =
                    obj::NewClosure(function);
                Push(obj::ObjVal(closure));
                for (int i = 0; i < closure->upvalue_count; ++i) {
                    uint8_t is_local = ReadByte(frame);
                    uint8_t index    = ReadByte(frame);
                    if (is_local) {
                        closure->upvalues[i] =
                            CaptureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case Chunk::OpCode::kOpGetUpvalue: {
                uint8_t slot = ReadByte(frame);
                Push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case Chunk::OpCode::kOpSetUpvalue: {
                uint8_t slot = ReadByte(frame);
                *frame->closure->upvalues[slot]->location = Peek(0);
                break;
            }
            case Chunk::OpCode::kOpCloseUpvalue: {
                CloseUpvalues(vm_stack.stack_top - 1);
                Pop();
                break;
            }
            case Chunk::OpCode::kOpClass: {
                std::shared_ptr<obj::ObjString> klass_name = ReadString(frame);
                Push(obj::ObjVal(obj::NewClass(klass_name)));
                break;
            }
            case Chunk::OpCode::kOpGetProperty: {
                if (!obj::IsInstance(Peek(0))) {
                    RuntimeError("Only instances have properties.");
                    return InterpretResult::kInterpretRuntimeError;
                }

                std::shared_ptr<obj::ObjInstance> instance =
                    obj::AsInstance(Peek(0));
                std::shared_ptr<obj::ObjString> name =
                    ReadString(frame);
                if (instance->fields.find(name) != instance->fields.end()) {
                    Pop();
                    Push(instance->fields[name]);
                    break;
                }

                if (!BindMethod(instance->klass, name))
                    return InterpretResult::kInterpretRuntimeError;
                break;
            }
            case Chunk::OpCode::kOpSetProperty: {
                if (!obj::IsInstance(Peek(1))) {
                    RuntimeError("Only instances have fields.");
                    return InterpretResult::kInterpretRuntimeError;
                }

                std::shared_ptr<obj::ObjInstance> instance =
                    obj::AsInstance(Peek(1));
                instance->fields[ReadString(frame)] = Peek(0);

                val::Value value = Pop();
                Pop();
                Push(value);
                break;
            }
            case Chunk::OpCode::kOpInvoke: {
                std::shared_ptr<obj::ObjString> method = ReadString(frame);
                int arg_count = ReadByte(frame);
                if (!Invoke(method, arg_count))
                    return InterpretResult::kInterpretRuntimeError;

                frame = &frames_[frame_count - 1];
                break;
            }
            case Chunk::OpCode::kOpMethod:
                DefineMethod(ReadString(frame));
                break;
        }
    }
}

VirtualMachine::VirtualMachine() :
    strings_(std::make_shared<LoxStringMap>()),
    frame_count(0),
    open_upvalues_(nullptr),
    init_string_(nullptr)
{
    ResetStack();
    init_string_ = obj::CopyString("init", strings_);
    DefineNative("clock", ClockNative);
}

VirtualMachine::InterpretResult VirtualMachine::Interpret(
    const std::string& source)
{
    lox::cl::Compiler compiler;
    std::shared_ptr<obj::ObjFunction> function =
        compiler.Compile(source, strings_);

    if (!function)
        return InterpretResult::kInterpretCompileError;

    Push(obj::ObjVal(function));
    std::shared_ptr<obj::ObjClosure> closure = obj::NewClosure(function);
    Pop();
    Push(obj::ObjVal(closure));
    Call(closure, 0);

    return Run();
}
} // end vm
} // end lox
