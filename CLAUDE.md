# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

**Dependency:** FTXUI terminal UI library.
```bash
brew install ftxui
```

### Command-line (from `cpp-learning/` directory)

```bash
g++ -std=c++20 \
  main.cpp \
  ui/AnimationEngine.cpp \
  ui/MenuRenderer.cpp \
  ui/ProblemPresenter.cpp \
  algorithms/TwoSum.cpp \
  -I/opt/homebrew/include \
  -I. \
  -L/opt/homebrew/lib \
  -lftxui-component -lftxui-dom -lftxui-screen \
  -o neo

./neo
```

**Important:** Run in a real terminal (Terminal.app), never in Xcode's console — FTXUI requires a proper TTY.

### Xcode

The `.xcodeproj` uses `PBXFileSystemSynchronizedRootGroup` which auto-discovers all `.cpp` files under `cpp-learning/`. No manual "Compile Sources" management needed — just keep the directory clean (one `main()`).

Build: `⌘B` · Run in terminal: find binary under `DerivedData/…/Build/Products/Debug/cpp-learning`

## Architecture

```
main.cpp                         ← entry point; wires the three layers
  │
  ├── ui/MenuRenderer            ← FTXUI interactive menu (controller)
  │     └── ui/AnimationEngine   ← Matrix rain, Neo intro, loading bar, Morpheus
  │
  ├── ui/ProblemPresenter        ← formats + displays algorithm results (view)
  │     └── ui/AnimationEngine
  │
  └── algorithms/TwoSum          ← pure algorithm, zero UI dependencies (model)
```

**Strict rule:** `algorithms/` has no UI includes. `ui/` never touches algorithm internals. `main.cpp` is the only file that crosses layers.

Include paths: `main.cpp` uses `#include "ui/Foo.h"` and `#include "algorithms/Bar.h"`. Files within a subdirectory use flat includes (`#include "AnimationEngine.h"`).

## Adding a new algorithm

1. Create `algorithms/AlgoName.h` / `.cpp` with `solve()`, `getName()`, `getTimeComplexity()`, `getSpaceComplexity()`. No UI includes.
2. Add a `handleAlgoName()` static function in `main.cpp` that creates the algorithm, calls `solve()`, and passes results to a `ProblemPresenter` method.
3. Add the menu entry string to `MenuRenderer::MenuRenderer()` (before "Beenden").
4. Add a `case` to the `switch` in `main.cpp`.

## Naming conventions

- Classes: `PascalCase` · Methods: `camelCase` · Private members: `trailing_` underscore
- `#pragma once` in all headers · `using namespace ftxui` only in `.cpp` files
