#include <string>
#include <memory>

#include "Expr.h"
#include "AstPrinter.h"

namespace lox
{
std::string AstPrinter::Parenthesize(const std::string& name,
                                     ExprList expressions) const
{
    std::string ret("(");
    ret.append(name);
    for (const auto& expr : expressions) {
        ret.append(" ");
        ret.append(GetValue(expr));
    }
    ret.append(")");

    return ret;
}
} // end lox
