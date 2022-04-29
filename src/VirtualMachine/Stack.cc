#include <iostream>

#include "Stack.h"
#include "Value.h"

namespace lox
{
template <>
void Stack<val::Value>::Print() const
{
    std::cout << "          ";
    for (std::size_t i = 0; i < stack_top_; ++i) {
        std::cout << "[ ";
        val::PrintValue(buffer_[i]);
        std::cout << " ]";
    }
    std::cout << std::endl;
}
} // end lox
