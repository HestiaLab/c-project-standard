# C Project Standard

用一套可靠的工程基础开始你的下一个 C 库或 CLI 工具：CMake、C11、CTest、
跨平台 CI，以及可被用户和 AI 代理稳定调用的项目生成器。

本仓库既是 C 项目模板，也是模板自身的基础设施。生成项目只接收通用工程基建，
MSI、发布链、维护者文档和向导源码不会进入生成物。

## 目录结构

```text
include/        公共头文件
src/            实现；src/wizard/ 是模板专属向导
tests/          CTest 测试
cmake/          CMake 模块
scripts/        本地引导、检查与模板维护脚本
docs/guides/    生成项目共享指南
docs/maintainer/ 模板维护文档（不进入生成物）
.github/        CI 工作流和仓库模板
```

## 使用此模板

1. **构建向导：**

   ```sh
   cmake --preset ninja-debug
   cmake --build --preset ninja-debug
   ```

2. **交互式生成项目：**

   ```sh
   ./build/ninja-debug/bin/cproject
   ```

   Windows 使用 `./build/ninja-debug/bin/cproject.exe`。

3. **非交互生成：**

   ```sh
   cproject new my-lib --output /tmp/projects --yes --json
   ```

   名称支持 PascalCase、camelCase、kebab-case 和 snake_case，统一归一化为
   PascalCase。目标目录不存在或为空时允许创建，非空目录会失败。

4. **预览：**

   ```sh
   cproject new MyLib --dry-run --yes --json
   ```

5. **原地初始化（仅维护者）：**

   ```sh
   cproject init --in-place MyLib --yes
   ```

生成器默认开启 installable、subproject、CI、CLI，关闭 AI policy。
共享库、sanitizer 和覆盖率预设始终保留。完整契约见
[模板维护](docs/maintainer/template.md)。

## 快速开始（生成项目）

**所需工具：** CMake 3.21+、C11 编译器（MSVC、GCC、Clang 或 AppleClang），
以及默认预设所需的 Ninja。

Windows 上请在 Developer PowerShell、Developer Command Prompt 或初始化了
MSVC 的 VS Code CMake kit 中运行 Ninja 预设。Windows 环境配置见
[docs/guides/environment.md](docs/guides/environment.md)。

**首次设置：**

Windows PowerShell
```powershell
./scripts/bootstrap.ps1
```

Linux/macOS
```sh
./scripts/bootstrap.sh
```

**日常检查：**

Windows PowerShell
```powershell
./scripts/check.ps1
```

Linux/macOS
```sh
./scripts/check.sh
```

关于 configure、build、test、install、preset 和常见错误的详细说明见
[docs/guides/cmake.md](docs/guides/cmake.md)。

## 包含内容

- 生成器：严格非交互 CLI、JSON 输出、退出码 0/1/2、dry-run 和 `--git-init`。
- 元数据落地：description、author、version、license 写入项目和包元信息。
- C11 作为目标级语言基线，支持 MSVC、GCC、Clang 和 AppleClang。
- debug、release、shared-library、sanitizer 和 coverage 构建预设。
- 基于 CTest 的单元测试，使用 vendored Unity 框架（MIT）。
- 可选的 `find_package` 与 `add_subdirectory()` 冒烟测试。
- 可选的 GitHub Actions CI、Dependabot 和 PR/issue 模板。
- 可选的 AI 代理策略文档。
- 中文项目文档和模板维护文档分层。

## 包消费

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

`tests/package_smoke/` 验证已安装的 C 和 C++ 消费者。
`tests/subproject_smoke/` 验证 `add_subdirectory()` 消费路径。

## 文档

本仓库文档只维护一种语言：中文。

### 共享指南（进入生成项目）

| 文档 | 说明 |
|---|---|
| [docs/guides/environment.md](docs/guides/environment.md) | Windows 开发环境与工具安装 |
| [docs/guides/cmake.md](docs/guides/cmake.md) | CMake 预设、工作流与故障排除 |
| [docs/guides/coding-style.md](docs/guides/coding-style.md) | 命名、头文件、格式与 API 文档约定 |
| [docs/guides/testing.md](docs/guides/testing.md) | 测试分层、静态分析与通用质量门禁 |

### 架构决策

| 文档 | 说明 |
|---|---|
| [docs/adr/README.md](docs/adr/README.md) | 架构决策记录（ADR）说明 |
| [docs/adr/0001-c11-cmake-toolchain-policy.md](docs/adr/0001-c11-cmake-toolchain-policy.md) | C11 与 CMake 工具链策略 |

### 模板维护（不进入生成物）

| 文档 | 说明 |
|---|---|
| [docs/maintainer/README.md](docs/maintainer/README.md) | 维护者文档索引 |
| [docs/maintainer/template.md](docs/maintainer/template.md) | 向导契约、结构开关与载荷分层 |
| [docs/maintainer/testing.md](docs/maintainer/testing.md) | 模板专属门禁与生成物矩阵 |
| [docs/maintainer/release.md](docs/maintainer/release.md) | CPack/MSI 与发布工作流 |

### 根级文档

| 文档 | 说明 |
|---|---|
| [CONTRIBUTING.md](CONTRIBUTING.md) | 分支、提交与 PR 工作流 |
| [SECURITY.md](SECURITY.md) | 漏洞报告与依赖处理 |
| [AGENTS.md](AGENTS.md) | AI 代理入口 |
| [C_PROJECT_STANDARD.md](C_PROJECT_STANDARD.md) | 可复用的 C 项目工程标准 |

## 许可证

MIT
