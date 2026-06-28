#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

namespace sym {

Lexer::Lexer(std::string src) : s_(std::move(src)), n_(s_.size()) {}

bool Lexer::isIdentStart(char c) {
    return std::isalpha((unsigned char)c) || c == '_';
}
bool Lexer::isIdentCont(char c) {
    return std::isalnum((unsigned char)c) || c == '_';
}

void Lexer::skipSpace() {
    while (i_ < n_ && std::isspace((unsigned char)s_[i_]))
        ++i_;
}

Token Lexer::next() {
    skipSpace();
    if (i_ >= n_)
        return {TokKind::End, ""};
    char c = s_[i_];

    // number
    if (std::isdigit((unsigned char)c) ||
        (c == '.' && i_ + 1 < n_ && std::isdigit((unsigned char)s_[i_ + 1]))) {
        size_t j = i_;
        if (s_[i_] == '.')
            ++i_;
        while (i_ < n_ && std::isdigit((unsigned char)s_[i_]))
            ++i_;
        if (i_ < n_ && s_[i_] == '.') {
            ++i_;
            while (i_ < n_ && std::isdigit((unsigned char)s_[i_]))
                ++i_;
        }
        if (i_ < n_ && (s_[i_] == 'e' || s_[i_] == 'E')) {
            size_t k = i_ + 1;
            if (k < n_ && (s_[k] == '+' || s_[k] == '-'))
                ++k;
            if (k < n_ && std::isdigit((unsigned char)s_[k])) {
                i_ = k;
                while (i_ < n_ && std::isdigit((unsigned char)s_[i_]))
                    ++i_;
            }
        }
        return {TokKind::Num, s_.substr(j, i_ - j)};
    }

    // identifier or function
    if (isIdentStart(c)) {
        size_t j = i_++;
        while (i_ < n_ && isIdentCont(s_[i_]))
            ++i_;
        std::string id = s_.substr(j, i_ - j);
        std::string low = id;
        for (char& ch : low)
            ch = (char)std::tolower((unsigned char)ch);
        static const std::unordered_set<std::string> funs{"sin", "cos", "tan", "ln", "log", "exp"};
        if (funs.count(low))
            return {TokKind::Func, low};
        return {TokKind::Id, id};
    }

    ++i_;
    switch (c) {
    case '+':
        return {TokKind::Plus, "+"};
    case '-':
        return {TokKind::Minus, "-"};
    case '*':
        return {TokKind::Star, "*"};
    case '/':
        return {TokKind::Slash, "/"};
    case '^':
        return {TokKind::Caret, "^"};
    case '(':
        return {TokKind::LParen, "("};
    case ')':
        return {TokKind::RParen, ")"};
    case ',':
        return {TokKind::Comma, ","};
    default:
        throw std::runtime_error(std::string("Unknown character: ") + c);
    }
}

} // namespace sym
