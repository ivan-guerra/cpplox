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

std::string AsStdString(const val::Value& value)
    { return std::static_pointer_cast<ObjString>(AsObj(value))->chars; }

bool IsObject(const val::Value& value)
    { return (value.type == val::ValueType::kObj); }

bool IsObjType(const val::Value& value, ObjType type)
    { return (IsObject(value) && AsObj(value)->type == type); }

bool IsString(const val::Value& value)
    { return IsObjType(value, ObjType::kObjString); }

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
} // end obj
} // end lox
