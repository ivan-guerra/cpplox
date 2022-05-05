#include <cstdio>
#include <variant>

#include "Value.h"
#include "Object.h"

namespace lox
{
namespace val
{
Value BoolVal(bool value)
    { return {ValueType::kBool, value}; }

Value NilVal()
    { return {ValueType::kNil, 0.0}; }

Value NumberVal(double value)
    { return {ValueType::kNumber, value}; }

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
        case ValueType::kObj:
            return (obj::AsObj(a) == obj::AsObj(b));
        default:
            /* Unreachable */
            return false;
    }
}

bool AsBool(const Value& value)
    { return std::get<bool>(value.as); }

double AsNumber(const Value& value)
    { return std::get<double>(value.as); }

bool IsBool(const Value& value)
    { return (value.type == ValueType::kBool); }

bool IsNil(const Value& value)
    { return (value.type == ValueType::kNil); }

bool IsNumber(const Value& value)
    { return (value.type == ValueType::kNumber); }

void PrintObject(const Value& value)
{
    switch (obj::GetType(value)) {
        case obj::ObjType::kObjString:
            std::printf("%s", obj::AsStdString(value).c_str());
            break;
        case obj::ObjType::kObjFunction:
            obj::PrintFunction(obj::AsFunction(value));
            break;
        case obj::ObjType::kObjNative:
            std::printf("<native fn>");
            break;
        case obj::ObjType::kObjClosure:
            obj::PrintFunction(obj::AsClosure(value)->function);
            break;
        case obj::ObjType::kObjUpvalue:
            std::printf("upvalue");
            break;
    }
}

void PrintValue(const Value& value)
{
    switch (value.type) {
        case ValueType::kBool:
            std::printf(AsBool(value) ? "true" : "false");
            break;
        case ValueType::kNil:
            std::printf("nil");
            break;
        case ValueType::kNumber:
            std::printf("%g", AsNumber(value));
            break;
        case ValueType::kObj:
            PrintObject(value);
            break;
    }
}
} // end val
} // end lox
