# 更新日志

> 新增段落用中文书写；已发布段落保留发布时的原文，不重写。

## 0.2.0 — 2026-10-22

### 破坏性变更

- `--version` 现在只打印向导版本；项目版本改用 `--project-version x.y.z`。
- `new` 与 `init --in-place` 统一使用同一套生成引擎；交互式与命令行结果
  保持一致。
- 可选特性收敛为 5 个结构开关：`installable`、`subproject`、`ci`、
  `ai-agent`、`cli`；共享库、sanitizer 与覆盖率预设始终保留。
- 移除 `scripts/rename-project.sh` 与 `scripts/rename-project.ps1`；向导是
  唯一的项目生成与重命名实现。
- 无交互模式（`--yes`、`--json`、`--dry-run` 或非 TTY）不再读取 stdin；
  缺少项目名称或参数值返回退出码 2。
- 非空输出目录不再被覆盖；不提供 `--force`。

### 新增

- 项目元数据全量落地：CMake、README、LICENSE、CHANGELOG 与包元信息。
- 名称归一化：PascalCase、camelCase、kebab-case、snake_case 均归一为
  PascalCase 标识符。
- `--json` 输出结构化 dry-run/结果/错误；退出码统一为 0/1/2。
- `--git-init` 可选初始化 git 仓库。
- `template-exclude.txt` 统一驱动生成物复制与安装暂存，模板专属基建不再
  泄漏到生成项目。
- 生成项目继承平衡 CI 矩阵与通用质量门禁。

### 修复

- 修复递归删除非空目录无效、特性裁剪未真正生效的问题。
- 修复显式 `--lang` 被环境变量覆盖的问题。
- 修复生成项目 README 指向已裁剪 `AGENTS.md` 的死链。
- 修复安装包携带 MSI 配置却缺少 installer 源文件的坏模板。

## 0.1.0 — 2026-06-05

- Initial template release.
- CMake 3.21+, C11, Ninja presets.
- Cross-platform support: MSVC, GCC, Clang, AppleClang.
- Build presets: debug, release, shared, asan, coverage.
- CTest and static analysis CI.
- `find_package` and `add_subdirectory()` smoke tests.
- Bootstrap, check, rename, and release scripts.
- Example library and executable.
- AI agent policy and contributor docs.
