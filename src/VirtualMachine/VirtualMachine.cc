#include <stack>
#include <queue>
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
val::Value VirtualMachine::Peek(int i)
{
    if (0 == i)
        return vm_stack_.top();

    std::queue<val::Value> aux;
    while (i--) {
        aux.push(vm_stack_.top());
        vm_stack_.pop();
    }

    val::Value ret = vm_stack_.top();

    while (!aux.empty()) {
        vm_stack_.push(aux.front());
        aux.pop();
    }
    return ret;
}

void VirtualMachine::PrintVmStack()
{
    std::stack<val::Value> aux;

    while (!vm_stack_.empty()) {
        aux.push(vm_stack_.top());
        vm_stack_.pop();
    }

    std::printf("          ");
    while (!aux.empty()) {
        std::printf("[ ");
        vm_stack_.push(aux.top());
        val::PrintValue(aux.top());
        aux.pop();
        std::printf(" ]");
    }
    std::printf("\n");
}

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
    vm_stack_ = {};
}

void VirtualMachine::Concatenate()
{
    std::shared_ptr<obj::ObjString> b = obj::AsString(vm_stack_.top());
    vm_stack_.pop();
    std::shared_ptr<obj::ObjString> a = obj::AsString(vm_stack_.top());
    vm_stack_.pop();

    std::shared_ptr<obj::ObjString> result = std::make_shared<obj::ObjString>();
    result->type = obj::ObjType::kObjString;
    result->chars = a->chars + b->chars;
    vm_stack_.push(ObjVal(result));
}

VirtualMachine::InterpretResult VirtualMachine::Run()
{
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        PrintVmStack();
        chunk_->Disassemble(ip_);
#endif
        uint8_t instruction = ReadByte();
        switch (instruction) {
            case Chunk::OpCode::kOpConstant:
                vm_stack_.push(ReadConstant());
                break;
            case Chunk::OpCode::kOpNil:
                vm_stack_.push(val::NilVal());
                break;
            case Chunk::OpCode::kOpTrue:
                vm_stack_.push(val::BoolVal(true));
                break;
            case Chunk::OpCode::kOpFalse:
                vm_stack_.push(val::BoolVal(false));
                break;
            case Chunk::OpCode::KOpEqual: {
                val::Value b = vm_stack_.top();
                vm_stack_.pop();
                val::Value a = vm_stack_.top();
                vm_stack_.pop();

                vm_stack_.push(val::BoolVal(val::ValuesEqual(a, b)));
                break;
            }
            case Chunk::OpCode::kOpGreater:
            case Chunk::OpCode::kOpLess:
                BinaryOp<bool>(val::BoolVal,
                               static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpNot: {
                bool is_falsey = IsFalsey(vm_stack_.top());
                vm_stack_.pop();
                vm_stack_.push(val::BoolVal(is_falsey));
                break;
            }
            case Chunk::OpCode::kOpNegate: {
                val::Value val = vm_stack_.top();
                if (!val::IsNumber(val)) {
                    RuntimeError("Operand must be a number.");
                    return InterpretResult::kInterpretRuntimeError;
                }
                vm_stack_.pop();
                vm_stack_.push(val::NumberVal(-val::AsNumber(val)));
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
            case Chunk::OpCode::kOpReturn:
                val::PrintValue(vm_stack_.top());
                vm_stack_.pop();
                std::printf("\n");
                return InterpretResult::kInterpretOk;
        }
    }
}

VirtualMachine::VirtualMachine() :
    ip_(0),
    chunk_(std::make_shared<Chunk>())
{

}

VirtualMachine::InterpretResult VirtualMachine::Interpret(
    const std::string& source)
{
    if (!compiler_.Compile(source, chunk_))
        return InterpretResult::kInterpretCompileError;

    /* Set the instruction pointer to point to the start of the chunk. */
    ip_ = 0;

    return Run();
}
} // end lox
