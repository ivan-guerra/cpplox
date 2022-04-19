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

bool ValuesEqual(const Value& a, const Value& b)
{
    if (a.type != b.type)
        return false;

    switch (a.type) {
        case ValueType::kBool:
            return (AsBool(a) == AsBool(b));
        case ValueType::kNil:
            return true;
        case ValueType::kNumber:
            return (AsNumber(a) == AsNumber(b));
        default:
            /* Unreachable */
            return false;
    }
}

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
    switch (val.type) {
        case ValueType::kBool:
            std::printf(AsBool(val) ? "true" : "false");
            break;
        case ValueType::kNil:
            std::printf("nil");
            break;
        case ValueType::kNumber:
            std::printf("%g", AsNumber(val));
            break;
    }
}
} // end value
} // end lox
