# PrettyMemory Copilot Instructions

## Build and test commands

Use CMake from the repository root:

```powershell
cmake -S . -B build
cmake --build build
```

Run the full test suite with:

```powershell
ctest --test-dir build --output-on-failure
```

List discovered tests with:

```powershell
ctest --test-dir build -N
```

Run a single discovered test with:

```powershell
ctest --test-dir build -R <test-name> --output-on-failure
```

After building on Windows with the default Visual Studio generator, you can also run a single GoogleTest case directly:

```powershell
.\bin\Debug\PrettyMemoryTests.exe --gtest_filter=SuiteName.TestName
```

The test harness is wired up through GoogleTest and `test\test_main.cpp` contains the repository's current basic tests. When adding coverage, prefer extending that target instead of creating a second test executable.

## High-level architecture

This repository is a very small header-only C++17 library. The implementation currently lives entirely in `include\PrettyMemory.h` under the `PrettyMemory` namespace, with no separate `src\` directory or compiled library target in the top-level build.

The public API is centered on `PrettyMemory::MemoryPool`, which manages fixed-size blocks through an intrusive free list. When the free list is empty, `allocate()` grows the pool in batches of 32 blocks, rewires the linked free list inside that newly allocated region, and then returns blocks by popping from the list. `deallocate()` pushes blocks back onto the same list.

The top-level `CMakeLists.txt` only sets the C++ standard, output directories, and the `BUILD_TESTS` option. When tests are enabled, it delegates to `test\CMakeLists.txt`, which wraps the header-only library as an `INTERFACE` target named `PrettyMemory`, fetches GoogleTest with `FetchContent`, builds `PrettyMemoryTests`, and registers tests with `gtest_discover_tests()`.

Build outputs are intentionally split: the main executable output directory is `bin\`, while intermediate build products and fetched dependencies stay under `build\`.

## Key conventions

Keep library implementation inline in headers unless the repository structure changes intentionally. New public functionality should usually be added in `include\PrettyMemory.h` and consumed from tests through the existing include path instead of introducing a parallel source layout ad hoc.

Match the current C++ conventions:

- Use the `PrettyMemory` namespace for public types.
- Prefer standard library headers only; the library itself currently has no third-party runtime dependencies.
- Use classic include guards (`#ifndef` / `#define` / `#endif`) rather than `#pragma once`.
- Keep the project on C++17; both the root project and the tests explicitly require it.

Preserve the current testing pattern when adding coverage: test code lives under `test\`, links against the `PrettyMemory` `INTERFACE` target, and relies on GoogleTest discovery through CMake instead of hand-maintained test registration.
