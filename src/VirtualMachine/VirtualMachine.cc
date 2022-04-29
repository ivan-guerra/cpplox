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

    size_t instruction = chunk_->GetCode().size() - ip_ - 1;
    int line = chunk_->GetLines().at(instruction);
    std::fprintf(stderr, "[line %d] in script\n", line);

    vm_stack_.Reset();
}

uint16_t VirtualMachine::ReadShort()
{
    ip_ += 2;
    return ((chunk_->GetInstruction(ip_ - 2) << 8) |
             chunk_->GetInstruction(ip_ - 1));
}

void VirtualMachine::Concatenate()
{
    std::shared_ptr<obj::ObjString> b = obj::AsString(vm_stack_.Pop());
    std::shared_ptr<obj::ObjString> a = obj::AsString(vm_stack_.Pop());

    std::shared_ptr<obj::ObjString> result = std::make_shared<obj::ObjString>();
    result->type = obj::ObjType::kObjString;
    result->chars = a->chars + b->chars;
    vm_stack_.Push(ObjVal(result));
}

VirtualMachine::InterpretResult VirtualMachine::Run()
{
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        vm_stack_.Print();
        chunk_->Disassemble(ip_);
#endif
        uint8_t instruction = ReadByte();
        switch (instruction) {
            case Chunk::OpCode::kOpConstant:
                vm_stack_.Push(ReadConstant());
                break;
            case Chunk::OpCode::kOpNil:
                vm_stack_.Push(val::NilVal());
                break;
            case Chunk::OpCode::kOpTrue:
                vm_stack_.Push(val::BoolVal(true));
                break;
            case Chunk::OpCode::kOpFalse:
                vm_stack_.Push(val::BoolVal(false));
                break;
            case Chunk::OpCode::KOpEqual: {
                val::Value b = vm_stack_.Pop();
                val::Value a = vm_stack_.Pop();
                vm_stack_.Push(val::BoolVal(val::ValuesEqual(a, b)));
                break;
            }
            case Chunk::OpCode::kOpGreater:
            case Chunk::OpCode::kOpLess:
                BinaryOp<bool>(val::BoolVal,
                               static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpNot: {
                bool is_falsey = IsFalsey(vm_stack_.Pop());
                vm_stack_.Push(val::BoolVal(is_falsey));
                break;
            }
            case Chunk::OpCode::kOpNegate: {
                val::Value val = vm_stack_.Top();
                if (!val::IsNumber(val)) {
                    RuntimeError("Operand must be a number.");
                    return InterpretResult::kInterpretRuntimeError;
                }
                vm_stack_.Pop();
                vm_stack_.Push(val::NumberVal(-val::AsNumber(val)));
                break;
            }
            case Chunk::OpCode::kOpAdd: {
                val::Value b = vm_stack_.Peek(0);
                val::Value a = vm_stack_.Peek(1);
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
                PrintValue(vm_stack_.Pop());
                std::printf("\n");
                break;
            case Chunk::OpCode::kOpPop:
                vm_stack_.Pop();
                break;
            case Chunk::OpCode::kOpDefineGlobal: {
                LoxString name = obj::AsString(ReadConstant());
                globals_[name] = vm_stack_.Pop();
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
                vm_stack_.Push(globals_[name]);
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
                globals_[name] = vm_stack_.Peek(0);
                break;
            }
            case Chunk::OpCode::kOpGetLocal: {
                uint8_t slot = ReadByte();
                vm_stack_.Push(vm_stack_.Peek(slot));
                break;
            }
            case Chunk::OpCode::kOpSetLocal: {
                uint8_t slot = ReadByte();
                vm_stack_.SetAt(slot, vm_stack_.Top());
                break;
            }
            case Chunk::OpCode::kOpJumpIfFalse: {
                uint16_t offset = ReadShort();
                if (IsFalsey(vm_stack_.Top()))
                    ip_ += offset;
                break;
            }
            case Chunk::OpCode::kOpJump: {
                uint16_t offset = ReadShort();
                ip_ += offset;
                break;
            }
            case Chunk::OpCode::kOpLoop: {
                uint16_t offset = ReadShort();
                ip_ -= offset;
                break;
            }
            case Chunk::OpCode::kOpReturn:
                return InterpretResult::kInterpretOk;
        }
    }
}

VirtualMachine::VirtualMachine() :
    ip_(0),
    chunk_(std::make_shared<Chunk>()),
    strings_(std::make_shared<LoxStringMap>())
{

}

VirtualMachine::InterpretResult VirtualMachine::Interpret(
    const std::string& source)
{
    if (!compiler_.Compile(source, chunk_, strings_))
        return InterpretResult::kInterpretCompileError;

    return Run();
}
} // end lox
