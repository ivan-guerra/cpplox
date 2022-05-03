#pragma once

#include <climits>

#include "Value.h"

namespace lox
{

static constexpr int kFramesMax = 64; /*!< Max number of call frames. */
static constexpr int kStackMax  =
    kFramesMax * (UINT8_MAX + 1);     /*!< Max number of stack elements. */

struct ValueStack
{
    val::Value  stack[kStackMax];
    val::Value* stack_top;
}; // end ValueStack

extern struct ValueStack vm_stack;

void InitStack(ValueStack* vs);

void ResetStack(ValueStack* vs);

void Push(ValueStack* vs, const val::Value& value);

val::Value Pop(ValueStack* vs);

val::Value Peek(ValueStack* vs, int distance);

void PrintStack(ValueStack* vs);
} // end lox
