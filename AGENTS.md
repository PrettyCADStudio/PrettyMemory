# AGENTS.md

## Project overview

Header-only C++17/C++20 smart pointer library. The entire implementation lives in a single file: `include/PrettyMemory.h`. There is no `src/` directory or compiled library target.

Public API types (namespace `prtm`):
- `OwnerPtr<T, Deleter>` — owning smart pointer, move-only, custom deleter support
- `ShadowPtr<T>` — non-owning observer that detects when the target is destroyed
- `EnableShadowFromThis<T>` — CRTP base for creating `ShadowPtr` from `this`

Internal implementation lives in `prtm::detail`.

## Build and test

Requires: CMake 3.28+, C++17 compiler. Windows with Visual Studio is the primary platform.

```powershell
# Configure + build + test (standard CMake workflow)
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

# Run a single test by name pattern
ctest --test-dir build -R ShadowPtrTest --output-on-failure

# List discovered tests without running
ctest --test-dir build -N
```

CI builds in Release with `-G "Visual Studio 17 2022" -A x64`.

### Pipenv scripts (Windows)

Python helper scripts wrap CMake. Default build config is `RelWithDebInfo` (not Release):

```powershell
pipenv run build              # build (RelWithDebInfo)
pipenv run test               # test
pipenv run build_release      # build Release
pipenv run test_release       # test Release
pipenv run clean              # remove build/, bin/, .vs/, .idea/
pipenv run doc                # generate Doxygen docs
```

## Test structure

Tests use GoogleTest (v1.14.0, fetched via `FetchContent`). All test sources under `test/` are compiled into a single `PrettyMemoryTests` executable.

Custom test macros in `test/PrettyMemoryTest.h`:
- `DEFINE_TEST_BEGIN(SuiteName, TestName, Subject)` / `DEFINE_TEST_END` — wraps GoogleTest with automatic leak checking via `CountableObject::Balance`. Every test MUST end with `DEFINE_TEST_END` which asserts `Balance == 0`.
- `CountableObject<T>` (in `test/CountableObject.h`) — tracks construction/destruction counts via a static `Balance` counter. Use as the test type to verify no leaks.

Test files: `OwnerPtr.Test.cpp`, `ShadowPtr.Test.cpp`, `EnableShadowFromThis.Test.cpp`, `ControlBlock.Test.cpp`, `CountableObject.Test.cpp`, `Deleter.Test.cpp`.

## Code conventions

- Namespace: `prtm` (public), `prtm::detail` (internal). NOT `PrettyMemory`.
- The header uses `#pragma once`.
- Standard library only — no third-party runtime dependencies.
- C++17 is the default; C++20 enables concepts and the `<=>` spaceship operator. Overridable via `-DCXX_STD=N`.
- MSVC gets `/utf-8` flag automatically.
- Build outputs go to `bin/`, intermediate files to `build/`.

## Documentation

Doxygen generates API docs from `include/PrettyMemory.h` comments:

```powershell
cmake -S . -B build\docs -DBUILD_DOCS=ON
cmake --build build\docs --target docs
```

Output: `doc/html`. Deployed to GitHub Pages on push to `main`.
