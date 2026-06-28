#include "diff.hpp"
#include "simplify.hpp"
#include <cmath>
#include <utility>

namespace sym {

// small predicates
static bool isConst(const Expr* e, const std::string& dx) {
    if (dynamic_cast<const Num*>(e))
        return true;
    if (auto v = dynamic_cast<const Var*>(e))
        return v->name != dx;
    if (auto u = dynamic_cast<const Unary*>(e))
        return isConst(u->a.get(), dx);
    if (auto b = dynamic_cast<const Bin*>(e))
        return isConst(b->l.get(), dx) && isConst(b->r.get(), dx);
    if (auto p = dynamic_cast<const Pow*>(e))
        return isConst(p->b.get(), dx) && isConst(p->e.get(), dx);
    if (auto f = dynamic_cast<const Fun*>(e))
        return isConst(f->a.get(), dx);
    return false;
}
static bool isNumber(const Expr* e, double* out = nullptr) {
    if (auto n = dynamic_cast<const Num*>(e)) {
        if (out)
            *out = n->v;
        return true;
    }
    return false;
}

ExprPtr differentiate(const Expr* e, const std::string& dx) {
    if (dynamic_cast<const Num*>(e))
        return num(0);
    if (auto v = dynamic_cast<const Var*>(e))
        return num(v->name == dx ? 1.0 : 0.0);

    if (auto u = dynamic_cast<const Unary*>(e)) {
        if (u->op == '-')
            return neg(differentiate(u->a.get(), dx));
    }

    if (auto b = dynamic_cast<const Bin*>(e)) {
        if (b->op == '+')
            return add(differentiate(b->l.get(), dx), differentiate(b->r.get(), dx));
        if (b->op == '-')
            return sub(differentiate(b->l.get(), dx), differentiate(b->r.get(), dx));
        if (b->op == '*')
            return add(mul(differentiate(b->l.get(), dx), b->r->clone()),
                       mul(b->l->clone(), differentiate(b->r.get(), dx)));
        if (b->op == '/')
            return divv(sub(mul(differentiate(b->l.get(), dx), b->r->clone()),
                            mul(b->l->clone(), differentiate(b->r.get(), dx))),
                        poww(b->r->clone(), num(2)));
    }

    if (auto p = dynamic_cast<const Pow*>(e)) {
        double nval, aval;
        bool baseConst = isConst(p->b.get(), dx);
        bool expoConst = isConst(p->e.get(), dx);

        if (expoConst && isNumber(p->e.get(), &nval)) {
            // (u^n)' = n * u' * u^(n-1)
            return mul(mul(num(nval), differentiate(p->b.get(), dx)),
                       poww(p->b->clone(), num(nval - 1)));
        }
        if (baseConst && isNumber(p->b.get(), &aval) && aval > 0.0) {
            // (a^u)' = a^u * ln(a) * u'
            return mul(mul(poww(num(aval), p->e->clone()), fun("ln", num(aval))),
                       differentiate(p->e.get(), dx));
        }
        // generic: u^v = exp(v*ln(u))
        auto repl = fun("exp", mul(p->e->clone(), fun("ln", p->b->clone())));
        return differentiate(repl.get(), dx);
    }

    if (auto f = dynamic_cast<const Fun*>(e)) {
        auto du = differentiate(f->a.get(), dx);
        const std::string& fn = f->f;
        if (fn == "sin")
            return mul(fun("cos", f->a->clone()), std::move(du));
        if (fn == "cos")
            return mul(neg(fun("sin", f->a->clone())), std::move(du));
        if (fn == "tan")
            return mul(divv(num(1), poww(fun("cos", f->a->clone()), num(2))), std::move(du));
        if (fn == "ln")
            return mul(divv(num(1), f->a->clone()), std::move(du));
        if (fn == "log")
            return mul(divv(num(1), mul(f->a->clone(), fun("ln", num(10)))), std::move(du));
        if (fn == "exp")
            return mul(fun("exp", f->a->clone()), std::move(du));
        throw std::runtime_error("Unknown function: " + fn);
    }

    throw std::runtime_error("Unsupported AST node in differentiation");
}

} // namespace sym
