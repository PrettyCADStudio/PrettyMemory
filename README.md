# PrettyMemory

PrettyMemory is a header-only C++17 smart pointer library focused on explicit ownership and safe observation.

[Online API documentation](https://prettycadstudio.github.io/PrettyMemory/)
[中文说明 / Chinese README](README.zh-CN.md)
[Contribution guide](CONTRIBUTING.md)

## What it provides

- `prtm::OwnerPtr<T, Deleter>`: owning smart pointer with move-only ownership, custom deleter support, `Create`, `Transfer`, `Cast`, and `Shadow`.
- `prtm::ShadowPtr<T>`: non-owning observing pointer that becomes invalid after the target object is destroyed or released.
- `prtm::EnableShadowFromThis<T>`: base class that lets an object create `ShadowPtr` instances from `this`.

All public implementation lives in `include\PrettyMemory.h`.

## Project layout

- `include\PrettyMemory.h`: public API and header-only implementation
- `test\`: GoogleTest-based unit tests
- `doxygen-assets\`: Doxygen filter and assets for bilingual documentation
- `.github\workflows\ci.yml`: Windows CI build and test workflow
- `.github\workflows\deploy-docs.yml`: GitHub Pages documentation deployment workflow

## Requirements

- C++17 compiler
- CMake 3.15 or newer
- For local API docs generation: Doxygen and Python 3

The repository includes helper scripts for Visual Studio 2022 and Visual Studio 2026 on Windows, but standard CMake commands also work directly.

## Build and run tests

Recommended manual workflow from the repository root:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

To inspect discovered tests without running them:

```powershell
ctest --test-dir build -N
```

To run a single discovered test:

```powershell
ctest --test-dir build -R ShadowPtrTest
```

## Windows helper scripts

Generate a Visual Studio 2022 solution:

```powershell
.\make-solution-vs-2022.ps1
```

Generate a Visual Studio 2026 solution:

```powershell
.\make-solution-vs-2026.ps1
```

Open the preferred generated solution:

```powershell
.\start-with-visual-studio.ps1
```

Build the preferred generated solution in `Release` mode and run all tests:

```powershell
.\build-and-run-tests.ps1
```

`start-with-visual-studio.ps1` and `build-and-run-tests.ps1` prefer `build\vs2026` and fall back to `build\vs2022`.

## Generate API documentation

`include\PrettyMemory.h` contains bilingual Doxygen comments written with `@~english` and `@~chinese`. During generation, a Python filter converts them into switchable language blocks in the HTML output.

Generate the documentation locally with:

```powershell
cmake -S . -B build\docs -DBUILD_DOCS=ON
cmake --build build\docs --target docs
```

Or use the Windows helper script:

```powershell
.\generate-document.ps1
```

Generated HTML is written to:

- `doc\html`

The generated site:

- does not expose absolute local file system paths
- includes a Chinese / English language toggle

## GitHub Actions and Pages

- `.github\workflows\ci.yml` builds and tests the project on Windows for pushes to `main` and `master`, and for pull requests.
- `.github\workflows\deploy-docs.yml` regenerates documentation on pushes to `main` and deploys it to GitHub Pages.

If this repository is being configured for Pages for the first time, set **Settings > Pages > Source** to **GitHub Actions**.

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
