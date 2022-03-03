#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cctype>
#include <unordered_map>

namespace lox
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

        kEof
    }; // end TokenType

    Token() = delete;

    /*!
     * \brief Construct a token.
     *
     * \param type Token type.
     * \param lexeme Token lexeme.
     * \param literal Literal value if any.
     * \param line Line number at which this token appears in the source code.
     */
    Token(TokenType type, const std::string& lexeme,
          const std::string& literal, int line);

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
     * \brief Return the Token lexeme.
     */
    std::string GetLexeme() const { return lexeme_; }

    /*!
     * \brief Return the Token literal.
     * \return If the token has a valid literal, that literal value is
     *         returned, else the empty string is returned.
     */
    std::string GetLiteral() const { return literal_; }

    /*!
     * \brief Return the Token line number.
     */
    int GetLine() const { return line_; }

    friend std::ostream& operator<<(std::ostream& os, const Token& token);

private:
    TokenType   type_;    /*!< Token type. */
    std::string lexeme_;  /*!< Token lexeme. */
    std::string literal_; /*!< Token literal (if any). */
    int         line_;    /*!< Line number at which this token appears. */
}; // end Token

/*!
 * \class Scanner
 * \brief The Scanner class implements the Lox lang source code scanner.
 *
 * Scanner takes as input a Lox source string and emits a vector of Token
 * objects. If an error is encountered, the Scanner will report the appropriate
 * error message via the ErrorLogger API. Scanner is a single use object. Once
 * you scan a source string, you must construct another Scanner object to scan
 * a subsequent string.
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
    Scanner(const std::string& source);
    ~Scanner() = default;

    Scanner(const Scanner&) = default;
    Scanner& operator=(const Scanner&) = default;

    Scanner(Scanner&&) = default;
    Scanner& operator=(Scanner&&) = default;

    /*!
     * \brief Scan the source code and emit Tokens.
     *
     * \return A vector of Token objects containing all tokens found by the
     *         Scanner.
     */
    std::vector<Token> ScanTokens();
private:
    static const std::unordered_map<std::string, Token::TokenType> kKeywords_; /*!< Map of keyword strings to their Token::TokenType. */

    /*!
     * \brief Return a substring of #source_ from [begin,end].
     */
    std::string SourceSubstring(int begin, int end) const
        { return source_.substr(begin, end - begin); }

    /*!
     * \brief Return \c true if \a c is an alphabetic char.
     *
     * Note, in this implementation, the '_' character is consider to be
     * alphabetic.
     */
    bool IsAlpha(char c) const { return (std::isalpha(c) || ('_' == c)); }

    /*!
     * \brief Return \c true if \a c is alphanumeric.
     */
    bool IsAlphaNumeric(char c) const
        { return (IsAlpha(c) || std::isdigit(c)); }

    /*!
     * \brief Return \c true if we have traversed the entire #source_ input.
     */
    bool IsAtEnd() const { return (current_ >= source_.size()); }

    /*!
     * \brief Return the current character and advance the #current_ index.
     */
    char Advance() { return source_[current_++]; }

    /*!
     * \brief Add token with type \a type and literal \a literal.
     */
    void AddToken(Token::TokenType type, const std::string& literal);

    /*!
     * \brief Add token with type \a type.
     */
    void AddToken(Token::TokenType type) { AddToken(type, ""); }

    /*!
     * \brief Return \c true if the current scan char matches \a expected.
     */
    bool Match(char expected);

    /*!
     * \brief Peek one character ahead in the input.
     */
    char Peek() const;

    /*!
     * \brief Peek two characters ahead in the input.
     */
    char PeekNext() const;

    /*!
     * \brief Consume a complete string token.
     */
    void String();

    /*!
     * \brief Consume a complete number token.
     */
    void Number();

    /*!
     * \brief Consume a complete identifier token.
     */
    void Identifier();

    /*!
     * \brief Consume a C-style comment block.
     *
     * Comment() does not emit any tokens. This method only scans over the
     * complete comment block taking no further action.
     */
    void Comment();

    /*!
     * \brief Implement the Lox lang scanner state machine.
     */
    void ScanToken();

    uint32_t           start_;   /*!< Source code start index. */
    uint32_t           current_; /*!< Current source code index. */
    uint32_t           line_;    /*!< Current line number. */
    std::string        source_;  /*!< Source code string. */
    std::vector<Token> tokens_;  /*!< Vector of scanned Tokens. */
}; // end Scanner
} // end lox
