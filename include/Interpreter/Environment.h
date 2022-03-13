#pragma once

#include <any>
#include <string>
#include <unordered_map>
#include <memory>

#include "Scanner.h"

namespace lox
{
class Environment
{
public:
    Environment() : enclosing_(nullptr) { }
    Environment(const std::shared_ptr<Environment>& env) : enclosing_(env) { }
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

    std::shared_ptr<Environment> enclosing_;
}; // end Environment
} // end lox
