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
        ret.append(std::any_cast<std::string>(GetValue(expr)));
    }
    ret.append(")");

    return ret;
}

void AstPrinter::VisitLiteralExpr(Literal& expr)
{
    if (!expr.value.has_value()) {
        Return(std::string("nil"));
        return;
    }

    if (typeid(std::string) == expr.value.type()) {
        Return(std::any_cast<std::string>(expr.value));
        return;
    }

    if (typeid(double) == expr.value.type()) {
        Return(std::to_string(std::any_cast<double>(expr.value)));
        return;
    }

    Return(std::string("unknown"));
}
} // end lox
