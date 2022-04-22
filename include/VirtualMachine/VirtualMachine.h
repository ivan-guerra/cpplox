#pragma once

#include <stack>
#include <memory>
#include <functional>
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
     * \brief Compile and execute the code defined in \a source.
     */
    InterpretResult Interpret(const std::string& source);

private:
    /*!
     * \brief Helper function used to peek at the ith index in #vm_stack_.
     *
     * If \a i is out of bounds or invalid, Peek()'s behavior will be
     * undefined.
     *
     * \param i Index from the top of the stack where the desired Value
     *          resides.
     *
     * \return The Value object at the ith index in #vm_stack_.
     */
    value::Value Peek(int i);

    /*!
     * \brief Print #vm_stack_ to STDOUT.
     *
     * This is a simple VM debug utility method.
     */
    void PrintVmStack();

    /*!
     * \brief Print a runtime error message to STDOUT.
     */
    void RuntimeError(const char* format, ...);

    /*!
     * \brief Return \c true if \a value contains a \c false value.
     *
     * nil is considered to be false. The only other possibility is the \c
     * false literal.
     */
    bool IsFalsey(const value::Value& value) const
        { return IsNil(value) || (IsBool(value) && !AsBool(value)); }

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
    value::Value ReadConstant()
        { return chunk_->GetConstants()[ReadByte()]; }

    /*!
     * \brief Concatenate the two string objects at the top of the stack.
     */
    void Concatenate();

    /*!
     * \brief Helper function used to evaluate binary operations.
     *
     * BinaryOp() pops the two values at the top of #vm_stack_ and applies
     * the binary operation specified by \a op taking care to order the
     * operands correctly. If the values at the top of the stack are invalid
     * operands, a runtime error is reported.
     *
     * \param value_type Value conversion function applied to each operand.
     * \param op The binary operation opcode.
     *
     * \return InterpretResult::kInterpretRuntimeError if an error occurs, else
     *         InterpretResult::kInterpretOk is returned.
     */
    template <typename T>
    InterpretResult BinaryOp(
        std::function<value::Value(T)> value_type,
        Chunk::OpCode op);

    /*!
     * \brief Execute the bytecode within #chunk_ one instruction at a time.
     */
    InterpretResult Run();

    std::size_t              ip_;       /*!< Instruction pointer always pointing to the next, unprocessed instruction. */
    std::shared_ptr<Chunk>   chunk_;    /*!< Chunk of bytecode this VM will be interpreting. */
    std::stack<value::Value> vm_stack_; /*!< The value stack. */
    lox::Compiler            compiler_; /*!< Bytecode compiler. */
}; // end VirtualMachine

template <typename T>
VirtualMachine::InterpretResult VirtualMachine::BinaryOp(
    std::function<value::Value(T)> value_type,
    Chunk::OpCode op)
{
    if (!value::IsNumber(vm_stack_.top())) {
        RuntimeError("Operands must be numbers.");
        return InterpretResult::kInterpretRuntimeError;
    }
    double b = value::AsNumber(vm_stack_.top());
    vm_stack_.pop();

    if (!value::IsNumber(vm_stack_.top())) {
        RuntimeError("Operands must be numbers.");
        return InterpretResult::kInterpretRuntimeError;
    }
    double a = value::AsNumber(vm_stack_.top());
    vm_stack_.pop();

    switch (op) {
        case Chunk::OpCode::kOpAdd:
            vm_stack_.push(value_type(a + b));
            break;
        case Chunk::OpCode::kOpSubtract:
            vm_stack_.push(value_type(a - b));
            break;
        case Chunk::OpCode::kOpMultiply:
            vm_stack_.push(value_type(a * b));
            break;
        case Chunk::OpCode::kOpDivide:
            vm_stack_.push(value_type(a / b));
            break;
        case Chunk::OpCode::kOpGreater:
            vm_stack_.push(value_type(a > b));
            break;
        case Chunk::OpCode::kOpLess:
            vm_stack_.push(value_type(a < b));
            break;
        default:
            RuntimeError("Unknown opcode");
            return InterpretResult::kInterpretRuntimeError;
    }
    return InterpretResult::kInterpretOk;
}
} // end lox
