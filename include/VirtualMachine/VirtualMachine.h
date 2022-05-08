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
namespace vm
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
    using UpvaluePtr      = std::shared_ptr<obj::ObjUpvalue>;

    /*!
     * \struct CallFrame
     * \brief The CallFrame struct represents a function call frame.
     */
    struct CallFrame
    {
        std::shared_ptr<obj::ObjClosure> closure; /*!< Lox closure object representation. */
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
     * \brief Construct a new CallFrame and add it to the #frames_ stack.
     */
    bool Call(std::shared_ptr<obj::ObjClosure> closure, int arg_count);

    /*!
     * \brief Forward the \a callee to the appropriate call handler.
     *
     * The callee can be a native function, standard function call, method,
     * etc. CallValue() executes the appropriate handler type depending
     * on the type of callable object represented by \a callee.
     */
    bool CallValue(const val::Value& callee, int arg_count);

    /*!
     * \brief Return the next byte in \a frame's chunk.
     *
     * ReadByte() has the side effect of always incrementing the frame's ip to
     * point to the next instruction in the chunk.
     */
    uint8_t ReadByte(CallFrame* frame)
        { return frame->closure->function->chunk.GetInstruction(frame->ip++); }

    /*!
     * \brief Return a constant in the \a frame's chunk.
     *
     * ReadConstant() relies on ReadByte() to fetch the a constant. Undefined
     * behavior can arise when ReadConstant() is called when not parsing a
     * constant instruction's argument.
     */
    val::Value ReadConstant(CallFrame* frame)
        { return frame->closure->function->chunk.GetConstants()[ReadByte(frame)]; }

    /*!
     * \brief Return the 16-bit operand at \a frame's IP location.
     */
    uint16_t ReadShort(CallFrame* frame);

    /*!
     * \brief Return the current constant as a ObjString object.
     */
    std::shared_ptr<obj::ObjString> ReadString(CallFrame* frame)
        { return obj::AsString(ReadConstant(frame)); }

    /*!
     * \brief Concatenate the two string objects at the top of the stack.
     */
    void Concatenate();

    /*!
     * \brief Define a new native function.
     *
     * \param name Name of the new function.
     * \param function NativeFn function object implementing the new behavior.
     */
    void DefineNative(const std::string& name, obj::NativeFn function);

    /*!
     * \brief Close on an upvalue.
     */
    void CloseUpvalues(val::Value* last);

    /*!
     * \brief Capture an upvalue on \a local.
     */
    std::shared_ptr<obj::ObjUpvalue> CaptureUpvalue(val::Value* local);

    /*!
     * \brief Helper function used to evaluate binary operations.
     *
     * BinaryOp() pops the two values at the top of #vm_stack and applies
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
     * \brief Execute the bytecode within the active CallFrame.
     */
    InterpretResult Run();

    /* Note, this is a stacked based virtual machine meaning values are
       stored on a stack as the User program is executed. VirtualMachine
       implements its stack and defines a handle to it in Stack.h */
    InternedStrings strings_; /*!< Collection of interned strings. */
    Globals         globals_; /*!< Map of global names to their associated Value. */
    CallFrame       frames_[kFramesMax];  /*!< Stack of function call frames. */
    int             frame_count; /*!< Number of frames currently in the #frames_ array. */
    UpvaluePtr      open_upvalues_; /*!< Singly linked list of open upvalues. */
}; // end VirtualMachine

template <typename T>
VirtualMachine::InterpretResult VirtualMachine::BinaryOp(
    std::function<val::Value(T)> value_type,
    Chunk::OpCode op)
{
    if (!val::IsNumber(Peek(0)) ||
        !val::IsNumber(Peek(1))) {
        RuntimeError("Operands must be numbers.");
        return InterpretResult::kInterpretRuntimeError;
    }

    double b = val::AsNumber(Pop());
    double a = val::AsNumber(Pop());
    switch (op) {
        case Chunk::OpCode::kOpAdd:
            Push(value_type(a + b));
            break;
        case Chunk::OpCode::kOpSubtract:
            Push(value_type(a - b));
            break;
        case Chunk::OpCode::kOpMultiply:
            Push(value_type(a * b));
            break;
        case Chunk::OpCode::kOpDivide:
            Push(value_type(a / b));
            break;
        case Chunk::OpCode::kOpGreater:
            Push(value_type(a > b));
            break;
        case Chunk::OpCode::kOpLess:
            Push(value_type(a < b));
            break;
        default:
            RuntimeError("Unknown opcode");
            return InterpretResult::kInterpretRuntimeError;
    }
    return InterpretResult::kInterpretOk;
}
} // end vm
} // end lox
