#include <iostream>

#include "Stack.h"
#include "Value.h"

namespace lox
{
namespace vm
{
struct ValueStack vm_stack;

void ResetStack(ValueStack* vs)
    { vs->stack_top = vs->stack; }

void Push(ValueStack* vs, const val::Value& value)
{
    *vs->stack_top = value;
    vs->stack_top++;
}

val::Value Pop(ValueStack* vs)
{
    vs->stack_top--;
    return *vs->stack_top;
}

val::Value Peek(ValueStack* vs, int distance)
    { return vs->stack_top[-1 - distance]; }

void PrintStack(ValueStack* vs)
{
    std::cout << "          ";
    for (val::Value* slot = vs->stack; slot < vs->stack_top; slot++) {
        std::cout << "[ ";
        val::PrintValue(*slot);
        std::cout << " ]";
    }
    std::cout << std::endl;
}
} // end vm
} // end lox
