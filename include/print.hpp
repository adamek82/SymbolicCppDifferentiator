#pragma once
#include "ast.hpp"
#include <string>

/*
 * Pretty-printer that prefers division to  * u^(-1).
 * Precedence: +,- =1 ; *,/ =2 ; ^ =3 ; unary =4 ; atom =5
 */

namespace sym {

class Printer {
public:
    std::string print(const Expr* e, int prec=0) const;
};

} // namespace sym
