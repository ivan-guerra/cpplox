#pragma once

#include <memory>
#include <variant>

namespace lox
{
/* Forward declaration of lox::obj objects to avoid a circular dependency
   between lox::value and lox::obj. */
namespace obj
{
    typedef struct Obj Obj;
    typedef struct ObjString ObjString;
} // end obj

namespace val
{
    /*!
     * \enum ValueType
     * \brief The ValueType enum defines the built-in Lox types.
     */
    enum ValueType
    {
        kBool,   /*!< Boolean. */
        kNil,    /*!< nil (i.e., NULL). */
        kNumber, /*!< Numerical. */
        kObj     /*!< Lox objects (e.g., strings, functions, etc.) */
    }; // end ValueType

    /*!
     * \struct Value
     * \brief The Value struct is a wrapper around a Lox built-in type.
     */
    struct Value
    {
        ValueType type;
        std::variant<bool, double, std::shared_ptr<obj::Obj>> as;
    }; // end Value

    /*!
     * \brief Convert \a value to a Value with boolean type info and data.
     */
    Value BoolVal(bool value);

    /*!
     * \brief Convert \a value to a Value with nil type info and data.
     */
    Value NilVal();

    /*!
     * \brief Convert \a value to a Value with number type info and data.
     */
    Value NumberVal(double value);

    /*!
     * \brief Return \c true if \a equals \a b.
     */
    bool ValuesEqual(const Value& a, const Value& b);

    /*!
     * \brief Convert \a value a to a C++ boolean type.
     */
    bool AsBool(const Value& value);

    /*!
     * \brief Convert \a value a to a C++ number type.
     */
    double AsNumber(const Value& value);

    /*!
     * \brief Return \c true if \a value represents a Lox boolean.
     */
    bool IsBool(const Value& value);

    /*!
     * \brief Return \c true if \a value represents a Lox's nil.
     */
    bool IsNil(const Value& value);

    /*!
     * \brief Return \c true if \a value represents a Lox number.
     */
    bool IsNumber(const Value& value);

    /*!
     * \brief Print the Lox object stored in \a value to STDOUT.
     */
    void PrintObject(const Value& value);

    /*!
     * \brief Print \a val to STDOUT.
     */
    void PrintValue(const Value& value);
} // end val
} // end lox
