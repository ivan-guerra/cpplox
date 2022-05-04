#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#include "Value.h"
#include "Chunk.h"

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
    kObjString,   /*!< String type. */
    kObjFunction, /*!< Function type. */
    kObjNative    /*!< Lox native function type. */
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
    std::string chars; /*!< Character string wrapped by this ObjString. */
}; // end ObjString

/*!
 * \struct ObjFunction
 * \brief The ObjFunction struct represents Lox functions.
 */
struct ObjFunction :
    public Obj
{
    int        arity; /*!< Number of arguments expected by the function. */
    lox::Chunk chunk; /*!< Chunk of bytecode representing the function body. */
    std::shared_ptr<ObjString> name; /*!< Source name of the function. */
}; // end ObjFunction

using NativeFn = std::function<val::Value(int,val::Value*)>;

/*!
 * \struct ObjNative
 * \brief The ObjNative struct represent Lox native functions.
 */
struct ObjNative :
    public Obj
{
    NativeFn function; /*!< Function implementing the native function. */
}; // end ObjNative

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
 * \brief Convert \a value to a Lox ObjFunction.
 */
std::shared_ptr<ObjFunction> AsFunction(const val::Value& value);

/*!
 * \brief Convert \a value to a NativeFn function object.
 */
NativeFn AsNative(const val::Value& value);

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
 * \brief Return \c true if \a value is an ObjFunction object.
 */
bool IsFunction(const val::Value& value);

bool IsNative(const val::Value& value);

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

/*!
 * \brief Return a pointer to a 'blank slate' Lox function object.
 */
std::shared_ptr<ObjFunction> NewFunction();

/*!
 * \brief Return a pointer to a new native function.
 */
std::shared_ptr<ObjNative> NewNative(NativeFn function);

/*!
 * \brief Print the name of \a function to STDOUT.
 */
void PrintFunction(std::shared_ptr<ObjFunction> function);
} // end obj
} // end lox
