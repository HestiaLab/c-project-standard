# AGENTS.md

面向在本模板及其派生项目中工作的 AI 代理的仓库专用说明。

## 必读文档

修改文件之前，先阅读：

- `CONTRIBUTING.md`
- `docs/guides/testing.md`

涉及架构的改动，还需阅读：

- `docs/adr/README.md`
- `docs/adr/` 下的相关 ADR 文件

涉及构建、CI、打包、发布、依赖或安全的工作，还需阅读：

- `docs/guides/cmake.md`
- `docs/maintainer/release.md`
- `SECURITY.md`

修改生成器、载荷或模板维护流程时，还需阅读：

- `docs/maintainer/template.md`
- `docs/maintainer/testing.md`

## 项目形状

这是一个使用 CMake 和 CTest 的 C11 项目模板。

```text
include/        公共头文件
src/            实现；src/wizard/ 仅属于模板
tests/          CTest 测试
cmake/          构建模块
scripts/        本地引导、检查与模板维护脚本
docs/guides/    生成项目共享指南
docs/maintainer/ 模板维护文档（不进入生成物）
```

## 构建与验证

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

构建产物 `build/ninja-debug/bin/cproject` 是**交互式项目初始化向导**。
运行 `cproject` 会从此模板创建一个新项目；用 `new MyProject --yes` 做非交互
创建，用 `new MyProject --dry-run --yes` 预览。只有当你确实想改动当前模板
目录本身时才使用 `init --in-place`。

Windows PowerShell：

```powershell
./scripts/check.ps1
```

Linux/macOS：

```sh
./scripts/check.sh
```

## 所有权规则

- 公共 API 属于 `include/`。
- 私有实现辅助属于 `src/`。
- 测试应覆盖行为，而不是偶然的实现细节。
- 保持 CMake 目标依赖最小化。
- 没有书面理由，不新增全局状态。
- 不为让改动通过而放宽警告、测试或 CI。

## AI 代理策略

当 AI 代理的改动与其他贡献一样被限定范围、评审和验证时，可以帮助开发。

### 允许的用途

- 聚焦的 bug 修复。
- 测试补充。
- 文档更新。
- 构建与 CI 维护。
- 保持行为的机械式重构。

### 受限的用途

AI 代理不应独立地：

- 在没有维护者指导的情况下重新设计核心架构。
- 削弱测试、警告、静态分析或 CI。
- 未经批准添加大型依赖或生成代码。
- 发布版本、轮换凭据或更改仓库权限。

### 披露

Pull request 应披露实质性的 AI 辅助：列出已读的必读文档、概述代理做了什么、
记录执行了哪些人工验证。

### 操作手册

**通用工作流：** 阅读 AGENTS.md 与相关文档，检查最小的相关文件集合，做聚焦的
改动，运行检查，写总结。

**Bug 修复：** 复现或定位失败行为，在可行时补回归测试，修复最小的归属模块，
运行针对性与更大范围的检查。

**重构：** 定义必须保持不变的行为，停留在模块边界内，避免把功能行为与重构
工作混在一起，运行覆盖所触及模块的测试。

**CI 或构建修复：** 检查失败的检查项与日志，在可行时本地复现，修复项目本身
的原因而不是掩盖检查，保持脚本与 preset 一致。

CMake、包、安装或子项目相关的工作，用 `docs/guides/cmake.md` 选择相关的
冒烟检查。

## 文档

当构建命令、CMake 用法、依赖、测试策略、发布策略、安全策略或公共 API 预期
发生变化时，更新本地文档。
