#pragma once

#include <stack>
#include <memory>
#include <cstddef>
#include <cstdint>

#include "Value.h"
#include "Chunk.h"
#include "Compiler.h"

namespace lox
{
/*!
 * \class VirtualMachine
 * \brief The VirtualMachine class implements the bytecode executor.
 */
class VirtualMachine
{
public:
    /*!
     * \enum InterpretResult
     * \brief The InterpretResult enum captures the interpretation result of the executed instruction.
     */
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

    /*!
     * \brief Interpret the bytecode contained within \a chunk.
     */
    InterpretResult Interpret(const std::string& source);

private:
    /*!
     * \brief Print #vm_stack_ to STDOUT.
     *
     * This is a simple VM debug utility method.
     */
    void PrintVmStack();

    /*!
     * \brief Return the next byte in #chunk_.
     *
     * ReadByte() has the side effect of always incrementing #ip_ to point to
     * the next instruction in the chunk_.
     */
    uint8_t ReadByte()
        { return chunk_->GetCode()[ip_++]; }

    /*!
     * \brief Return a constant in #chunk_.
     *
     * ReadConstant() relies on ReadByte() to fetch the a constant. Undefined
     * behavior can arise when ReadConstant() is called when not parsing a
     * constant instruction's argument.
     */
    value::value_t ReadConstant()
        { return chunk_->GetConstants()[ReadByte()]; }

    /*!
     * \brief Helper function used to evaluate binary operations.
     *
     * BinaryOp() pops the two values at the top of #vm_stack_ and applies
     * the binary operation specified by \a op taking care to order the
     * operands correctly.
     */
    void BinaryOp(Chunk::OpCode op);

    /*!
     * \brief Execute the bytecode within #chunk_ one instruction at a time.
     */
    InterpretResult Run();

    std::size_t                ip_;       /*!< Instruction pointer always pointing to the next, unprocessed instruction. */
    std::shared_ptr<Chunk>     chunk_;    /*!< Chunk of bytecode this VM will be interpreting. */
    std::stack<value::value_t> vm_stack_; /*!< The value stack. */
    lox::Compiler              compiler_; /*!< Bytecode compiler. */
}; // end VirtualMachine
} // end lox
