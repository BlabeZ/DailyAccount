# DailyAccount 日常记账

DailyAccount 是一个本地运行的 Qt Widgets 记账工具。它支持流水增删改查、父子分类、按日期筛选、分类与月度统计、文本导出及自定义分类管理。

后续产品范围、Android/Windows 双端方案、SQLite 迁移和本地优先同步设计见 [`docs/product-architecture.md`](docs/product-architecture.md)。该文档描述目标架构，当前已实现行为仍以本 README 和代码为准。

## 运行环境

- C++17 编译器
- Qt 6 Widgets
- Windows 发布构建使用 MinGW 和 qmake
- 不依赖数据库或第三方运行时库

## Windows 构建

默认工具路径为：

```bat
QT_DIR=D:\tools\Qt\6.9.3\mingw_64
MINGW_DIR=D:\tools\mingw64\bin
```

路径不同时，可在调用脚本前覆盖环境变量：

```bat
set QT_DIR=C:\Qt\6.9.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin
build\build.bat
```

脚本可以从任意工作目录启动，并依次执行：

1. 单独编译和运行后端测试。
2. 编译 Release GUI 程序。
3. 调用 `windeployqt --compiler-runtime` 收集 Qt 插件和运行库。

发布目录为 `build\dist`，入口程序为 `DailyAccount.exe`。

## 后端测试

Linux 或其他带有 g++ 的开发环境可直接运行：

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Ibackend tests/backend_tests.cpp backend/category.cpp backend/storage.cpp backend/ledger.cpp -o /tmp/dailyaccount_backend_tests
/tmp/dailyaccount_backend_tests
```

## 数据存储

程序使用 `QStandardPaths::AppDataLocation` 保存数据。当前组织名和应用名均为 `DailyAccount`，Windows 上通常位于 `%APPDATA%\DailyAccount\DailyAccount`，实际位置由 Qt 和系统配置决定。

- `ledger.dat`：当前完整账本，格式头为 `#DAILYACCOUNT_V3`。
- `ledger.dat.bak`：最近一次成功保存前的完整快照。
- `ledger.dat.corrupt`：用户确认恢复备份时保留的损坏快照。
- `dailyaccount.lock`：运行期间的单实例数据锁。

金额以 64 位整数“分”保存，不使用浮点数持久化；日期支持 `0100-01-01` 至 `9999-12-31`。每次修改会先写入并同步临时快照，验证校验和后再提交；记录、分类和下一个 ID 始终作为一个整体保存。

不要手工编辑 `ledger.dat`。文件截断、无效字段、重复 ID、金额溢出或尾标不完整都会使加载失败，程序会在存在有效备份时询问是否恢复。

## 旧数据迁移

旧版本的 `records.dat` 和 `categories.dat` 仍可读取。首次使用稳定数据目录时，程序只检查以下旧位置：

- 当前工作目录下的 `data`。
- 可执行文件目录下的 `data`。

只有目标目录没有任何数据且恰好找到一个物理来源时，程序才会验证旧数据并在新目录中生成单一 V3 快照。旧文件不会被删除或改写。若来源需要使用 `ledger.dat.bak`，程序会先明确询问；若备份和旧版文件都有效，可由用户选择迁移哪一份。若发现多个不同来源，程序会停止启动并显示路径，避免自动覆盖或合并账本。

## 清除与导出

“清除数据”会在一次事务中移除当前账本的全部记录和自定义分类，并把清除前状态保留为最近安全备份。后续成功修改会替换该备份；需要长期保留时，应先使用“数据导出”。

导出文件写入系统桌面目录，采用 UTF-8 文本并按日期分组。
