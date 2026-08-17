# PrettyMemory

Header-only C++17 smart pointer library with explicit ownership and safe observation.

[API documentation](doc/html/index.html) | [中文说明](README.zh-CN.md) | [Contributing](CONTRIBUTING.md)

## Types

All public types live in namespace `prtm`. Implementation details are in `prtm::detail`.

| Type | Description |
|------|-------------|
| `OwnerPtr<T, Deleter>` | Owning smart pointer. Move-only. Supports custom deleters. |
| `ShadowPtr<T>` | Non-owning observer. Detects when the target object is destroyed. |
| `EnableShadowFromThis<T>` | CRTP base. Lets an object create `ShadowPtr` from `this`. |

The entire implementation lives in `include/PrettyMemory.h`.

## Build and test

Requires: CMake 3.28+, C++17 compiler.

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run a single test by name pattern:

```powershell
ctest --test-dir build -R ShadowPtrTest --output-on-failure
```

List discovered tests without running:

```powershell
ctest --test-dir build -N
```

### Pipenv scripts (Windows)

Python helper scripts wrap CMake. Default build config is `RelWithDebInfo`:

```powershell
pipenv run build              # configure + build (RelWithDebInfo)
pipenv run test               # run tests
pipenv run build_release      # build Release
pipenv run test_release       # test Release
pipenv run clean              # remove build/, bin/, .vs/, .idea/
pipenv run doc                # generate Doxygen docs
```

CI builds in Release with `-G "Visual Studio 17 2022" -A x64`.

## Generate API documentation

Doxygen generates the API reference from comments in `include/PrettyMemory.h`.

```powershell
cmake -S . -B build\docs -DBUILD_DOCS=ON
cmake --build build\docs --target docs
```

Or use the helper script:

```powershell
pipenv run doc
```

Output goes to `doc/html`. The generated site does not expose absolute local file paths.

## Project layout

```
include/PrettyMemory.h    public API and header-only implementation
test/                     GoogleTest-based unit tests (one file per type)
CMakeLists.txt            top-level build: C++ standard, output dirs, BUILD_TESTS option
Pipfile                   Python helper scripts for build/test/clean/doc
```

## Example

```cpp
#include <PrettyMemory.h>

struct Object
{
    int Value{ 0 };
};

auto owner = prtm::OwnerPtr<Object>::Create();
owner->Value = 42;

auto shadow = owner.Shadow();
owner.Reset();

bool expired = shadow.Expired();
```

## CI and Pages

- `.github/workflows/ci.yml` — builds and tests on Windows for pushes to `main`/`master` and pull requests.
- `.github/workflows/deploy-docs.yml` — regenerates docs on push to `main`, deploys to GitHub Pages.

First-time Pages setup: set **Settings > Pages > Source** to **GitHub Actions**.
