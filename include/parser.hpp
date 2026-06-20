#pragma once
#include "ast.hpp"
#include "lexer.hpp"

/*
 * Recursive-descent parser producing an AST.
 * Grammar (right-assoc power):
 *   Expr  := Term (('+'|'-') Term)*
 *   Term  := Power (('*'|'/') Power)*
 *   Power := Unary ('^' Power)?
 *   Unary := ('+'|'-') Unary | Primary
 *   Primary := Num | Id | Func '(' Expr ')' | '(' Expr ')'
 */

namespace sym {

class Parser {
public:
    explicit Parser(std::string src);
    ExprPtr parse();

private:
    Lexer lex_;
    Token t_;

    void eat(TokKind k);
    bool accept(TokKind k);

    ExprPtr parseExpr();
    ExprPtr parseTerm();
    ExprPtr parsePower();
    ExprPtr parseUnary();
    ExprPtr parsePrimary();
};

} // namespace sym
