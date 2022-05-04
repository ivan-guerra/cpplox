#pragma once

#include <climits>

#include "Value.h"

namespace lox
{
namespace vm
{
static constexpr int kFramesMax = 64; /*!< Max number of call frames. */
static constexpr int kStackMax  =
    kFramesMax * (UINT8_MAX + 1);     /*!< Max number of stack elements. */

/*!
 * \struct ValueStack
 * \brief The ValueStack struct defines a stack storing val::Value elements.
 */
struct ValueStack
{
    val::Value  stack[kStackMax]; /*!< Stack buffer. */
    val::Value* stack_top;        /*!< Pointer to the item at the stack top. */
}; // end ValueStack

extern struct ValueStack vm_stack; /*!< Global var representing the VM value stack. */

/*!
 * \brief Reset the stack.
 *
 * A reset sets the stack top to point at the stack base. There is no
 * actual deallocation/destruction of the Value objects stored in the stack
 * at the time ResetStack() is called.
 */
void ResetStack();

/*!
 * \brief Push \a value into the stack.
 */
void Push(const val::Value& value);

/*!
 * \brief Pop the Value at the top of the stack.
 *
 * Popping from an empty #vm_stack objects produces undefined behavior.
 */
val::Value Pop();

/*!
 * \brief Return the value \a distance slots back from the stack top.
 *
 * Calling Peek() with an invalid \a distance argument produces undefined
 * behavior.
 */
val::Value Peek(int distance);

/*!
 * \brief Print stack contents to STDOUT.
 */
void PrintStack();
} // end vm
} // end lox
