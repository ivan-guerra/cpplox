#include <stack>
#include <cstdio>
#include <cstdint>

#include "Chunk.h"
#include "Value.h"
#include "VirtualMachine.h"

namespace lox
{
void VirtualMachine::PrintVmStack()
{
    std::stack<value::value_t> aux;

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

void VirtualMachine::BinaryOp(Chunk::OpCode op)
{
    value::value_t b = vm_stack_.top();
    vm_stack_.pop();
    value::value_t a = vm_stack_.top();
    vm_stack_.pop();

    switch (op) {
        case Chunk::OpCode::kOpAdd:
            vm_stack_.push(a + b);
            break;
        case Chunk::OpCode::kOpSubtract:
            vm_stack_.push(a - b);
            break;
        case Chunk::OpCode::kOpMultiply:
            vm_stack_.push(a * b);
            break;
        case Chunk::OpCode::kOpDivide:
            vm_stack_.push(a / b);
            break;
        default:
            std::printf("Unknown opcode %d\n", op);
            exit(EXIT_FAILURE);
    }
}

VirtualMachine::InterpretResult VirtualMachine::Run()
{
    while (true) {
#ifdef DEBUG_TRACE_EXECUTION
        PrintVmStack();
        chunk_.Disassemble(ip_);
#endif
        uint8_t instruction = ReadByte();
        switch (instruction) {
            case Chunk::OpCode::kOpConstant:
                vm_stack_.push(ReadConstant());
                break;
            case Chunk::OpCode::kOpNegate: {
                value::value_t val = vm_stack_.top();
                vm_stack_.pop();
                vm_stack_.push(-val);
                break;
            }
            case Chunk::OpCode::kOpAdd:
            case Chunk::OpCode::kOpSubtract:
            case Chunk::OpCode::kOpMultiply:
            case Chunk::OpCode::kOpDivide:
                BinaryOp(static_cast<Chunk::OpCode>(instruction));
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
    ip_(0)
{

}

VirtualMachine::InterpretResult VirtualMachine::Interpret(const Chunk& chunk)
{
   chunk_ = chunk;
   ip_    = 0;
   return Run();
}
} // end lox
