#include <any>
#include <string>
#include <memory>

#include "Scanner.h"
#include "Environment.h"
#include "RuntimeError.h"

namespace lox
{
Environment::EnvPtr Environment::Ancestor(int distance)
{
    EnvPtr env = shared_from_this();
    for (int i = 0; i < distance; ++i)
        env = env->enclosing_;

    return env;
}

std::any Environment::Get(const Token& name) const
{
    if (env_.find(name.GetLexeme()) != env_.end())
        return env_.at(name.GetLexeme());

    if (enclosing_)
        return enclosing_->Get(name);

    std::string error_msg = std::string("Undefined variable '") +
                            name.GetLexeme() + std::string("'.");
    throw RuntimeError(name, error_msg);
}

void Environment::Assign(const Token& name, const std::any& value)
{
    if (env_.find(name.GetLexeme()) != env_.end()) {
        env_[name.GetLexeme()] = value;
        return;
    }

    if (enclosing_) {
        enclosing_->Assign(name, value);
        return;
    }

    std::string error_msg = std::string("Undefined variable '" +
                                        name.GetLexeme() + "'.");
    throw RuntimeError(name, error_msg);
}
} // end lox
