#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <cstddef>
#include <cstdint>

#include "Stack.h"
#include "Value.h"
#include "Object.h"
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
     * \brief The InterpretResult enum captures the bytecode interpretation result.
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
    using LoxString       = std::shared_ptr<obj::ObjString>;
    using LoxStringMap    =
        std::unordered_map<std::string, LoxString>;
    using InternedStrings = std::shared_ptr<LoxStringMap>;
    using Globals         =
        std::unordered_map<LoxString, val::Value>;

    static constexpr int kFramesMax = 64;                           /*!< Max number of call frames. */
    static constexpr int kStackMax  = kFramesMax * (UINT8_MAX + 1); /*!< Max number of values allowed on the VM stack. */

    /*!
     * \struct CallFrame
     * \brief The CallFrame struct represents a function call frame.
     */
    struct CallFrame
    {
        std::shared_ptr<obj::ObjFunction> function; /*!< Lox function object representation. */
        int          ip;    /*!< Instruction pointer. */
        val::Value*  slots; /*!< Call frame starting point on the VM value stack. */
    }; // end CallFrame

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
    bool IsFalsey(const val::Value& value) const
        { return IsNil(value) || (IsBool(value) && !AsBool(value)); }

    /*!
     * \brief Return the next byte in the active frame's chunk.
     *
     * ReadByte() has the side effect of always incrementing the frame's ip to
     * point to the next instruction in the chunk.
     */
    uint8_t ReadByte();

    /*!
     * \brief Return a constant in the active frame's chunk.
     *
     * ReadConstant() relies on ReadByte() to fetch the a constant. Undefined
     * behavior can arise when ReadConstant() is called when not parsing a
     * constant instruction's argument.
     */
    val::Value ReadConstant();

    /*!
     * \brief Return the 16-bit operand at the current frame's IP location.
     */
    uint16_t ReadShort();

    /*!
     * \brief Concatenate the two string objects at the top of the stack.
     */
    void Concatenate();

    /*!
     * \brief Helper function used to evaluate binary operations.
     *
     * BinaryOp() pops the two values at the top of #stack_ and applies
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
        std::function<val::Value(T)> value_type,
        Chunk::OpCode op);

    /*!
     * \brief Execute the bytecode within #chunk_ one instruction at a time.
     */
    InterpretResult Run();

    InternedStrings        strings_; /*!< Collection of interned strings. */
    Globals                globals_; /*!< Map of global names to their associated Value. */
    lox::Stack<CallFrame>  frames_;  /*!< Stack of function call frames. */
    lox::Stack<val::Value> stack_;   /*!< The value stack. */
}; // end VirtualMachine

template <typename T>
VirtualMachine::InterpretResult VirtualMachine::BinaryOp(
    std::function<val::Value(T)> value_type,
    Chunk::OpCode op)
{
    if (!val::IsNumber(stack_.Peek(0)) ||
        !val::IsNumber(stack_.Peek(1))) {
        RuntimeError("Operands must be numbers.");
        return InterpretResult::kInterpretRuntimeError;
    }

    double b = val::AsNumber(stack_.Pop());
    double a = val::AsNumber(stack_.Pop());
    switch (op) {
        case Chunk::OpCode::kOpAdd:
            stack_.Push(value_type(a + b));
            break;
        case Chunk::OpCode::kOpSubtract:
            stack_.Push(value_type(a - b));
            break;
        case Chunk::OpCode::kOpMultiply:
            stack_.Push(value_type(a * b));
            break;
        case Chunk::OpCode::kOpDivide:
            stack_.Push(value_type(a / b));
            break;
        case Chunk::OpCode::kOpGreater:
            stack_.Push(value_type(a > b));
            break;
        case Chunk::OpCode::kOpLess:
            stack_.Push(value_type(a < b));
            break;
        default:
            RuntimeError("Unknown opcode");
            return InterpretResult::kInterpretRuntimeError;
    }
    return InterpretResult::kInterpretOk;
}
} // end lox
