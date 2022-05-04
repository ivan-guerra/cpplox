#pragma once

#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace lox
{
namespace scanr
{
/*!
 * \class Token
 * \brief The Token class wraps token data read by the Scanner.
 */
class Token
{
public:
    /*!
     * \enum TokenType
     * \brief Lox token type.
     */
    enum class TokenType
    {
        /* Single-character tokens. */
        kLeftParen,
        kRightParen,
        kLeftBrace,
        kRightBrace,
        kComma,
        kDot,
        kMinus,
        kPlus,
        kSemicolon,
        kSlash,
        kStar,

        /* One or two character tokens. */
        kBang,
        kBangEqual,
        kEqual,
        kEqualEqual,
        kGreater,
        kGreaterEqual,
        kLess,
        kLessEqual,

        /* Literals. */
        kIdentifier,
        kString,
        kNumber,

        /* Keywords. */
        kAnd,
        kClass,
        kElse,
        kFalse,
        kFun,
        kFor,
        kIf,
        kNil,
        kOr,
        kPrint,
        kReturn,
        kSuper,
        kThis,
        kTrue,
        kVar,
        kWhile,

        kError,
        kEof
    }; // end TokenType

    Token();

    /*!
     * \brief Construct a token.
     *
     * \param type Token type.
     * \param lexeme Token lexeme.
     * \param line Line number at which this token appears in the source code.
     */
    Token(TokenType type, const std::string& lexeme, int line);

    ~Token() = default;

    Token(const Token&) = default;
    Token& operator=(const Token&) = default;

    Token(Token&&) = default;
    Token& operator=(Token&&) = default;

    /*!
     * \brief Return the type of this Token.
     */
    TokenType GetType() const { return type_; }

    /*!
     * \brief Return the type of this Token in string format.
     */
    std::string GetTypeStr() const { return kTokenToStr_.at(type_); }

    /*!
     * \brief Return the Token lexeme.
     */
    std::string GetLexeme() const { return lexeme_; }

    /*!
     * \brief Return the Token line number.
     */
    int GetLine() const { return line_; }

private:
    static const std::unordered_map<TokenType, std::string> kTokenToStr_; /*!< Map of TokenTypes to their corresponding string representation. */

    TokenType   type_;    /*!< Token type. */
    std::string lexeme_;  /*!< Token lexeme. */
    int         line_;    /*!< Line number at which this token appears. */
}; // end Token

/*!
 * \class Scanner
 * \brief The Scanner class implements the Lox lang source code scanner.
 *
 * Scanner takes as input a Lox source string and emits Token objects on
 * demand.
 */
class Scanner
{
public:
    Scanner() = delete;

    /*!
     * \brief Construct a Scanner object with reference to some source code.
     *
     * \param source Lox lang source code.
     */
    explicit Scanner(const std::string& source);

    ~Scanner() = default;

    Scanner(const Scanner&) = default;
    Scanner& operator=(const Scanner&) = default;
    Scanner(Scanner&&) = default;
    Scanner& operator=(Scanner&&) = default;

    /*!
     * \brief Return the next Token in the source code stream.
     * \return The next Token in the source code stream. If the end of the
     *         source code has been reached, a Token with TokenType EOF is
     *         returned.
     */
    Token ScanToken();

private:
    static const std::unordered_map<std::string, Token::TokenType> kKeywords_; /*!< Map of keyword strings to their Token::TokenType. */

    std::string SourceSubstring(int begin, int end) const
        { return source_.substr(begin, end - begin); }

    /*!
     * \brief Return \c true if the entire source text has been scanned.
     */
    bool IsAtEnd() const
        { return (current_ >= source_.size()); }

    /*!
     * \brief Return \c true if \a c is a digit.
     */
    bool IsDigit(char c) const
        { return ((c >= '0') && (c <='9')); }

    /*!
     * \brief Return \c true if \a c is alphabet character or underscore.
     */
    bool IsAlpha(char c) const
        {
            return (c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   c == '_';
        }

    /*!
     * \brief Return a Token with TokenType \a type.
     */
    Token MakeToken(Token::TokenType type) const
        { return Token(type, SourceSubstring(start_, current_), line_); }

    /*!
     * \brief Return a Token whose lexeme contains the error message \a message.
     */
    Token ErrorToken(const std::string& message) const
        { return Token(Token::TokenType::kError, message, line_); }

    /*!
     * \brief Advance the Scanner's source code cursor.
     * \return The character currently under the Scanner's cursor prior to
     *         updating the cursor.
     */
    char Advance()
        { return source_[current_++]; }

    /*!
     * \brief Return the character currentl under the Scanner's cursor.
     */
    char Peek() const;

    /*!
     * \brief Return the character one index past the current cursor position.
     * \return If the the scanner is at the end of the source text, the null
     *         character is returned.
     */
    char PeekNext() const;

    /*!
     * \brief Return \c true if the char under the cursor matches \a expected.
     */
    bool Match(char expected);

    /*!
     * \brief Consume whitespace from the current cursor position.
     */
    void SkipWhitespace();

    /*!
     * \brief Return a string type Token.
     */
    Token String();

    /*!
     * \brief Return a number Token.
     */
    Token Number();

    /*!
     * \brief Return an identifier Token.
     */
    Token Identifier();

    uint32_t    start_;   /*!< Source code start index. */
    uint32_t    current_; /*!< Current source code index. */
    uint32_t    line_;    /*!< Current line number. */
    std::string source_;  /*!< Source code string. */
}; // end Scanner
} // end scanr
} // end lox
