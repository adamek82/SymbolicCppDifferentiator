# Design Document

This document describes the high-level design of `SymbolicCppDifferentiator`.

The project is intentionally small and educational. Its purpose is to show how a symbolic differentiator can be built from simple compiler-like components:

1. tokenization,
2. recursive-descent parsing,
3. AST construction,
4. symbolic AST transformation,
5. simplification,
6. pretty-printing.

The code is deliberately split into small modules so that each step can be studied independently.

## Goals

The main goals of the project are:

- parse mathematical expressions written as text,
- represent them as an abstract syntax tree,
- compute symbolic derivatives with respect to a selected variable,
- simplify the resulting expression using basic algebraic rules,
- print the final expression in a readable form,
- keep the implementation small enough to be understandable as a learning project.

The project is not intended to be a full computer algebra system. The simplifier is intentionally limited and local. It handles useful simple cases, but it does not try to perform global algebraic normalization.

## High-level architecture

The program follows this pipeline:

```text
input string
    |
    v
Lexer
    |
    v
tokens
    |
    v
Parser
    |
    v
AST
    |
    v
Differentiator
    |
    v
derivative AST
    |
    v
Simplifier
    |
    v
simplified AST
    |
    v
Printer
    |
    v
output string
```

Each stage has one main responsibility. The lexer does not know about calculus. The parser does not know about differentiation. The differentiator does not care how the expression was originally typed by the user. This separation is the central design idea of the project.

## Module overview

### AST

Files:

- `include/ast.hpp`

The AST module defines the expression tree.

It contains node types such as:

- `Num` for numeric constants,
- `Var` for variables,
- `Unary` for unary operators,
- `Bin` for binary operators,
- `Pow` for exponentiation,
- `Fun` for one-argument functions such as `sin`, `cos`, `ln`, and `exp`.

The project uses a classic polymorphic AST design: all expression nodes derive from the base class `Expr`.

Expressions are owned by `std::unique_ptr<Expr>`. This gives simple ownership semantics: every AST node owns its children, and there is no sharing unless a subtree is explicitly cloned.

The AST also provides small factory helpers such as `num()`, `var()`, `add()`, `mul()`, `poww()`, and `fun()`. These make transformation code easier to read.

A detailed AST document can later be added here:

- `docs/ast.md`

### Lexer

Files:

- `include/lexer.hpp`
- `src/lexer.cpp`

The lexer converts the input string into tokens.

For example, this expression:

```text
5*x^3+6*x/y
```

is converted into a sequence of tokens such as:

```text
Num(5), Star, Id(x), Caret, Num(3), Plus, Num(6), Star, Id(x), Slash, Id(y)
```

The lexer recognizes:

- numbers,
- identifiers,
- supported function names,
- arithmetic operators,
- parentheses,
- commas.

The lexer is intentionally simple. It does not build any tree and does not understand operator precedence. It only provides a clean stream of tokens for the parser.

A detailed lexer document can later be added here:

- `docs/lexer.md`

### Parser

Files:

- `include/parser.hpp`
- `src/parser.cpp`

The parser is implemented using recursive descent.

It consumes tokens produced by the lexer and builds an AST. Operator precedence is encoded directly in the parser functions.

The grammar is roughly:

```text
Expr    := Term (('+' | '-') Term)*
Term    := Power (('*' | '/') Power)*
Power   := Unary ('^' Power)?
Unary   := ('+' | '-') Unary | Primary
Primary := Num | Id | Func '(' Expr ')' | '(' Expr ')'
```

The `Power` rule is right-associative, so an expression like:

```text
a^b^c
```

is parsed as:

```text
a^(b^c)
```

This is the conventional mathematical interpretation of chained exponentiation.

A detailed parser document can later be added here:

- `docs/parser.md`

### Differentiator

Files:

- `include/diff.hpp`
- `src/diff.cpp`

The differentiator transforms one AST into another AST.

It implements symbolic differentiation rules such as:

- constant rule,
- variable rule,
- sum rule,
- difference rule,
- product rule,
- quotient rule,
- power rule,
- chain rule for supported functions.

For example:

```text
d/dx sin(x^2)
```

becomes an AST corresponding to:

```text
cos(x^2) * 2*x
```

The chain rule is implemented naturally because differentiating a function node also differentiates its argument.

The differentiator currently supports:

- `sin`,
- `cos`,
- `tan`,
- `ln`,
- `log`,
- `exp`.

For powers, it supports important cases such as:

- `u^n`,
- `a^u`,
- general `u^v` through rewriting into `exp(v * ln(u))`.

A detailed differentiator document can later be added here:

- `docs/differentiator.md`

### Simplifier

Files:

- `include/simplify.hpp`
- `src/simplify.cpp`

The simplifier applies local algebraic cleanup rules.

Examples:

```text
0 + x      -> x
x * 1      -> x
x * 0      -> 0
x / 1      -> x
x^1        -> x
x^0        -> 1
y / y      -> 1
(A*y)/y^2  -> A/y
```

The simplifier is intentionally not a full algebraic normalization engine. It does not yet canonicalize all products, combine all constants in nested expressions, or reorder terms.

For example, it may keep:

```text
5*3*x^2
```

instead of reducing it to:

```text
15*x^2
```

unless a specific simplification rule handles that exact shape.

This is a good place for future improvements, because symbolic simplification can grow into a large topic on its own.

A detailed simplifier document can later be added here:

- `docs/simplifier.md`

### Printer

Files:

- `include/print.hpp`
- `src/print.cpp`

The printer converts an AST back into a textual expression.

It uses operator precedence to avoid unnecessary parentheses while preserving meaning.

For example, it can print:

```text
5*x^3+6*x/y
```

instead of:

```text
((5*(x^3))+((6*x)/y))
```

The printer is not responsible for algebraic simplification. It only decides how to represent the current AST as a string.

A detailed printer document can later be added here:

- `docs/printer.md`

### Command-line interface

Files:

- `src/main.cpp`

The CLI is intentionally minimal.

It asks the user for:

1. the variable to differentiate with respect to,
2. the expression to differentiate.

It then prints:

1. normalized input,
2. raw derivative,
3. simplified derivative.

This module wires together all other modules:

```text
Parser -> differentiate() -> simplify() -> Printer
```

A detailed CLI document can later be added here:

- `docs/cli.md`

## Why this design uses `dynamic_cast`

The AST is represented as a class hierarchy with a virtual base class and derived node classes.

Operations such as differentiation, simplification, and printing need to inspect the concrete node type. In this project this is done with `dynamic_cast`.

For a small educational project, this is acceptable and keeps the code straightforward. It avoids adding the boilerplate required by the Visitor pattern.

Alternative designs are possible:

- Visitor pattern,
- `std::variant` with `std::visit`,
- virtual methods such as `Expr::differentiate()` and `Expr::print()`.

The current approach keeps AST node definitions simple and keeps operations in separate modules.

If the project grows significantly, the Visitor pattern or a `std::variant`-based AST could be considered.

## Current limitations

The project currently has several intentional limitations:

- functions are limited to a small predefined set,
- simplification is local and heuristic,
- expressions are printed in a readable but not fully canonical form,
- there is no formal test suite yet,
- error reporting is simple,
- all numeric constants are represented as `double`.

These limitations are acceptable for the current goal: a compact, understandable symbolic differentiator.

## Possible future work

Potential future improvements:

- add unit tests,
- add more functions such as `sqrt`, `asin`, `acos`, `atan`,
- improve simplification of products and powers,
- combine numeric factors in nested products,
- normalize signs more consistently,
- add expression equivalence tests,
- add a REPL mode,
- add detailed documentation for each module,
- consider replacing `dynamic_cast` with Visitor or `std::variant` if the project grows.

## Documentation plan

This document is the top-level design overview.

Suggested future documentation files:

```text
docs/
  design.md
  ast.md
  lexer.md
  parser.md
  differentiator.md
  simplifier.md
  printer.md
  cli.md
```

The detailed module documents should describe implementation decisions without turning the source files into long tutorial texts. This keeps the code readable and keeps educational explanations in Markdown files.
