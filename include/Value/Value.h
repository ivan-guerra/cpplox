#pragma once

namespace lox
{
namespace value
{
    /*!
     * \enum ValueType
     * \brief The ValueType enum defines the built-in Lox types.
     */
    enum ValueType
    {
        kBool,  /*!< Boolean. */
        kNil,   /*!< nil (i.e., NULL). */
        kNumber /*!< Numerical. */
    }; // end ValueType

    /*!
     * \struct Value
     * \brief The Value struct is a wrapper around a Lox built-in type.
     */
    struct Value
    {
        ValueType type; /*!< Type of this Value. */

        /*!
         * \union as
         * \brief Union storing the underlying C++ representation of this value.
         */
        union
        {
            bool   boolean;
            double number;
        } as; // end as
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
     * \brief Print \a val to STDOUT.
     */
    void PrintValue(const Value& val);
} // end value
} // end lox
