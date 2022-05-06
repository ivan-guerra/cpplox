#include <cstdio>
#include <string>
#include <variant>

#include "Object.h"

namespace lox
{
namespace obj
{
ObjType GetType(const val::Value& value)
    { return AsObj(value)->type; }

val::Value ObjVal(std::shared_ptr<Obj> value)
    { return val::Value{val::ValueType::kObj, value}; }

std::shared_ptr<Obj> AsObj(const val::Value& value)
    { return std::get<std::shared_ptr<Obj>>(value.as); }

std::shared_ptr<ObjString> AsString(const val::Value& value)
    { return std::static_pointer_cast<ObjString>(AsObj(value)); }

std::shared_ptr<ObjFunction> AsFunction(const val::Value& value)
    { return std::static_pointer_cast<ObjFunction>(AsObj(value)); }

NativeFn AsNative(const val::Value& value)
    { return std::static_pointer_cast<ObjNative>(AsObj(value))->function; }

std::shared_ptr<ObjClosure> AsClosure(const val::Value& value)
    { return std::static_pointer_cast<ObjClosure>(AsObj(value)); }

std::string AsStdString(const val::Value& value)
    { return std::static_pointer_cast<ObjString>(AsObj(value))->chars; }

bool IsObject(const val::Value& value)
    { return (value.type == val::ValueType::kObj); }

bool IsObjType(const val::Value& value, ObjType type)
    { return (IsObject(value) && AsObj(value)->type == type); }

bool IsString(const val::Value& value)
    { return IsObjType(value, ObjType::kObjString); }

bool IsFunction(const val::Value& value)
    { return IsObjType(value, ObjType::kObjFunction); }

bool IsNative(const val::Value& value)
    { return IsObjType(value, ObjType::kObjNative); }

bool IsClosure(const val::Value& value)
    { return IsObjType(value, ObjType::kObjClosure); }

std::shared_ptr<ObjString> CopyString(
    const std::string& str,
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<ObjString>>> strs)
{
    if (strs->find(str) != strs->end())
        return strs->at(str);

    std::shared_ptr<ObjString> str_obj = std::make_shared<ObjString>();
    str_obj->type  = ObjType::kObjString;
    str_obj->chars = str;

    /* Insert the newly formed ObjString into the intern string table. */
    (*strs)[str] = str_obj;

    return str_obj;
}

std::shared_ptr<ObjFunction> NewFunction()
{
    std::shared_ptr<ObjFunction> function = std::make_shared<ObjFunction>();
    function->type          = ObjType::kObjFunction;
    function->arity         = 0;
    function->upvalue_count = 0;
    function->name          = nullptr;

    return function;
}

std::shared_ptr<ObjNative> NewNative(NativeFn function)
{
    std::shared_ptr<ObjNative> native = std::make_shared<ObjNative>();
    native->type     = ObjType::kObjNative;
    native->function = function;

    return native;
}

std::shared_ptr<ObjClosure> NewClosure(std::shared_ptr<ObjFunction> function)
{
    std::shared_ptr<ObjClosure> closure = std::make_shared<ObjClosure>();
    closure->type     = ObjType::kObjClosure;
    closure->function = function;
    closure->upvalues =
        std::vector<std::shared_ptr<ObjUpvalue>>(
            function->upvalue_count, std::make_shared<ObjUpvalue>());
    closure->upvalue_count = function->upvalue_count;

    return closure;
}

std::shared_ptr<ObjUpvalue> NewUpvalue(val::Value* slot)
{
    std::shared_ptr<ObjUpvalue> upvalue = std::make_shared<ObjUpvalue>();
    upvalue->type     = ObjType::kObjUpvalue;
    upvalue->location = slot;
    upvalue->closed   = val::NilVal();
    upvalue->next     = nullptr;

    return upvalue;
}

void PrintFunction(std::shared_ptr<ObjFunction> function)
{
    if (!function->name) {
        std::printf("<script>");
        return;
    }
    std::printf("<fn %s>", function->name->chars.c_str());
}
} // end obj
} // end lox
