#pragma once

#include <stack>
#include <cstddef>
#include <cstdint>

#include "Value.h"
#include "Chunk.h"

namespace lox
{
class VirtualMachine
{
public:
    enum class InterpretResult
    {
        kInterpretOk,
        kInterpretCompileError,
        kInterpretRuntimeError
    }; // end InterpretResult

    VirtualMachine();
    ~VirtualMachine() = default;

    VirtualMachine(const VirtualMachine&) = default;
    VirtualMachine& operator=(const VirtualMachine&) = default;

    VirtualMachine(VirtualMachine&&) = default;
    VirtualMachine& operator=(VirtualMachine&&) = default;

    InterpretResult Interpret(const Chunk& chunk);

private:
    void PrintVmStack();

    uint8_t ReadByte()
        { return chunk_.GetCode()[ip_++]; }

    value::value_t ReadConstant()
        { return chunk_.GetConstants()[ReadByte()]; }

    void BinaryOp(uint8_t op);

    InterpretResult Run();

    std::size_t                ip_;
    Chunk                      chunk_;
    std::stack<value::value_t> vm_stack_;
}; // end VirtualMachine
} // end lox
