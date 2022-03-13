#pragma once

#include <any>
#include <string>
#include <unordered_map>

#include "Scanner.h"

namespace lox
{
class Environment
{
public:
    Environment() = default;
    ~Environment() = default;

    Environment(const Environment&) = default;
    Environment& operator=(const Environment&) = default;

    Environment(Environment&&) = default;
    Environment& operator=(Environment&&) = default;

    void Define(const std::string& name, const std::any& value)
        { env_[name] = value; }

    std::any Get(const Token& name) const;

    void Assign(Token name, const std::any& value);

private:
    std::unordered_map<std::string, std::any> env_;
}; // end Environment
} // end lox
