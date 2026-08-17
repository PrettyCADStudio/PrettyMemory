# PrettyMemory

仅头文件的 C++17 智能指针库，提供显式所有权管理与安全的对象观察机制。

[API 文档](doc/html/index.html) | [English README](README.md) | [贡献指南](CONTRIBUTING.md)

## 类型

所有公开类型位于 `prtm` 命名空间，内部实现位于 `prtm::detail`。

| 类型 | 说明 |
|------|------|
| `OwnerPtr<T, Deleter>` | 拥有型智能指针，仅支持移动语义，可自定义删除器 |
| `ShadowPtr<T>` | 非拥有型观察指针，目标对象销毁后自动失效 |
| `EnableShadowFromThis<T>` | CRTP 基类，使对象能从 `this` 创建 `ShadowPtr` |

全部实现位于 `include/PrettyMemory.h`。

## 编译与测试

需要：CMake 3.28+、C++17 编译器。

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

按名称模式运行单个测试：

```powershell
ctest --test-dir build -R ShadowPtrTest --output-on-failure
```

仅列出已发现的测试（不运行）：

```powershell
ctest --test-dir build -N
```

### Pipenv 脚本（Windows）

Python 辅助脚本封装了 CMake 命令。默认编译配置为 `RelWithDebInfo`：

```powershell
pipenv run build              # 配置 + 编译（RelWithDebInfo）
pipenv run test               # 运行测试
pipenv run build_release      # Release 编译
pipenv run test_release       # Release 测试
pipenv run clean              # 清理 build/、bin/、.vs/、.idea/
pipenv run doc                # 生成 Doxygen 文档
```

CI 使用 `-G "Visual Studio 17 2022" -A x64` 以 Release 配置构建。

## 生成 API 文档

Doxygen 从 `include/PrettyMemory.h` 中的注释生成 API 参考文档。

```powershell
cmake -S . -B build\docs -DBUILD_DOCS=ON
cmake --build build\docs --target docs
```

或使用辅助脚本：

```powershell
pipenv run doc
```

输出位于 `doc/html`。生成的站点不会暴露本地绝对路径。

## 项目结构

```
include/PrettyMemory.h    公开 API 与头文件实现
test/                     基于 GoogleTest 的单元测试（按类型分文件）
CMakeLists.txt            顶层构建：C++ 标准、输出目录、BUILD_TESTS 选项
Pipfile                   构建/测试/清理/文档的 Python 辅助脚本
```

## 示例

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

## CI 与 Pages

- `.github/workflows/ci.yml` — 推送到 `main`/`master` 或提交 PR 时在 Windows 上构建并测试。
- `.github/workflows/deploy-docs.yml` — 推送到 `main` 时重新生成文档并部署到 GitHub Pages。

首次启用 Pages：在 **Settings > Pages > Source** 中选择 **GitHub Actions**。
