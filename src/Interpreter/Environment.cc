#include <any>
#include <string>

#include "Scanner.h"
#include "Environment.h"
#include "RuntimeError.h"

namespace lox
{
std::any Environment::Get(const Token& name) const
{
    if (env_.find(name.GetLexeme()) != env_.end())
        return env_.at(name.GetLexeme());

    std::string error_msg = std::string("Undefined variable '") +
                            name.GetLexeme() + std::string("'.");
    throw RuntimeError(name, error_msg);
}
} // end lox
