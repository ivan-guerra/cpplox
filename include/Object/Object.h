#pragma once

#include <string>
#include <memory>

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
ObjType GetType(const value::Value& value);

/*!
 * \brief Convert \a value to a Value with obj type info and data.
 */
value::Value ObjVal(std::shared_ptr<Obj> value);

/*!
 * \brief Convert \a value to a Lox object pointer.
 */
std::shared_ptr<Obj> AsObj(const value::Value& value);

/*!
 * \brief Convert \a value to a Lox ObjString.
 */
std::shared_ptr<ObjString> AsString(const value::Value& value);

/*!
 * \brief Convert \a value to Lox ObjString and return the underlying std::string.
 */
std::string AsStdString(const value::Value& value);

/*!
 * \brief Return \c true if \a value represents a Lox object.
 */
bool IsObject(const value::Value& value);

/*!
 * \brief Return \c true if \a value is an Object with type \a type.
 */
bool IsObjType(const value::Value& value, ObjType type);

/*!
 * \brief Return \c true if \a value is an ObjString object.
 */
bool IsString(const value::Value& value);

/*!
 * \brief Construct an ObjString initialized with \a str data.
 */
std::shared_ptr<ObjString> CopyString(const std::string& str);
} // end obj
} // end lox
