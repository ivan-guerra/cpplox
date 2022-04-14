#include <cstdio>
#include <string>

#include "Scanner.h"
#include "Compiler.h"

namespace lox
{
void Compiler::Compile(const std::string& source)
{
    using Token = lox::Token;

    lox::Scanner scanner(source);
    int line = -1;
    while (true) {
        Token token = scanner.ScanToken();
        if (token.GetLine() != line) {
            std::printf("%4d ", token.GetLine());
            line = token.GetLine();
        } else {
            std::printf("   | ");
        }
        std::printf("%15s '%s'\n",
                    token.GetTypeStr().c_str(), token.GetLexeme().c_str());

        if (token.GetType() == Token::TokenType::kEof)
            break;
    }
}
} // end lox
