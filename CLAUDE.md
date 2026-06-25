# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在此仓库中工作时提供指导。

## 构建与运行

```bash
# 在项目根目录执行构建
export PATH="D:/tools/Qt/6.9.3/mingw_64/bin:D:/tools/mingw64/bin:$PATH"
qmake jizhang.pro
mingw32-make -j$(nproc)
# 输出文件: jizhang.exe —— 重命名为 记账.exe 发布
mv -f jizhang.exe "记账.exe"
```

- **Qt 6.9.3** (MinGW 64-bit)，路径：`D:/tools/Qt/6.9.3/mingw_64`
- **MinGW 工具链**，路径：`D:/tools/mingw64/bin`
- 目标文件：`记账.exe`（静态链接，C++17，Windows 子系统）
- **每次修改代码后必须自动重新编译生成 exe。**

## Git 工作流

- **远程仓库：** `https://github.com/BlabeZ/DailyAccount.git`
- **分支策略：** 始终在 `latest` 分支上工作，线性递增版本号提交（`v6: ...`、`v7: ...` 以此类推）
- 每次修改后执行：`git add -A` → `git commit -m "v<N>: <描述>"` → `git push origin latest`

## 项目架构

```
记账/                        # Qt 6 C++ 桌面记账应用
├── backend/                  # 数据层（仅 category 模块依赖 QString，其余无 Qt GUI 依赖）
│   ├── record.h              #   核心结构体：Record {id, date, type, amount, category, note}
│   ├── storage.h/.cpp        #   文件读写：管道符分隔的 records.dat 和 categories.dat，存放于 data/
│   ├── category.h/.cpp       #   CategoryManager：预设分类 + 自定义分类管理
│   └── ledger.h/.cpp         #   Ledger：增删改查、查询、统计（getCategorySummaries 等）
├── gui/                      # 界面层（Qt Widgets）
│   ├── main_gui.cpp          #   入口：QApplication + 全局 QSS 样式表 + 组件装配
│   ├── mainwindow.h/.cpp     #   MainWindow：左侧导航栏 + QStackedWidget（6个页面）
│   ├── homepage.h/.cpp       #   页面0：启动首页，淡入动画显示"记账本"
│   ├── dashboardpage.h/.cpp  #   页面1：概览 —— 汇总卡片 + 最近10条记录 + 支出分布柱状图
│   ├── flowpage.h/.cpp       #   页面2：账目明细 —— 按日期分组的流水树 + 筛选工具栏
│   ├── flowpage.ui           #   FlowPage 工具栏布局（日期/类型/分类筛选、流水树）
│   ├── flowdialog.h/.cpp     #   添加/编辑弹窗，含分类→子分类联动
│   ├── statisticspage.h/.cpp #   页面3：统计 —— 自绘饼图 + 柱状图 + 月度趋势
│   ├── categorypage.h/.cpp   #   页面4：分类管理（增删自定义分类）
│   └── otherpage.h/.cpp      #   页面5：其他功能（导出TXT、清除数据）
└── jizhang.pro               # Qt 项目文件
```

### 数据流
1. `StorageManager` 读写 `data/records.dat`（管道符分隔）和 `data/categories.dat`
2. `Ledger` 封装 StorageManager 和 CategoryManager，提供 CRUD 和聚合统计接口
3. `MainWindow` 持有 Ledger& 和 CategoryManager&，以引用方式传递给所有页面
4. 每个页面都有 `refresh()` 方法；`MainWindow::refreshAll()` 依次调用所有页面刷新并更新状态栏

### 分类格式
- 记录中以纯字符串存储分类。两个分类含有子分类，格式为 `主分类(子分类)`：如 `"饮食(午饭)"`、`"交通(公交)"`
- `Ledger::getCategorySummaries()` 聚合时剥离括号内子分类，确保按一级分类汇总
- `FlowDialog` 保存时组合为 `"主分类(子分类)"`，编辑时解析拆分
- FlowPage 的分类筛选使用级联 QMenu：一级分类 → 悬停 → 子分类子菜单

### 关键 UI 模式
- **全局样式表** 在 `main_gui.cpp` 中定义 —— 所有 QComboBox、QMenu、QPushButton、QTableWidget 等统一在此设置样式
- **侧边栏导航** 使用 `QPushButton[class="nav"]`，通过动态 `[active="true"]` 属性 + `unpolish()/polish()` 刷新高亮状态
- **汇总卡片** 使用 QLabel + HTML RichText 实现卡片外观（左侧彩色边框）
- **饼图/柱状图** 均为 QPainter 手绘的自定义控件（无 Qt Charts 依赖）
- **支出分布柱状图**（DashboardPage）：每行使用 QWidget 容器通过 `addWidget()` 添加（而非 `addLayout()`），确保刷新时清理代码能正确销毁旧控件
