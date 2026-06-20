#pragma once
#include "ast.hpp"

/*
 * Small algebraic simplifier:
 * - folds numeric ops, neutral elements
 * -  y/y = 1,  (A*y)/y = A,  (A*y)/y^n = A*y^(1-n), etc.
 * -  u^(-n) -> 1/(u^n)
 * -  A*(1/B) -> A/B
 */

namespace sym {
ExprPtr simplify(ExprPtr e);
} // namespace sym
