# Core of a simple, generic Events system for modern C++.
# Platforms:
  - Windows
  - Mac (tested on M1 laptop using Clang 19)
  - Linux (not tested, but certainly should work)

## Dependencies:
A good C++20 compiler. That's all!

Look inside the src/EventingTest.cpp file for example usage.

### Features:
Modern C++.
Easy to understand.
Many, many callable types supported:
  - Free functions
  - Member functions (const or not)
  - Lambdas

#### Test Coverage:
ASAN (address-sanitizer) and UBSAN (undefined behaviour) tested in 'nix.
ASAN alone tested in MSVC (No other sanitizers avaiable in Windows)

GPL-3: Open Source


