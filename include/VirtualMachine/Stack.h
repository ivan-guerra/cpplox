#pragma once

#include <vector>
#include <iostream>

#include "Value.h"

namespace lox
{

/*!
 * \class Stack
 * \brief The Stack class is stack data structure specialized for the VM.
 *
 * The Stack class operates as your typical stack data structure adding a
 * handful of utility methods that are used by the VirtualMachine. The latter
 * methods are inefficient to implement using std::stack hence the need to
 * define Stack.
 */
template <typename T>
class Stack
{
public:
    static const std::size_t kDefaultCapacity = 256; /*!< Default stack capacity. */

    /*!
     * \brief Construct a stack with storage capacity \a capacity.
     *
     * \param capacity Size of the stack.
     */
    Stack(std::size_t capacity=kDefaultCapacity);

    /* Default definitions suffice for the special class methods. */
    ~Stack() = default;
    Stack(const Stack&) = default;
    Stack& operator=(const Stack&) = default;
    Stack(Stack&&) = default;
    Stack& operator=(Stack&&) = default;

    /*!
     * \brief Return a read-only view of the item at the top of the stack.
     */
    const T& Top() const
        { return buffer_[stack_top_ - 1]; }

    /*!
     * \brief Get a read-only view of the item \a distance away from the top.
     *
     * \param distance Number of slots away from the stack top.
     */
    const T& Peek(int distance) const
        { return buffer_[stack_top_ - 1 - distance]; }

    /*!
     * \brief Push \a val into the stack.
     */
    void Push(const T& val)
        { buffer_[stack_top_++] = val; }

    /*!
     * \brief Set the item at index \a i to \a val.
     */
    void SetAt(int i, const T& val)
        { buffer_[i] = val; }

    /*!
     * \brief Pop and return the item at the top of the stack.
     */
    T Pop()
        { return buffer_[--stack_top_]; }

    /*!
     * \brief Clear the stack.
     */
    void Reset()
        { stack_top_ = 0; }

    /*!
     * \brief Print stack contents to STDOUT.
     */
    void Print() const;

private:
    std::size_t    stack_top_;
    std::vector<T> buffer_;
}; // end Stack

template <typename T>
Stack<T>::Stack(std::size_t capacity) :
    stack_top_(0),
    buffer_(capacity)
{

}

template <typename T>
void Stack<T>::Print() const
{
    std::cout << "          ";
    for (std::size_t i = 0; i < stack_top_; ++i) {
        std::cout << "[ ";
        std::cout << buffer_[i];
        std::cout << " ]";
    }
    std::cout << std::endl;
}

template <>
void Stack<val::Value>::Print() const;
} // end lox
