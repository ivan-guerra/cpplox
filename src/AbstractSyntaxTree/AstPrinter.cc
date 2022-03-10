#include <string>
#include <memory>

#include "Expr.h"
#include "AstPrinter.h"

namespace lox
{
std::string AstPrinter::Parenthesize(const std::string& name,
                                     ExprList expressions)
{
    std::string ret("(");
    ret.append(name);
    for (const auto& expr : expressions) {
        ret.append(" ");
        ret.append(std::any_cast<std::string>(expr->Accept(*this)));
    }
    ret.append(")");

    return ret;
}

std::any AstPrinter::VisitLiteralExpr(Literal& expr)
{
    if (!expr.value.has_value())
        return std::string("nil");

    if (typeid(std::string) == expr.value.type())
        return std::any_cast<std::string>(expr.value);

    if (typeid(double) == expr.value.type())
        return std::to_string(std::any_cast<double>(expr.value));

    return std::string("unknown");
}
} // end lox
