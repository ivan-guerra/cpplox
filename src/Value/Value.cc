#include <cstdio>

#include "Value.h"

namespace lox
{
namespace value
{
Value BoolVal(bool value)
    { return {ValueType::kBool, {.boolean = value}}; }

Value NilVal()
    { return {ValueType::kNil, {.number = 0}}; }

Value NumberVal(double value)
    { return {ValueType::kNumber, {.number = value}}; }

bool AsBool(const Value& value)
    { return value.as.boolean; }

double AsNumber(const Value& value)
    { return value.as.number; }

bool IsBool(const Value& value)
    { return (value.type == ValueType::kBool); }

bool IsNil(const Value& value)
    { return (value.type == ValueType::kNil); }

bool IsNumber(const Value& value)
    { return (value.type == ValueType::kNumber); }

void PrintValue(const Value& val)
{
    std::printf("%g", AsNumber(val));
}
} // end value
} // end lox
