#pragma once

namespace lox
{
namespace value
{
    typedef double value_t; /*!< Representation of a Lox value. */

    /*!
     * \brief Print \a val to STDOUT.
     */
    void PrintValue(value_t val);
} // end value
} // end lox
