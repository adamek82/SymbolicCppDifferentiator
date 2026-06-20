#pragma once
#include "ast.hpp"
#include <string>

/*
 * Symbolic differentiation rules.
 * Supports: + - * / ^, sin cos tan ln log exp, and power cases:
 *   u^n, a^u, general u^v via exp(v*ln(u)).
 */

namespace sym {

ExprPtr differentiate(const Expr* e, const std::string& dx);

} // namespace sym
