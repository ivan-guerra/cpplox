#include <iostream>

#include "Stack.h"
#include "Value.h"

namespace lox
{
namespace vm
{
struct ValueStack vm_stack;

void
ResetStack() { vm_stack.stack_top = vm_stack.stack; }

void
Push(const val::Value& value)
{
    *vm_stack.stack_top = value;
    vm_stack.stack_top++;
}

val::Value
Pop()
{
    vm_stack.stack_top--;
    return *vm_stack.stack_top;
}

val::Value
Peek(int distance) { return vm_stack.stack_top[-1 - distance]; }

void
PrintStack()
{
    std::cout << "          ";
    for (val::Value* slot = vm_stack.stack; slot < vm_stack.stack_top; slot++)
    {
        std::cout << "[ ";
        val::PrintValue(*slot);
        std::cout << " ]";
    }
    std::cout << std::endl;
}
} // end vm
} // end lox
