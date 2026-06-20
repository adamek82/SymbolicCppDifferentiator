#pragma once
#include <string>
#include <unordered_set>

/*
 * Simple tokenizer for numbers, identifiers, operators and functions.
 */

namespace sym {

enum class TokKind {
    End, Num, Id,
    Plus, Minus, Star, Slash, Caret,
    LParen, RParen, Comma,
    Func
};

struct Token {
    TokKind kind{};
    std::string text; // for Num/Id/Func
};

class Lexer {
public:
    explicit Lexer(std::string src);
    Token next();

private:
    std::string s_;
    size_t i_{0}, n_{0};

    static bool isIdentStart(char c);
    static bool isIdentCont(char c);
    void skipSpace();
};

} // namespace sym
