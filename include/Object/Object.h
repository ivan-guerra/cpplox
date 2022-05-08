#pragma once

#include <string>
#include <vector>
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
    kObjNative,   /*!< Lox native function type. */
    kObjClosure,  /*!< Closure type. */
    kObjUpvalue,  /*!< Closure upvalue type. */
    kObjClass,    /*!< Lox class type. */
    kObjInstance  /*!< Lox class instance type. */
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
    int        arity;         /*!< Number of arguments expected by the function. */
    int        upvalue_count; /*!< Number of upvalues referenced. */
    lox::Chunk chunk;         /*!< Chunk of bytecode representing the function body. */
    std::shared_ptr<ObjString> name; /*!< Source name of the function. */
}; // end ObjFunction

/*!
 * \struct ObjUpvalue
 * \brief The ObjUpvalue struct represents a local variable in an enclosing function.
 */
struct ObjUpvalue :
    public Obj
{
    val::Value* location; /*!< Location of upvalue on the stack. */
    val::Value  closed;   /*!< Copy of a closed upvalue. */
    std::shared_ptr<ObjUpvalue> next; /*!< Pointer to the next closed upvalue on the head. */
}; // end ObjUpvalue

/*!
 * \struct ObjClosure
 * \brief The ObjClosure struct represents a function closure.
 */
struct ObjClosure :
    public Obj
{
    std::shared_ptr<ObjFunction>             function; /*!< Closed function. */
    std::vector<std::shared_ptr<ObjUpvalue>> upvalues; /*!< Vector of upvalues referenced by this closure (see ObjUpvalue). */
    int                                      upvalue_count; /*!< Number of upvalues referenced by this closure. */
}; // end ObjClosure

using Table = std::unordered_map<std::shared_ptr<ObjString>, val::Value>;
/*!
 * \struct ObjClass
 * \brief The ObjClass struct represents a class object.
 */
struct ObjClass :
    public Obj
{
    std::shared_ptr<ObjString> name;    /*!< Class name mainly for error reporting. */
    Table                      methods; /*!< Map of class methods. */
}; // end ObjClass

/*!
 * \struct ObjInstance
 * \brief The ObjInstance struct represent class instances.
 */
struct ObjInstance :
    public Obj
{
    std::shared_ptr<ObjClass> klass;  /*!< Name of the class. */
    Table                     fields; /*!< Instance state data. */
}; // end ObjInstance

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
 * \brief Convert \a value to a ObjClosure object.
 */
std::shared_ptr<ObjClosure> AsClosure(const val::Value& value);

/*!
 * \brief Convert \a value to a ObjClass object.
 */
std::shared_ptr<ObjClass> AsClass(const val::Value& value);

/*!
 * \brief Convert \a value to a ObjInstance object.
 */
std::shared_ptr<ObjInstance> AsInstance(const val::Value& value);

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

/*!
 * \brief Return \c true if \a value is an ObjNative object.
 */
bool IsNative(const val::Value& value);

/*!
 * \brief Return \c true if \a value is an ObjClosure object.
 */
bool IsClosure(const val::Value& value);

/*!
 * \brief Return \c true if \a value is an ObjClass object.
 */
bool IsClass(const val::Value& value);

/*!
 * \brief Return \c true if \a value is an ObjInstance object.
 */
bool IsInstance(const val::Value& value);

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
 * \brief Return a pointer to a new ObjClosure object.
 */
std::shared_ptr<ObjClosure> NewClosure(std::shared_ptr<ObjFunction> function);

/*!
 * \brief Return a pointer to a new ObjUpvalue object.
 */
std::shared_ptr<ObjUpvalue> NewUpvalue(val::Value* slot);

/*!
 * \brief Return a pointer to a new ObjClass object.
 */
std::shared_ptr<ObjClass> NewClass(std::shared_ptr<ObjString> name);

/*!
 * \brief Return a pointer to a new ObjInstance object.
 */
std::shared_ptr<ObjInstance> NewInstance(std::shared_ptr<ObjClass> klass);

/*!
 * \brief Print the name of \a function to STDOUT.
 */
void PrintFunction(std::shared_ptr<ObjFunction> function);
} // end obj
} // end lox
