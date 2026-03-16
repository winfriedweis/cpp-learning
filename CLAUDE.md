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
  handlers/Registry.cpp \
  handlers/TwoSumHandler.cpp \
  handlers/AddTwoNumbersHandler.cpp \
  handlers/RayCasterHandler.cpp \
  ui/AnimationEngine.cpp \
  ui/MenuRenderer.cpp \
  ui/ProblemPresenter.cpp \
  algorithms/TwoSum.cpp \
  algorithms/AddTwoNumbers.cpp \
  games/RayCaster.cpp \
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
main.cpp                         ← entry point; wires registry + menu (no algo knowledge)
  │
  ├── handlers/Registry          ← list of all AlgoEntry{name, run} — only file to edit when adding
  │     ├── TwoSumHandler        ← input form + solve + present (self-contained)
  │     ├── AddTwoNumbersHandler
  │     └── RayCasterHandler
  │
  ├── ui/MenuRenderer            ← FTXUI interactive menu (controller)
  │     └── ui/AnimationEngine   ← Matrix rain, Neo intro, loading bar
  │
  ├── ui/ProblemPresenter        ← generic display utilities (formatArray, formatComplexity…)
  │
  └── algorithms/TwoSum, AddTwoNumbers   ← pure algorithms, zero UI (model)
      games/RayCaster
```

**Strict rule:** `algorithms/` has no UI includes. `ui/` never touches algorithm internals. `handlers/` files include both UI and algorithm headers — they are the glue layer. `main.cpp` knows only about the registry and menu.

Include paths: `main.cpp` uses `#include "ui/Foo.h"` and `#include "handlers/Bar.h"`. Handler files use `#include "ui/Foo.h"` and `#include "algorithms/Bar.h"` (project-root-relative). Files within `ui/` use flat includes (`#include "AnimationEngine.h"`).

## Adding a new algorithm

1. Create `algorithms/AlgoName.h/.cpp` — pure algorithm, no UI includes.
2. Create `handlers/AlgoNameHandler.h/.cpp` — input form, solve, present (use `ProblemPresenter` static helpers for formatting).
3. Add one line to `handlers/Registry.cpp` — the menu updates automatically, no changes to `main.cpp` needed.

## Naming conventions

- Classes: `PascalCase` · Methods: `camelCase` · Private members: `trailing_` underscore
- `#pragma once` in all headers · `using namespace ftxui` only in `.cpp` files
- Handler `.cpp` files use anonymous `namespace { }` for all file-scope private helpers (preferred over `static` for multiple items)
