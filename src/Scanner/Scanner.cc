#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>

#include "Scanner.h"
#include "ErrorLogger.h"

namespace lox
{
static const std::unordered_map<Token::TokenType, std::string> kTokenToStr =
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
    {Token::TokenType::kEof,          "EOF"}
};

Token::Token(TokenType type, const std::string& lexeme,
             const std::string& literal, int line) :
    type_(type),
    lexeme_(lexeme),
    literal_(literal),
    line_(line)
{

}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << "(type:" << kTokenToStr.at(token.GetType())
        << ", lexeme:" << token.GetLexeme()
        << ", literal:";
    if (token.GetLiteral().empty())
        os << "NONE)";
    else
        os << token.GetLiteral() << ")";

    return os;
}

const std::unordered_map<std::string, Token::TokenType> Scanner::kKeywords_ =
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

void Scanner::AddToken(Token::TokenType type, const std::string& literal)
{
    std::string text = SourceSubstring(start_, current_);
    tokens_.emplace_back(type, text, literal, line_);
}

bool Scanner::Match(char expected)
{
    if (IsAtEnd())
        return false;

    if (source_[current_] != expected)
        return false;

    current_++;
    return true;
}

char Scanner::Peek() const
{
    if (IsAtEnd())
        return '\0';

    return source_.at(current_);
}

char Scanner::PeekNext() const
{
    if ((current_ + 1) >= source_.size())
        return '\0';

    return source_.at(current_ + 1);
}

void Scanner::String()
{
    while ((Peek() != '"') && !IsAtEnd()) {
        if (Peek() == '\n')
            line_++;
        Advance();
    }

    if (IsAtEnd()) {
        LOG_ERROR(line_, "unterminated string");
        return;
    }

    /* The closing ". */
    Advance();

    /* Trim the surrounding quotes. */
    std::string value = SourceSubstring(start_ + 1, current_ - 1);
    AddToken(Token::TokenType::kString, value);
}

void Scanner::Number()
{
    while (std::isdigit(Peek()))
        Advance();

    /* Look for a fractional part. */
    if ((Peek() == '.') && std::isdigit(PeekNext())) {
        /* Consume the "." */
        Advance();

        while (std::isdigit(Peek()))
            Advance();
    }

    AddToken(Token::TokenType::kNumber, SourceSubstring(start_, current_));
}

void Scanner::Identifier()
{
    while (IsAlphaNumeric(Peek()))
        Advance();

    std::string text = SourceSubstring(start_, current_);
    if (kKeywords_.find(text) == kKeywords_.end())
        AddToken(Token::TokenType::kIdentifier);
    else
        AddToken(kKeywords_.at(text));
}

void Scanner::Comment()
{
    bool     is_wellformed_comment = false;
    uint32_t comment_start_line    = line_;
    do {
        /* Determine whether we are at the close of a comment block. */
        if (('*' == Peek() && ('/' == PeekNext()))) {
            /* Consume the two closing comment chars. */
            Advance();
            Advance();

            /* Exit, we found a complete comment. */
            is_wellformed_comment = true;
            break;
        }

        if ('\n' == Advance())
            line_++;
    } while (!IsAtEnd());

    if (!is_wellformed_comment)
        LOG_ERROR(comment_start_line, "unterminated comment block");
}

void Scanner::ScanToken()
{
    using tt = Token::TokenType;
    char c = Advance();
    switch (c) {
        case '(':
            AddToken(tt::kLeftParen);
            break;
        case ')':
            AddToken(tt::kRightParen);
            break;
        case '{':
            AddToken(tt::kLeftBrace);
            break;
        case '}':
            AddToken(tt::kRightBrace);
            break;
        case ',':
            AddToken(tt::kComma);
            break;
        case '.':
            AddToken(tt::kDot);
            break;
        case '-':
            AddToken(tt::kMinus);
            break;
        case '+':
            AddToken(tt::kPlus);
            break;
        case ';':
            AddToken(tt::kSemicolon);
            break;
        case '*':
            AddToken(tt::kStar);
            break;
        case '!':
            AddToken(Match('=') ? tt::kBangEqual : tt::kBang);
            break;
        case '=':
            AddToken(Match('=') ? tt::kEqualEqual : tt::kEqual);
            break;
        case '<':
            AddToken(Match('=') ? tt::kLessEqual : tt::kLess);
            break;
        case '>':
            AddToken(Match('=') ? tt::kGreaterEqual : tt::kGreater);
            break;
        case '/':
            if (Match('/')) {
                /* A comment goes until the end of the line. */
                while ((Peek() != '\n') && !IsAtEnd())
                    Advance();
            } else if (Match('*')) {
                Comment();
            } else {
                AddToken(tt::kSlash);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            /* Ignore whitespace. */
            break;
        case '\n':
            line_++;
            break;
        case '"':
            String();
            break;
        default:
            if (std::isdigit(c)) {
                Number();
            } else if (IsAlpha(c)) {
                Identifier();
            } else {
                std::string err_msg = "unexpected character: ";
                err_msg += c;
                LOG_ERROR(line_, err_msg);
            }
            break;
    }
}

Scanner::Scanner(const std::string& source) :
    start_(0),
    current_(0),
    line_(1),
    source_(source)
{

}

std::vector<Token> Scanner::ScanTokens()
{
    while (!IsAtEnd()) {
        start_ = current_;
        ScanToken();
    }

    tokens_.emplace_back(Token::TokenType::kEof, "", "", line_);
    return tokens_;
}
} // end lox
