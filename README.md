# C Project Standard

**A C11 project template plus `cproject`, a scriptable project generator with
CMake, CTest, packaging and cross-platform CI baked in.** · 一套 C11 项目模板，
外加一个可被脚本和 AI 代理稳定调用的项目生成器。

**定位**：需要用统一工程基线（CMake 预设、CTest、跨平台 CI、安装与打包、中文文档
分层）立刻开始 C 库或 CLI 工具的 C 开发者；以及需要非交互、可重复地批量生成项目的
维护者与 AI 代理。
**状态**：已发布 v0.2.1（依据：
[v0.2.1 Release](https://github.com/HestiaLab/c-project-standard/releases/tag/v0.2.1)
提供 Windows MSI、macOS pkg、Linux deb/rpm、源码归档与 SHA256SUMS 共 9 个制品；
`CMakeLists.txt` 的 `project(... VERSION 0.2.1)` 与 `CHANGELOG.md` 一致）。仓库已设为
GitHub Template，`Use this template` 可用。

[![CI](https://github.com/HestiaLab/c-project-standard/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/HestiaLab/c-project-standard/actions/workflows/ci.yml)
[![Static Analysis](https://github.com/HestiaLab/c-project-standard/actions/workflows/static-analysis.yml/badge.svg?branch=main)](https://github.com/HestiaLab/c-project-standard/actions/workflows/static-analysis.yml)
[![Release](https://img.shields.io/github/v/release/HestiaLab/c-project-standard)](https://github.com/HestiaLab/c-project-standard/releases)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)

[开始使用](#开始使用) · [生成物示例](#生成物示例) · [Releases](https://github.com/HestiaLab/c-project-standard/releases) · [向导契约](docs/maintainer/template.md) · [生成项目指南](docs/guides/cmake.md)

本仓库既是 C 项目模板，也是模板自身的基础设施。生成项目只接收通用工程基建，
MSI、发布链、维护者文档和向导源码不会进入生成物（单一清单见
[template-exclude.txt](template-exclude.txt)）。

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

## 开始使用

两条入口，取决于你要的是模板本身，还是只含通用工程基线的干净生成物。

| 路径 | 得到什么 | 何时选 |
| --- | --- | --- |
| **A · Use this template** | 模板仓库的一份副本，**含**向导源码、`installer/`、`docs/maintainer/` 与发布链 | 要改模板自身，或想在副本里原地转成项目 |
| **B · 本地生成** | `cproject new` 产出的项目，**不含**任何模板专属内容 | 要一个干净起点，或需要脚本化、批量生成 |

### 路径 A：Use this template

打开 [Use this template](https://github.com/HestiaLab/c-project-standard/generate)
创建新仓库（本仓库已设为 GitHub Template）。副本带有模板自身的基础设施；要把这个
目录原地转换成项目，使用下面第 5 步的 `init --in-place`，它会修改当前目录。

### 路径 B：本地生成

1. **构建向导**（需要 CMake 3.21+、C11 编译器与 Ninja，见
   [环境配置](docs/guides/environment.md)）：

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

不想自己构建向导时，可以从
[Releases](https://github.com/HestiaLab/c-project-standard/releases) 安装对应平台的
发行版（Windows `.msi`、macOS `.pkg`、Linux `.deb`/`.rpm`），安装后 `cproject` 进入
PATH，其余步骤与路径 B 相同。

生成器默认开启 installable、subproject、CI、CLI，关闭 AI policy。
共享库、sanitizer 和覆盖率预设始终保留。完整契约见
[模板维护](docs/maintainer/template.md)。

## 生成物示例

以下记录是 2026-10-29 的一次真实执行：构建向导 → 跑模板自身测试 → 生成
`WidgetLab` → 再 configure、build、CTest 这个生成物。环境为 Windows 11 +
CMake 4.3.3 + Ninja 1.13.2 + GCC 16.2（MSYS2 UCRT64）。

```text
$ cmake --preset ninja-debug
-- Windows MSI installer target 'msi' configured (WiX: ...)
-- Configuring done (1.6s)
-- Generating done (0.1s)
-- Build files have been written to: C:/cps-demo/c-project-standard/build/ninja-debug

$ cmake --build --preset ninja-debug
[19/19] Linking C executable bin\cproject.exe

$ ctest --preset ninja-debug
100% tests passed, 0 tests failed out of 14

$ ./build/ninja-debug/bin/cproject.exe new WidgetLab --output projects --yes
Project
  Name         WidgetLab
  Namespace    widgetlab
  Version      0.1.0
  License      MIT
  Template     C:\cps-demo\c-project-standard
  Destination  projects\WidgetLab
  Features: installable=on subproject=on ci=on ai-agent=off cli=on

  Project created.
  Renamed files: 34
  Removed: AGENTS.md
  Wrote: LICENSE
  Wrote: CHANGELOG.md
  Wrote: README.md
  Wrote: src/main.c
  Wrote: .github/workflows/ci.yml

$ cd projects/WidgetLab
$ cmake --preset ninja-debug && cmake --build --preset ninja-debug && ctest --preset ninja-debug
[11/11] Linking C executable bin\widgetlab.exe
100% tests passed, 0 tests failed out of 7
```

生成物的目录结构（75 个文件，此处按目录折叠）：

```text
WidgetLab/
├── include/widgetlab/     公共头文件（已按项目名重命名）
├── src/widgetlab/         实现
├── src/main.c             CLI 入口（由向导生成）
├── tests/                 CTest 用例与 vendored Unity
│   ├── package_smoke/     find_package 消费冒烟
│   └── subproject_smoke/  add_subdirectory 消费冒烟
├── cmake/                 构建、安装与打包模块（含 WidgetLabConfig.cmake.in）
├── scripts/               bootstrap 与检查脚本
├── docs/guides/           共享指南（环境、CMake、编码风格、测试）
├── docs/adr/              架构决策记录
├── .github/               CI、Dependabot 与 issue/PR 模板
├── res/                   Windows 版本资源与图标
├── CMakeLists.txt · CMakePresets.json · CMakeUserPresets.example.json
├── README.md · LICENSE · CHANGELOG.md · CONTRIBUTING.md · SECURITY.md
└── C_PROJECT_STANDARD.md · .clang-format · .clang-tidy · .editorconfig
```

`ai-agent` 开关默认关闭，因此生成物里没有 `AGENTS.md`（对应上面 `Removed: AGENTS.md`
一行）；加 `--with-ai-agent` 才会生成。

## 生成物边界

单一清单是 [template-exclude.txt](template-exclude.txt)，向导的 `copy_tree()` 与
`cmake/InstallRules.cmake` 共同消费它，因此生成项目与安装暂存不会各说各话。

不进入生成物：

- `installer/`、`cmake/PackagingMSI.cmake`、`cmake/GenerateWiXTemplateTree.cmake`：
  MSI/WiX 打包源；
- `src/wizard/`、`cmake/Wizard.cmake`、`tests/wizard/`：向导自身；
- `docs/maintainer/`：模板维护文档；
- `.github/workflows/release.yml`，以及 `scripts/` 下的 package、publish、msi-smoke、
  gate-floor-probe、changelog 与 doc-path 检查脚本；
- `res/run-wizard.bat`。

由向导按项目元数据重新生成（模板里的版本先被排除）：`LICENSE`、`CHANGELOG.md`、
`README.md`、`src/main.c`、`.github/workflows/ci.yml`。

关闭结构开关会再移除对应载荷：`installable` 去掉安装规则与 package smoke，
`subproject` 去掉 `tests/subproject_smoke/`，`ci` 去掉 `.github/`，`cli` 去掉示例
CLI 桩与 `src/main.c`。共享库、sanitizer 与覆盖率预设始终保留。

## 快速开始（生成项目）

以下命令在**生成出来的项目目录**（或路径 A 的模板副本）中执行，不是在本模板仓库的
源码目录里执行。

**所需工具：** CMake 3.21+、C11 编译器（MSVC、GCC、Clang 或 AppleClang），
以及默认预设所需的 Ninja。

Windows 上请在 Developer PowerShell、Developer Command Prompt 或初始化了
MSVC 的 VS Code CMake kit 中运行 Ninja 预设。Windows 环境配置见
[docs/guides/environment.md](docs/guides/environment.md)；该页推荐的
MSYS2 UCRT64 GCC + 原生 CMake + Ninja 组合是本仓库 README 全部命令与
[生成物示例](#生成物示例) 的实际验证环境。

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

[MIT](LICENSE)。
