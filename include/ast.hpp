#pragma once
#include <memory>
#include <string>
#include <utility>
#include <stdexcept>

/*
 * Core AST node types + small helpers/factories.
 * Everything lives in namespace sym.
 */

namespace sym {

struct Expr {
    virtual ~Expr() = default;
    virtual std::unique_ptr<Expr> clone() const = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

struct Num : Expr {
    double v;
    explicit Num(double vv) : v(vv) {}
    ExprPtr clone() const override { return std::make_unique<Num>(v); }
};

struct Var : Expr {
    std::string name;
    explicit Var(std::string n) : name(std::move(n)) {}
    ExprPtr clone() const override { return std::make_unique<Var>(name); }
};

struct Unary : Expr {
    char op;      // currently only '-'
    ExprPtr a;
    Unary(char op, ExprPtr a) : op(op), a(std::move(a)) {}
    ExprPtr clone() const override { return std::make_unique<Unary>(op, a->clone()); }
};

struct Bin : Expr {
    char op;      // '+', '-', '*', '/'
    ExprPtr l, r;
    Bin(char op, ExprPtr l, ExprPtr r) : op(op), l(std::move(l)), r(std::move(r)) {}
    ExprPtr clone() const override { return std::make_unique<Bin>(op, l->clone(), r->clone()); }
};

struct Pow : Expr {
    ExprPtr b, e; // base ^ exponent
    Pow(ExprPtr b, ExprPtr e) : b(std::move(b)), e(std::move(e)) {}
    ExprPtr clone() const override { return std::make_unique<Pow>(b->clone(), e->clone()); }
};

struct Fun : Expr {
    std::string f; // "sin","cos","tan","ln","log","exp",...
    ExprPtr a;
    Fun(std::string f, ExprPtr a) : f(std::move(f)), a(std::move(a)) {}
    ExprPtr clone() const override { return std::make_unique<Fun>(f, a->clone()); }
};

// ---- small helpers/factories ----
inline ExprPtr num(double v) { return std::make_unique<Num>(v); }
inline ExprPtr var(std::string n) { return std::make_unique<Var>(std::move(n)); }
inline ExprPtr add(ExprPtr a, ExprPtr b){ return std::make_unique<Bin>('+', std::move(a), std::move(b)); }
inline ExprPtr sub(ExprPtr a, ExprPtr b){ return std::make_unique<Bin>('-', std::move(a), std::move(b)); }
inline ExprPtr mul(ExprPtr a, ExprPtr b){ return std::make_unique<Bin>('*', std::move(a), std::move(b)); }
inline ExprPtr divv(ExprPtr a, ExprPtr b){ return std::make_unique<Bin>('/', std::move(a), std::move(b)); }
inline ExprPtr neg(ExprPtr a){ return std::make_unique<Unary>('-', std::move(a)); }
inline ExprPtr poww(ExprPtr a, ExprPtr b){ return std::make_unique<Pow>(std::move(a), std::move(b)); }
inline ExprPtr fun(std::string f, ExprPtr a){ return std::make_unique<Fun>(std::move(f), std::move(a)); }

// Safe downcasts used by simplifier
inline const Var* asVar(const Expr* e) { return dynamic_cast<const Var*>(e); }

} // namespace sym
