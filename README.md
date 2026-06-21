# SymbolicCppDifferentiator

A small C++ symbolic differentiator implemented with a recursive-descent parser.

The program parses mathematical expressions into an AST, differentiates them symbolically, and applies basic simplification rules.

## Documentation

- [Design document](docs/design.md)

The design document explains the high-level architecture of the project and describes the role of each module.

## Supported features

- variables and numeric constants
- operators: `+`, `-`, `*`, `/`, `^`
- parentheses
- functions: `sin`, `cos`, `tan`, `ln`, `log`, `exp`
- differentiation with respect to a selected variable
- basic simplification of derivative expressions

## Build

### Windows / MSYS2 UCRT64

Make sure MSYS2 UCRT64 is installed and that the MSYS2 binaries are available in `PATH` when running the compiler.

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;" + $env:PATH
C:/msys64/ucrt64/bin/mingw32-make.exe CXX=C:/msys64/ucrt64/bin/g++.exe debug
```

Run:

```powershell
.\build\debug\diff.exe
```

### Linux

```bash
make CXX=g++ debug
./build/debug/diff
```

## Example

```text
Differentiate with respect to (e.g., x): x
Enter expression: 5*x^3+6*x/y
Input (normalized): 5*x^3+6*x/y
Derivative:  0*x^3+5*3*1*x^2+((0*x+6*1)*y-6*x*0)/y^2
Simplified: 5*3*x^2+6/y
```

## Project layout

```text
SymbolicCppDifferentiator/
  include/
    ast.hpp
    lexer.hpp
    parser.hpp
    diff.hpp
    simplify.hpp
    print.hpp
  src/
    main.cpp
    lexer.cpp
    parser.cpp
    diff.cpp
    simplify.cpp
    print.cpp
  docs/
    design.md
  .vscode/
    c_cpp_properties.windows.example.json
    c_cpp_properties.linux.example.json
    launch.json
    settings.json
    tasks.json
  Makefile
  SymbolicCppDifferentiator.code-workspace
```

## Notes

The program is intentionally small and educational. Its simplifier performs only basic algebraic transformations, so derivative expressions may not always be reduced to the most compact mathematical form.
