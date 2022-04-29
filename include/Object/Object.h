#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "Value.h"

namespace lox
{
namespace obj
{
/*!
 * \enum ObjType
 * \brief The ObjType enum names all the Lox object types.
 */
enum ObjType
{
    kObjString /*!< String type. */
}; // end ObjType

/*!
 * \struct Obj
 * \brief The Obj struct defines the base type for Lox objects.
 */
struct Obj
{
    ObjType type;
}; // end Obj

/*!
 * \struct ObjString
 * \brief The ObjString struct represents Lox strings.
 *
 * ObjString is a thin wrapper around C++'s std::string type.
 */
struct ObjString :
    public Obj
{
    std::string chars;
}; // end ObjString

/*!
 * \brief Return the ObjType of the object contained within \a value.
 */
ObjType GetType(const val::Value& value);

/*!
 * \brief Convert \a value to a Value with obj type info and data.
 */
val::Value ObjVal(std::shared_ptr<Obj> value);

/*!
 * \brief Convert \a value to a Lox object pointer.
 */
std::shared_ptr<Obj> AsObj(const val::Value& value);

/*!
 * \brief Convert \a value to a Lox ObjString.
 */
std::shared_ptr<ObjString> AsString(const val::Value& value);

/*!
 * \brief Convert \a value to Lox ObjString and return the underlying std::string.
 */
std::string AsStdString(const val::Value& value);

/*!
 * \brief Return \c true if \a value represents a Lox object.
 */
bool IsObject(const val::Value& value);

/*!
 * \brief Return \c true if \a value is an Object with type \a type.
 */
bool IsObjType(const val::Value& value, ObjType type);

/*!
 * \brief Return \c true if \a value is an ObjString object.
 */
bool IsString(const val::Value& value);

/*!
 * \brief Construct an ObjString initialized with \a str data.
 *
 * CopyString() will create a ObjString pointer and register it with \a strs
 * if \a str does not already exist in \a strs. Otherwise, a copy of \a str
 * is not performed an instead the existing pointer in \a strs is returned.
 *
 * \param str A string object identified by the Compiler.
 * \param strs Pointer to a map of std::string objects to
 *             std::shared_ptr<ObjString>. The \a strs map is used to identify
 *             whether \a str is already being tracked by the interpreter.
 */
std::shared_ptr<ObjString> CopyString(
    const std::string& str,
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<ObjString>>> strs);
} // end obj
} // end lox
