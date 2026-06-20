#include "simplify.hpp"
#include <cmath>

namespace sym {

static bool isNumber(const Expr* e, double* out=nullptr){
    if(auto n=dynamic_cast<const Num*>(e)){ if(out) *out=n->v; return true; }
    return false;
}
static bool isOne(const Expr* e){ double v; return isNumber(e,&v) && v==1.0; }
static bool sameVar(const Expr* a, const Expr* b){
    auto va = asVar(a), vb = asVar(b);
    return va && vb && va->name == vb->name;
}

static ExprPtr simplifyRec(ExprPtr e);

static ExprPtr simplifyBin(char op, ExprPtr L, ExprPtr R){
    L = simplifyRec(std::move(L));
    R = simplifyRec(std::move(R));
    double a,b;

    if(op=='+'){
        if(isNumber(L.get(), &a) && isNumber(R.get(), &b)) return num(a+b);
        if(isNumber(L.get(), &a) && a==0) return std::move(R);
        if(isNumber(R.get(), &b) && b==0) return std::move(L);
        return std::make_unique<Bin>(op, std::move(L), std::move(R));
    }
    if(op=='-'){
        if(isNumber(L.get(), &a) && isNumber(R.get(), &b)) return num(a-b);
        if(isNumber(R.get(), &b) && b==0) return std::move(L);
        if(isNumber(L.get(), &a) && a==0) return std::make_unique<Unary>('-', std::move(R));
        return std::make_unique<Bin>(op, std::move(L), std::move(R));
    }
    if(op=='*'){
        if(isNumber(L.get(), &a) && isNumber(R.get(), &b)) return num(a*b);
        if(isNumber(L.get(), &a) && a==0) return num(0);
        if(isNumber(R.get(), &b) && b==0) return num(0);
        if(isNumber(L.get(), &a) && a==1) return std::move(R);
        if(isNumber(R.get(), &b) && b==1) return std::move(L);
        if(isNumber(L.get(), &a) && a==-1) return std::make_unique<Unary>('-', std::move(R));
        if(isNumber(R.get(), &b) && b==-1) return std::make_unique<Unary>('-', std::move(L));

        // A * (1/B)  ->  A / B
        if(auto rb = dynamic_cast<Bin*>(R.get()); rb && rb->op=='/' && isOne(rb->l.get()))
            return simplifyRec( divv(std::move(L), rb->r->clone()) );
        // (1/B) * A  ->  A / B
        if(auto lb = dynamic_cast<Bin*>(L.get()); lb && lb->op=='/' && isOne(lb->l.get()))
            return simplifyRec( divv(std::move(R), lb->r->clone()) );

        return std::make_unique<Bin>(op, std::move(L), std::move(R));
    }
    if(op=='/'){
        if(isNumber(L.get(), &a) && isNumber(R.get(), &b)) return num(a/b);
        if(isNumber(L.get(), &a) && a==0) return num(0);
        if(isNumber(R.get(), &b) && b==1) return std::move(L);

        // y / y -> 1
        if(sameVar(L.get(), R.get())) return num(1);

        // y / y^n -> y^(1-n)
        if(asVar(L.get())){
            if(auto pr = dynamic_cast<Pow*>(R.get())){
                if(asVar(pr->b.get()) && asVar(L.get())->name == asVar(pr->b.get())->name){
                    if(isNumber(pr->e.get(), &b))
                        return simplifyRec( poww(var(asVar(L.get())->name), num(1 - b)) );
                }
            }
        }

        // (A*y) / y  and  (y*A) / y
        if(auto lb = dynamic_cast<Bin*>(L.get()); lb && lb->op=='*'){
            if(asVar(lb->r.get()) && sameVar(lb->r.get(), R.get()))
                return simplifyRec(lb->l->clone());
            if(asVar(lb->l.get()) && sameVar(lb->l.get(), R.get()))
                return simplifyRec(lb->r->clone());

            // (A*y) / y^n  -> A * y^(1-n)
            if(asVar(lb->r.get())){
                if(auto pr = dynamic_cast<Pow*>(R.get())){
                    if(asVar(pr->b.get()) && asVar(lb->r.get())->name == asVar(pr->b.get())->name){
                        if(isNumber(pr->e.get(), &b))
                            return simplifyRec( mul(lb->l->clone(),
                                                    poww(var(asVar(lb->r.get())->name), num(1 - b))) );
                    }
                }
            }
            // (y*A) / y^n  -> A * y^(1-n)
            if(asVar(lb->l.get())){
                if(auto pr = dynamic_cast<Pow*>(R.get())){
                    if(asVar(pr->b.get()) && asVar(lb->l.get())->name == asVar(pr->b.get())->name){
                        if(isNumber(pr->e.get(), &b))
                            return simplifyRec( mul(lb->r->clone(),
                                                    poww(var(asVar(lb->l.get())->name), num(1 - b))) );
                    }
                }
            }
        }

        return std::make_unique<Bin>(op, std::move(L), std::move(R));
    }
    return std::make_unique<Bin>(op, std::move(L), std::move(R));
}

static ExprPtr simplifyPow(ExprPtr B, ExprPtr E){
    B = simplifyRec(std::move(B)); E = simplifyRec(std::move(E));
    double a,b;
    if(isNumber(E.get(), &b)){
        if(b==0) return num(1);
        if(b==1) return std::move(B);
        if(b<0)  // u^(-n) -> 1/(u^n)
            return simplifyRec( divv(num(1), poww(std::move(B), num(-b))) );
    }
    if(isNumber(B.get(), &a)){
        if(a==0) return num(0);
        if(a==1) return num(1);
    }
    return std::make_unique<Pow>(std::move(B), std::move(E));
}

static ExprPtr simplifyFun(std::string f, ExprPtr A){
    A = simplifyRec(std::move(A));

    // Constant folding for functions (when safe)
    if(auto n = dynamic_cast<Num*>(A.get())){
        double x = n->v;
        if(f=="sin") return num(std::sin(x));
        if(f=="cos") return num(std::cos(x));
        if(f=="tan") return num(std::tan(x));
        if(f=="ln")  return num(std::log(x));
        if(f=="log") return num(std::log10(x));
        if(f=="exp") return num(std::exp(x));
    }
    return std::make_unique<Fun>(std::move(f), std::move(A));
}

static ExprPtr simplifyRec(ExprPtr e){
    if(dynamic_cast<Num*>(e.get()) || dynamic_cast<Var*>(e.get())) return e;

    if(auto u = dynamic_cast<Unary*>(e.get())){
        auto a = simplifyRec(u->a->clone());
        if(auto inner = dynamic_cast<Unary*>(a.get())) // --x -> x
            if(inner->op=='-') return inner->a->clone();
        double v; if(isNumber(a.get(), &v)) return num(-v);
        return std::make_unique<Unary>('-', std::move(a));
    }
    if(auto b = dynamic_cast<Bin*>(e.get()))
        return simplifyBin(b->op, b->l->clone(), b->r->clone());
    if(auto p = dynamic_cast<Pow*>(e.get()))
        return simplifyPow(p->b->clone(), p->e->clone());
    if(auto f = dynamic_cast<Fun*>(e.get()))
        return simplifyFun(f->f, f->a->clone());
    return e;
}

ExprPtr simplify(ExprPtr e){ return simplifyRec(std::move(e)); }

} // namespace sym
