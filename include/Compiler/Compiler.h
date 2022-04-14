#pragma once

#include <string>

namespace lox
{
class Compiler
{
public:
    Compiler() = default;
    ~Compiler() = default;

    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    Compiler(Compiler&&) = delete;
    Compiler& operator=(Compiler&&) = delete;

    void Compile(const std::string& source);
}; // end Compiler
} // end lox
