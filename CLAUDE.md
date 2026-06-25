# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
# Build from project root
export PATH="D:/tools/Qt/6.9.3/mingw_64/bin:D:/tools/mingw64/bin:$PATH"
qmake jizhang.pro
mingw32-make -j$(nproc)
# Output: jizhang.exe — rename to 记账.exe for distribution
mv -f jizhang.exe "记账.exe"
```

- **Qt 6.9.3** (MinGW 64-bit) at `D:/tools/Qt/6.9.3/mingw_64`
- **MinGW toolchain** at `D:/tools/mingw64/bin`
- Target: `记账.exe` (static link, C++17, windows subsystem)
- **After every code change, rebuild the exe automatically.**

## Git Workflow

- **Remote:** `https://github.com/BlabeZ/DailyAccount.git`
- **Branch:** always work on `latest`, linear versioned commits (`v6: ...`, `v7: ...`, etc.)
- After every change: `git add -A` → `git commit -m "v<N>: <描述>"` → `git push origin latest`

## Architecture

```
记账/                        # Qt 6 C++ desktop accounting app (记账本)
├── backend/                  # Data layer (no Qt GUI dependency except QString in category)
│   ├── record.h              #   Core struct: Record {id, date, type, amount, category, note}
│   ├── storage.h/.cpp        #   File I/O: pipe-delimited records.dat + categories.dat in data/
│   ├── category.h/.cpp       #   CategoryManager: preset categories + custom categories
│   └── ledger.h/.cpp         #   Ledger: CRUD, queries, statistics (getCategorySummaries, etc.)
├── gui/                      # GUI layer (Qt Widgets)
│   ├── main_gui.cpp          #   Entry point: QApplication + global QSS stylesheet + wiring
│   ├── mainwindow.h/.cpp     #   MainWindow: sidebar navigation + QStackedWidget (6 pages)
│   ├── homepage.h/.cpp       #   Index 0: Startup splash with animated "记账本" text
│   ├── dashboardpage.h/.cpp  #   Index 1: Summary cards + recent 10 records + expense bar chart
│   ├── flowpage.h/.cpp       #   Index 2: Transaction list (QTreeWidget grouped by date) + filters
│   ├── flowpage.ui           #   FlowPage toolbar layout (date/type/category filters, tree)
│   ├── flowdialog.h/.cpp     #   Add/Edit dialog with category→subcategory cascade
│   ├── statisticspage.h/.cpp #   Index 3: PieChart + BarChart (QPainter drawn), monthly trends
│   ├── categorypage.h/.cpp   #   Index 4: Category management (add/remove custom categories)
│   └── otherpage.h/.cpp      #   Index 5: Export to TXT, clear all data
└── jizhang.pro               # Qt project file
```

### Data Flow
1. `StorageManager` reads/writes `data/records.dat` (pipe-delimited) and `data/categories.dat`
2. `Ledger` wraps StorageManager + CategoryManager, provides CRUD and aggregation APIs
3. `MainWindow` owns Ledger& and CategoryManager&, passes them to all pages by reference
4. Every page has a `refresh()` method; `MainWindow::refreshAll()` calls all of them + status bar

### Category Format
- Records store categories as plain strings. Two categories have subcategories using `主分类(子分类)` format: `"饮食(午饭)"`, `"交通(公交)"`
- `Ledger::getCategorySummaries()` strips parenthetical subcategories so aggregation is by top-level only
- `FlowDialog` composes `"主分类(子分类)"` on save, parses on edit
- FlowPage's category filter uses cascading QMenu: main category → hover → subcategory submenu

### Key UI Patterns
- **Global stylesheet** in `main_gui.cpp` — all QComboBox, QMenu, QPushButton, QTableWidget, etc. styled here
- **Sidebar navigation** uses `QPushButton[class="nav"]` with dynamic `[active="true"]` property + `unpolish()/polish()` refresh
- **Summary cards** use QLabel + HTML RichText for card-like appearance (colored left border)
- **Pie/Bar charts** are QPainter-drawn custom widgets (no Qt Charts dependency)
- **Expense bar chart** in DashboardPage: each row is a QWidget container added via `addWidget()` (not `addLayout()`) so cleanup code works correctly on refresh
