#include <string>
#include <variant>

#include "Object.h"

namespace lox
{
namespace obj
{
ObjType GetType(const value::Value& value)
    { return AsObj(value)->type; }

value::Value ObjVal(std::shared_ptr<Obj> value)
    { return value::Value{value::ValueType::kObj, value}; }

std::shared_ptr<Obj> AsObj(const value::Value& value)
    { return std::get<std::shared_ptr<Obj>>(value.as); }

std::shared_ptr<ObjString> AsString(const value::Value& value)
    { return std::static_pointer_cast<ObjString>(AsObj(value)); }

std::string AsStdString(const value::Value& value)
    { return std::static_pointer_cast<ObjString>(AsObj(value))->chars; }

bool IsObject(const value::Value& value)
    { return (value.type == value::ValueType::kObj); }

bool IsObjType(const value::Value& value, ObjType type)
    { return (IsObject(value) && AsObj(value)->type == type); }

bool IsString(const value::Value& value)
    { return IsObjType(value, ObjType::kObjString); }

std::shared_ptr<ObjString> CopyString(const std::string& str)
{
    std::shared_ptr<ObjString> str_obj = std::make_shared<ObjString>();
    str_obj->type  = ObjType::kObjString;
    str_obj->chars = str;

    return str_obj;
}
} // end obj
} // end lox
