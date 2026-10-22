# 模板发布

本页只描述模板仓库自身的发布链。生成项目使用自己的 CPack 配置，不继承
MSI、维护者脚本或本页的发布工作流。

## 制品

| 平台 | 生成器 | 制品 |
| --- | --- | --- |
| Linux | CPack DEB/RPM/TGZ | `.deb`、`.rpm`、`.tar.gz` |
| macOS | CPack TGZ；发布工作流 pkgbuild/productbuild | `.pkg`、`.tar.gz` |
| Windows | WiX 6 MSI | `.msi` |

## 本地构建

```sh
cmake --preset ninja-release
cmake --build --preset ninja-release

# Linux/macOS
cd build/ninja-release && cpack

# Windows
cmake --build --preset ninja-release --target msi
```

Windows MSI 还需要 WiX 6 与 UI/Util 扩展：

```sh
dotnet tool install --global wix --version 6.*
wix extension add WixToolset.UI.wixext
wix extension add WixToolset.Util.wixext
```

`scripts/package.ps1` 可生成本地便携 zip；它只属于模板仓库。
`scripts/publish.ps1` 通过已认证的 `gh` 上传 Release；发布是对远端仓库的
有状态操作，只能在维护者完成本地验证并明确批准后执行。
`scripts/msi-smoke.ps1` 对已构建的 MSI 做行政安装，运行安装后的向导生成项目，
并执行 configure/build/CTest。

## GitHub Actions

`.github/workflows/release.yml` 支持：

1. 推送 `v*` tag；
2. 手动 `workflow_dispatch`，通过 `tag` 输入指定要发布的 tag。

工作流会构建三平台安装包与源码归档，并在 tag 上下文创建 GitHub Release。
手动触发时必须填写一个已存在的 `v*` tag；仅构建、不发布时应使用普通 CI。

发布前必须确认：

1. `CMakeLists.txt` 的版本与 `CHANGELOG.md` 一致；
2. CHANGELOG 的发布段落已冻结；
3. 默认与最小生成物均通过 CTest；
4. 安装暂存无模板专属文件泄漏；
5. Windows MSI 安装后，向导能生成、configure、build、CTest 一个项目；
6. 没有 AI 代理未经维护者批准发布版本。

## 发布检查清单

1. 更新模板 `project(... VERSION x.y.z)` 与 `CHANGELOG.md`。
2. 运行模板完整检查：

   ```sh
   ./scripts/check.sh --enable-tidy        # Linux/macOS
   ./scripts/check.ps1 -EnableTidy         # Windows
   ```

3. 运行 default/minimal 生成物矩阵与安装泄漏检查，见
   [模板测试](testing.md)。
4. 构建 Windows MSI 并执行人工安装检查。
5. 提交改动，打 tag 并推送。
6. 观察 Release 工作流，核验全部制品与 checksum。

## 安装后人工检查

1. 安装向导页面正常（许可证、安装目录、进度）。
2. 开始菜单和桌面快捷方式指向 `run-wizard.bat`。
3. 新终端能运行 `cproject --version`。
4. `cproject new MyProject --yes` 能生成项目。
5. 生成项目能 configure、build、CTest。
6. 卸载后快捷方式与开始菜单项被移除。
