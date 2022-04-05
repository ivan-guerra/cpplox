#include <cstdio>

#include "Value.h"

namespace lox
{
namespace value
{
void PrintValue(value_t val)
{
    std::printf("%g", val);
}
} // end value
} // end lox
