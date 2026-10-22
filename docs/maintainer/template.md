# 模板维护

本页说明模板生成器（`cproject` 向导）的契约，以及修改模板时必须同步维护的内容。
生成项目的日常 CMake 用法见 [CMake 指南](../guides/cmake.md)。

## 构建向导

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

Windows 产物：`build/ninja-debug/bin/cproject.exe`。Linux/macOS 产物
`build/ninja-debug/bin/cproject`。

## 生成项目

向导默认从模板创建项目，不会修改模板目录：

```sh
cproject new MyLib --yes
cproject new my-lib --output /tmp/projects --yes --json
cproject new MyLib --dry-run --yes --json
```

Windows PowerShell 可直接调用：

```powershell
./build/ninja-debug/bin/cproject.exe new MyLib --yes --json
```

`new` 的输出目录优先级：

1. `--output <parent>`；
2. `CPROJECT_DEFAULT_OUTPUT`；
3. 非交互时使用当前目录；
4. 交互时使用桌面（不存在则回退当前目录）。

目标目录不存在或为空时允许创建；非空目录直接失败。向导不提供 `--force`。

## 名称与元数据

名称接受 PascalCase、camelCase、kebab-case 和 snake_case，统一归一化为
PascalCase 标识符；目录名使用同一 PascalCase 名称。非 ASCII、数字开头、
非法字符和首尾/连续分隔符会被拒绝。

```sh
cproject new my-lib --yes \
  --description "Image codec" \
  --author "Example Org" \
  --project-version 1.2.3 \
  --license MIT
```

元数据落地位置：

| 元数据 | 写入位置 |
| --- | --- |
| description | `CMakeLists.txt`、生成 `README.md`、包摘要 |
| author | `LICENSE`、CMake 包变量、`README`/`AGENTS` |
| project version | `CMakeLists.txt`、`CHANGELOG.md`、包版本 |
| license | `LICENSE`、`README.md`、CMake 包变量 |

默认许可证为 MIT。非 MIT 许可证必须提供可读取的 `--license-file <path>`，
否则向导以退出码 2 失败。`--license` 只记录许可证名称，文本来自该文件。

`--version` 永远只打印向导版本；项目版本使用 `--project-version`。

## 结构开关

生成器只提供 5 个结构开关，每个都有 `--with-*` 与 `--no-*` 两种写法：

| 开关 | `--yes` 默认 | 关闭时移除 |
| --- | --- | --- |
| `installable` | 开 | CPack/find_package 安装规则与 package smoke |
| `subproject` | 开 | `tests/subproject_smoke/` 与相关文档 |
| `ci` | 开 | `.github/`（工作流、Dependabot、issue/PR 模板） |
| `ai-agent` | 关 | `AGENTS.md` 与 README 中的链接 |
| `cli` | 开 | 示例 CLI 桩、`src/main.c` 与资源文件 |

共享库、sanitizer 和覆盖率预设始终保留，不属于可选开关。

## 非交互契约

以下任一情况进入严格非交互模式，绝不读取 stdin：

- 传入 `--yes`；
- 传入 `--json`；
- 传入 `--dry-run`；
- stdin 不是 TTY。

非交互模式下缺少项目名称、缺少选项值、名称非法或版本非法都以退出码 2
失败。`--json` 隐含非交互，stdout 只输出一个 JSON 对象，诊断信息不混入
stdout。

退出码：

| 退出码 | 含义 |
| --- | --- |
| 0 | 成功 |
| 1 | 运行时失败（模板缺失、目标冲突、复制/写入失败、git 初始化失败） |
| 2 | 用法或参数校验失败 |

JSON 成功对象包含 `project`、`features`、`template`、`destination`、`dryRun`
和 `changes`。`--dry-run` 会在临时目录执行完整生成流程，但不触碰目标目录。

`--git-init` 在生成成功后于目标目录运行 `git init`；git 不可用时生成失败。

## 原地初始化

`init --in-place` 只用于维护者把当前模板树转换为项目，会修改当前目录：

```sh
cproject init --in-place MyLib --yes --json
```

非交互执行必须显式传 `--yes`；交互执行会显示警告并要求确认。它与 `new`
使用同一套生成引擎，不允许与 `--output`/`--template-root` 组合。

## 载荷分层

`template-exclude.txt` 是模板专属路径的单一清单，由以下两处共同消费：

- 向导的 `copy_tree()`：生成项目时跳过这些路径；
- `cmake/InstallRules.cmake`：安装暂存时排除这些路径。

当前模板专属内容：

- MSI/WiX 源、`cmake/PackagingMSI.cmake` 与
  `cmake/GenerateWiXTemplateTree.cmake`；
- 向导实现 `src/wizard/` 与 `cmake/Wizard.cmake`；
- 维护者文档 `docs/maintainer/`；
- release/package/publish 脚本；
- `res/run-wizard.bat`。

修改模板目录结构、安装规则或生成器时，必须同步检查该清单。生成项目中的
文件必须来自“通用工程基建”或向导显式生成的文档，不能依赖模板仓库里的
维护者文件。

## 生成物验证

每次模板改动至少运行：

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

然后在一个仓库外的临时目录生成项目和最小项目，分别 configure、build、
CTest。模板专属验证细节见 [模板测试](testing.md)。
