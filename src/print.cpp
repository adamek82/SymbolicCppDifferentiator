#include "print.hpp"
#include <sstream>
#include <iomanip>

namespace sym {

std::string Printer::print(const Expr* e, int prec) const {
    if (auto n = dynamic_cast<const Num*>(e)) {
        std::ostringstream os;
        os << std::setprecision(15) << n->v;
        return os.str();
    }
    if (auto v = dynamic_cast<const Var*>(e))
        return v->name;
    if (auto u = dynamic_cast<const Unary*>(e)) {
        std::string s = "-" + print(u->a.get(), 4);
        return (4 < prec) ? ("(" + s + ")") : s;
    }
    if (auto b = dynamic_cast<const Bin*>(e)) {
        int my = (b->op == '+' || b->op == '-') ? 1 : 2;
        // right associativity tweak for '-' and '/'
        std::string s = print(b->l.get(), my) + std::string(1, b->op) +
                        print(b->r.get(), my + (b->op == '-' || b->op == '/' ? 1 : 0));
        return (my < prec) ? ("(" + s + ")") : s;
    }
    if (auto p = dynamic_cast<const Pow*>(e)) {
        int my = 3;
        std::string s = print(p->b.get(), my) + "^" + print(p->e.get(), my + 1);
        return (my < prec) ? ("(" + s + ")") : s;
    }
    if (auto f = dynamic_cast<const Fun*>(e)) {
        return f->f + "(" + print(f->a.get(), 0) + ")";
    }
    return "?";
}

} // namespace sym
