// src/main.cpp
#include <iostream>
#include <limits>
#include "parser.hpp"
#include "diff.hpp"
#include "simplify.hpp"
#include "print.hpp"

/*
 * Simple CLI:
 * - asks for variable to differentiate with respect to
 * - reads a single expression line
 * - prints normalized input, raw derivative, and simplified derivative
 */

int main(){
    using namespace sym;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::cout << "Differentiate with respect to (e.g., x): " << std::flush;
    std::string dx;
    if (!(std::cin >> dx)) return 0;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter expression: " << std::flush;
    std::string line;
    std::getline(std::cin, line);

    try{
        Parser P(line);
        auto ast = P.parse();

        Printer pr;
        std::cout << "Input (normalized): " << pr.print(ast.get()) << "\n";

        auto d = differentiate(ast.get(), dx);
        std::cout << "Derivative:  " << pr.print(d.get()) << "\n";

        auto s = simplify(std::move(d));
        std::cout << "Simplified: " << pr.print(s.get()) << "\n";
    }
    catch(const std::exception& ex){
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
