#include "parser.hpp"
#include <stdexcept>

namespace sym {

Parser::Parser(std::string src) : lex_(std::move(src)) {
    t_ = lex_.next();
}

void Parser::eat(TokKind k) {
    if (t_.kind != k)
        throw std::runtime_error("Syntax error");
    t_ = lex_.next();
}
bool Parser::accept(TokKind k) {
    if (t_.kind == k) {
        t_ = lex_.next();
        return true;
    }
    return false;
}

ExprPtr Parser::parse() {
    auto e = parseExpr();
    if (t_.kind != TokKind::End)
        throw std::runtime_error("Trailing garbage");
    return e;
}

ExprPtr Parser::parseExpr() {
    auto e = parseTerm();
    while (t_.kind == TokKind::Plus || t_.kind == TokKind::Minus) {
        char op = (t_.kind == TokKind::Plus) ? '+' : '-';
        t_ = lex_.next();
        auto r = parseTerm();
        e = std::make_unique<Bin>(op, std::move(e), std::move(r));
    }
    return e;
}

ExprPtr Parser::parseTerm() {
    auto e = parsePower();
    while (t_.kind == TokKind::Star || t_.kind == TokKind::Slash) {
        char op = (t_.kind == TokKind::Star) ? '*' : '/';
        t_ = lex_.next();
        auto r = parsePower();
        e = std::make_unique<Bin>(op, std::move(e), std::move(r));
    }
    return e;
}

ExprPtr Parser::parsePower() {
    auto b = parseUnary();
    if (t_.kind == TokKind::Caret) {
        eat(TokKind::Caret);
        auto e = parsePower(); // right-associativity
        return std::make_unique<Pow>(std::move(b), std::move(e));
    }
    return b;
}

ExprPtr Parser::parseUnary() {
    if (accept(TokKind::Plus))
        return parseUnary();
    if (accept(TokKind::Minus))
        return std::make_unique<Unary>('-', parseUnary());
    return parsePrimary();
}

ExprPtr Parser::parsePrimary() {
    if (t_.kind == TokKind::Num) {
        double v = std::stod(t_.text);
        eat(TokKind::Num);
        return std::make_unique<Num>(v);
    }
    if (t_.kind == TokKind::Id) {
        std::string id = t_.text;
        eat(TokKind::Id);
        return std::make_unique<Var>(id);
    }
    if (t_.kind == TokKind::Func) {
        std::string f = t_.text;
        eat(TokKind::Func);
        eat(TokKind::LParen);
        auto a = parseExpr();
        eat(TokKind::RParen);
        return std::make_unique<Fun>(f, std::move(a));
    }
    if (accept(TokKind::LParen)) {
        auto e = parseExpr();
        eat(TokKind::RParen);
        return e;
    }
    throw std::runtime_error("Expected number/identifier/function/'('");
}

} // namespace sym
