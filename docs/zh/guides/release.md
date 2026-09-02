<!-- 对应英文版：docs/en/guides/release.md | 最后同步：2026-09-02 -->

# 发布

本模板提供多平台打包，以及一个为 Linux、macOS 和 Windows 生成安装包的 GitHub Actions 工作流。

## 打包

各平台的安装包由 CPack（Linux/macOS）和 WiX（Windows）构建：

| 平台 | 生成器 | 制品 |
| -------- | ----------------------------- | -------------------------- |
| Linux    | DEB, RPM, TGZ                 | `.deb`, `.rpm`, `.tar.gz`  |
| macOS    | productbuild, TGZ             | `.pkg`, `.tar.gz`          |
| Windows  | WiX (MSI), ZIP (CPack)        | `.msi`, `.zip`             |

在本地构建安装包：

```sh
cmake --preset ninja-release
cmake --build --preset ninja-release

# Linux / macOS
cd build/ninja-release && cpack

# Windows
cmake --build --preset ninja-release --target msi
```

安装包输出到 `build/ninja-release/`（Linux/macOS）或 `build/ninja-release/msi/`（Windows）。

### 便携 zip（package.ps1）

想快速本地打一个完整安装树的 zip——不依赖 WiX/CPack——用打包脚本：

```powershell
cmake --preset ninja-release
cmake --build --preset ninja-release
./scripts/package.ps1
```

它把构建安装到暂存目录，对 `cproject` 向导做冒烟检查，然后打包 `bin/`、`include/`、`lib/`、`lib/cmake/` 以及 `share/` 下的模板源码（即下方 *包内容* 的布局），并写出 `SHA256SUMS.txt`。产物是 `dist/cproject-standard-win64-<version>.zip`。

### Windows MSI 前置条件

Windows MSI 需要 WiX Toolset v6 和两个扩展。一次性装好：

```sh
dotnet tool install --global wix --version 6.*
wix extension add WixToolset.UI.wixext
wix extension add WixToolset.Util.wixext
```

### 源码包

```sh
cmake --preset ninja-debug
cd build/ninja-debug && cpack --config CPackSourceConfig.cmake
```

## 发布工作流

`.github/workflows/release.yml` 工作流的触发方式：

1. **自动**：推送匹配 `v*` 的 tag（例如 `v0.2.0`）。
2. **手动**：在 Actions 标签页用 `workflow_dispatch` 触发。

该工作流会：

1. 在 `ubuntu-latest`、`macos-latest`、`windows-latest` 上构建 `ninja-release`。
2. 运行 CPack（Linux/macOS）或 CMake 的 `msi` 目标（Windows）生成各平台安装包。
3. 构建源码归档（`tar.gz` + `zip`）。
4. 推送 tag 时，创建附带全部制品的 GitHub Release。

### 本地 zip 发布（publish.ps1）

从本机用已认证的 `gh` CLI 把 `package.ps1` 打好的 zip 上传到 GitHub Release：

```powershell
./scripts/publish.ps1 -Tag v0.1.0
```

若 release 不存在则创建（附上 zip 和 `SHA256SUMS.txt`），否则上传到已有 release。用 `-Draft` / `-Prerelease` 表示草稿或预发布，用 `-NotesFile <file>` 提供自定义发布说明。发布是对远端仓库的有状态操作，须在本地验证并经过维护者审查后再执行。

## 发布检查清单

1. 更新 `CMakeLists.txt` 里的版本号（`project(... VERSION x.y.z)`）。
2. 更新 `CHANGELOG.md`。
3. 运行本地验证：

   **Windows PowerShell**

   ```powershell
   ./scripts/check.ps1 -Preset ninja-release
   cmake --preset ninja-shared
   cmake --build --preset ninja-shared
   ctest --preset ninja-shared --output-on-failure
   ```

   **Linux/macOS**

   ```sh
   ./scripts/check.sh --preset ninja-release
   cmake --preset ninja-shared
   cmake --build --preset ninja-shared
   ctest --preset ninja-shared --output-on-failure
   ```

4. 运行静态库和共享库的包冒烟测试（见 `cmake.md`）。
5. 提交并推送。
6. 打发布 tag：

   ```sh
   git tag -a v0.2.0 -m "v0.2.0"
   git push origin v0.2.0
   ```

7. 在 GitHub Actions 里观察 Release 工作流的运行。
8. 检查生成的 release 和安装包。

## 包内容

每个二进制安装包会安装以下内容：

| 路径 | 内容 |
| --- | --- |
| `bin/` | `cproject` 项目向导 |
| `include/` | 公共 C 头文件 |
| `lib/` | `libcproject_core` 库 |
| `lib/cmake/CProjectStandard/` | CMake `find_package` 配置 |
| `share/c-project-standard/` | 完整项目模板源码 |

### 安装之后

安装后的 `cproject` 命令以只读方式使用 `share/c-project-standard/` 作为模板来源，并在用户可写的输出目录中创建项目。安装完成后，打开一个新终端并运行：

```sh
cproject
```

或者以非交互方式创建项目：

```sh
cproject new MyProject --yes
```

在 Windows 上，安装器还会为 Project Setup Wizard 创建开始菜单和桌面快捷方式。

### Windows 安装包人工检查

发布 Windows 版本之前，安装生成的 `.msi` 并逐项确认：

1. 安装向导页面显示正常（许可协议、安装目录、进度）。
2. **当前用户的个人桌面**上有名为 `CProjectStandard` 的桌面快捷方式。
3. 开始菜单文件夹包含 `Project Setup Wizard`、`Template Files` 和 `Uninstall`。
4. 新终端能运行 `cproject --version` 和 `cproject new MyProject --yes`。
5. 生成的项目能正常 configure、build 并通过测试。
6. 通过 *应用和功能*（Apps & Features）或 `msiexec /x` 卸载后，桌面快捷方式和开始菜单条目应被移除。

未经维护者批准，不要发布来自 AI 辅助改动的版本。
