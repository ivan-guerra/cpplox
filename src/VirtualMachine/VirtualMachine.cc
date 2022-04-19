#include <stack>
#include <memory>
#include <cstdio>
#include <cstdarg>
#include <cstdint>

#include "Chunk.h"
#include "Value.h"
#include "VirtualMachine.h"

namespace lox
{
void VirtualMachine::PrintVmStack()
{
    std::stack<value::Value> aux;

    while (!vm_stack_.empty()) {
        aux.push(vm_stack_.top());
        vm_stack_.pop();
    }

    std::printf("          ");
    while (!aux.empty()) {
        std::printf("[ ");
        vm_stack_.push(aux.top());
        value::PrintValue(aux.top());
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
                vm_stack_.push(value::NilVal());
                break;
            case Chunk::OpCode::kOpTrue:
                vm_stack_.push(value::BoolVal(true));
                break;
            case Chunk::OpCode::kOpFalse:
                vm_stack_.push(value::BoolVal(false));
                break;
            case Chunk::OpCode::KOpEqual: {
                value::Value b = vm_stack_.top();
                vm_stack_.pop();
                value::Value a = vm_stack_.top();
                vm_stack_.pop();

                vm_stack_.push(value::BoolVal(value::ValuesEqual(a, b)));
                break;
            }
            case Chunk::OpCode::kOpGreater:
            case Chunk::OpCode::kOpLess:
                BinaryOp<bool>(value::BoolVal,
                               static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpNot: {
                bool is_falsey = IsFalsey(vm_stack_.top());
                vm_stack_.pop();
                vm_stack_.push(value::BoolVal(is_falsey));
                break;
            }
            case Chunk::OpCode::kOpNegate: {
                value::Value val = vm_stack_.top();
                if (!value::IsNumber(val)) {
                    RuntimeError("Operand must be a number.");
                    return InterpretResult::kInterpretRuntimeError;
                }
                vm_stack_.pop();
                vm_stack_.push(value::NumberVal(-value::AsNumber(val)));
                break;
            }
            case Chunk::OpCode::kOpAdd:
            case Chunk::OpCode::kOpSubtract:
            case Chunk::OpCode::kOpMultiply:
            case Chunk::OpCode::kOpDivide:
                BinaryOp<double>(value::NumberVal,
                                 static_cast<Chunk::OpCode>(instruction));
                break;
            case Chunk::OpCode::kOpReturn:
                value::PrintValue(vm_stack_.top());
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
