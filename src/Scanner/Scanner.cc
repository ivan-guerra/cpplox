#include <cctype>
#include <string>
#include <unordered_map>

#include "Scanner.h"

namespace lox
{
namespace scanr
{
const std::unordered_map<Token::TokenType, std::string>
Token::kTokenToStr_ =
{
    {Token::TokenType::kLeftParen,    "LeftParen"},
    {Token::TokenType::kRightParen,   "RightParen"},
    {Token::TokenType::kLeftBrace,    "LeftBrace"},
    {Token::TokenType::kRightBrace,   "RightBrace"},
    {Token::TokenType::kComma,        "Comma"},
    {Token::TokenType::kDot,          "Dot"},
    {Token::TokenType::kMinus,        "Minus"},
    {Token::TokenType::kPlus,         "Plus"},
    {Token::TokenType::kSemicolon,    "Semicolon"},
    {Token::TokenType::kSlash,        "Slash"},
    {Token::TokenType::kStar,         "Star"},
    {Token::TokenType::kBang,         "Bang"},
    {Token::TokenType::kBangEqual,    "BangEqual"},
    {Token::TokenType::kEqual,        "Equal"},
    {Token::TokenType::kEqualEqual,   "EqualEqual"},
    {Token::TokenType::kGreater,      "Greater"},
    {Token::TokenType::kGreaterEqual, "GreaterEqual"},
    {Token::TokenType::kLess,         "Less"},
    {Token::TokenType::kLessEqual,    "LessEqual"},
    {Token::TokenType::kIdentifier,   "Identifier"},
    {Token::TokenType::kString,       "String"},
    {Token::TokenType::kNumber,       "Number"},
    {Token::TokenType::kAnd,          "And"},
    {Token::TokenType::kClass,        "Class"},
    {Token::TokenType::kElse,         "Else"},
    {Token::TokenType::kFalse,        "False"},
    {Token::TokenType::kFor,          "For"},
    {Token::TokenType::kFun,          "Fun"},
    {Token::TokenType::kIf,           "If"},
    {Token::TokenType::kNil,          "Nil"},
    {Token::TokenType::kOr,           "Or"},
    {Token::TokenType::kPrint,        "Print"},
    {Token::TokenType::kReturn,       "Return"},
    {Token::TokenType::kSuper,        "Super"},
    {Token::TokenType::kThis,         "This"},
    {Token::TokenType::kTrue,         "True"},
    {Token::TokenType::kVar,          "Var"},
    {Token::TokenType::kWhile,        "While"},
    {Token::TokenType::kError,        "ERROR"},
    {Token::TokenType::kEof,          "EOF"}
};

Token::Token() :
    type_(TokenType::kEof),
    lexeme_(""),
    line_(0)
{

}

Token::Token(TokenType type, const std::string& lexeme, int line) :
    type_(type),
    lexeme_(lexeme),
    line_(line)
{

}

const std::unordered_map<std::string, Token::TokenType>
Scanner::kKeywords_ =
{
    {"and",    Token::TokenType::kAnd},
    {"class",  Token::TokenType::kClass},
    {"else",   Token::TokenType::kElse},
    {"false",  Token::TokenType::kFalse},
    {"for",    Token::TokenType::kFor},
    {"fun",    Token::TokenType::kFun},
    {"if",     Token::TokenType::kIf},
    {"nil",    Token::TokenType::kNil},
    {"or",     Token::TokenType::kOr},
    {"print",  Token::TokenType::kPrint},
    {"return", Token::TokenType::kReturn},
    {"super",  Token::TokenType::kSuper},
    {"this",   Token::TokenType::kThis},
    {"true",   Token::TokenType::kTrue},
    {"var",    Token::TokenType::kVar},
    {"while",  Token::TokenType::kWhile}
};

char
Scanner::Peek() const
{
    if (IsAtEnd())
        return '\0';

    return source_.at(current_);
}

char
Scanner::PeekNext() const
{
    if ((current_ + 1) >= source_.size())
        return '\0';

    return source_.at(current_ + 1);
}

bool
Scanner::Match(char expected)
{
    if (IsAtEnd())
        return false;

    if (expected != source_[current_])
        return false;

    current_++;
    return true;
}

void
Scanner::SkipWhitespace()
{
    while (true) {
        char c = Peek();
        switch(c) {
            case ' ':
            case '\r':
            case '\t':
                Advance();
                break;
            case '\n':
                line_++;
                Advance();
                break;
            case '/':
                if (PeekNext() == '/') {
                    while (!IsAtEnd() && (Peek() != '\n'))
                        Advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

Token
Scanner::String()
{
    while (!IsAtEnd() && Peek() != '"') {
        if (Peek() == '\n')
            line_++;
        Advance();
    }

    if (IsAtEnd())
        return ErrorToken("Unterminated string.");

    Advance();
    return MakeToken(Token::TokenType::kString);
}

Token
Scanner::Number()
{
    while (IsDigit(Peek()))
        Advance();

    /* Look for the fractional part. */
    if (Peek() == '.' && IsDigit(PeekNext()))
        /* Consume the '.'. */
        Advance();

    while (IsDigit(Peek()))
        Advance();

    return MakeToken(Token::TokenType::kNumber);
}

Token
Scanner::Identifier()
{
    while (IsAlpha(Peek()) || IsDigit(Peek()))
        Advance();

    std::string candidate = SourceSubstring(start_, current_);
    if (kKeywords_.find(candidate) != kKeywords_.end())
        return MakeToken(kKeywords_.at(candidate));

    return MakeToken(Token::TokenType::kIdentifier);
}

Scanner::Scanner(const std::string& source) :
    start_(0),
    current_(0),
    line_(1),
    source_(source)
{

}

Token
Scanner::ScanToken()
{
    SkipWhitespace();

    start_ = current_;
    if (IsAtEnd())
        return MakeToken(Token::TokenType::kEof);

    char c = Advance();

    if (IsAlpha(c))
        return Identifier();

    if (IsDigit(c))
        return Number();

    using TokenType = Token::TokenType;
    switch (c) {
        case '(': return MakeToken(TokenType::kLeftParen);
        case ')': return MakeToken(TokenType::kRightParen);
        case '{': return MakeToken(TokenType::kLeftBrace);
        case '}': return MakeToken(TokenType::kRightBrace);
        case ';': return MakeToken(TokenType::kSemicolon);
        case ',': return MakeToken(TokenType::kComma);
        case '.': return MakeToken(TokenType::kDot);
        case '-': return MakeToken(TokenType::kMinus);
        case '+': return MakeToken(TokenType::kPlus);
        case '/': return MakeToken(TokenType::kSlash);
        case '*': return MakeToken(TokenType::kStar);
        case '!':
            return MakeToken(
                Match('=') ? TokenType::kBangEqual: TokenType::kBang);
        case '=':
            return MakeToken(
                Match('=') ? TokenType::kEqualEqual : TokenType::kEqual);
        case '<':
            return MakeToken(
                Match('=') ? TokenType::kLessEqual : TokenType::kLess);
        case '>':
            return MakeToken(
                Match('=') ? TokenType::kGreaterEqual : TokenType::kGreater);
        case '"':
            return String();
    }
    return ErrorToken("Unexpected character.");
}
} // end scanr
} // end lox
