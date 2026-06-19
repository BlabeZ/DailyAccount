/*
 * ============================================================================
 * 文件名:   flowpage.cpp
 * 所属模块: GUI - 流水记录列表页面（实现文件）
 * 功    能: FlowPage 类的完整实现。包含：
 *           1. 界面搭建（setupUI）—— 标题、筛选工具栏、流水树、分页栏
 *           2. 数据加载（loadPage）—— 从账本读取、筛选、分组、分页、渲染
 *           3. 交互逻辑（onAdd/onEdit/onDelete/翻页/筛选）
 * 编    码: UTF-8
 * 说    明: 本文件包含大量详细注释，逐行解释每个操作背后的"为什么"。
 *           建议按照函数从上到下的顺序阅读，先看 setupUI 了解界面结构，
 *           再看 loadPage 理解数据渲染流程，最后看各个槽函数了解交互逻辑。
 * ============================================================================
 */

// ---------- 本模块头文件 ----------
#include "flowpage.h"   // 自己的头文件（声明了类接口）

// ---------- 项目内部模块 ----------
#include "flowdialog.h" // 添加/编辑流水的对话框（本页面会弹出它）
#include "ledger.h"            // 账本类 —— 提供流水的增删改查接口
#include "category.h"          // 分类相关定义（RecordType 枚举、typeToChinese 等）

// ---------- Qt 框架头文件 ----------
#include <QVBoxLayout>    // 【垂直布局】将控件从上到下依次排列
#include <QHBoxLayout>    // 【水平布局】将控件从左到右依次排列
#include <QHeaderView>    // 【表头控件】QTreeWidget 的列标题行
#include <QMessageBox>    // 【消息对话框】弹出提示、警告、确认等窗口
#include <QFrame>         // 【框架容器】一个可以设置背景和边框的矩形区域
#include <QDate>          // 【日期类】提供日期计算和格式化功能


// ============================================================================
// 构  造  函  数
// ============================================================================
/*
 * 函  数: FlowPage::FlowPage
 * 参  数:
 *   ledger  - 账本对象引用（必须已初始化）
 *   catMan  - 分类管理器引用（必须已初始化）
 *   parent  - 父窗口指针，用于 Qt 的父子内存管理机制
 *
 * 执  行:
 *   1. 调用 QWidget 的构造函数，将 parent 传入，建立父子关系
 *   2. 用初始化列表将 m_ledger 和 m_catMan 绑定到传入的引用
 *      （C++ 引用必须在初始化列表中绑定，不能在函数体内赋值）
 *   3. 调用 setupUI() 创建整个页面的界面
 *
 * 注  意: 构造函数不加载数据。界面创建完成后是空白的，需要外部调用
 *         refresh() 才会从账本读取数据并显示。
 */
FlowPage::FlowPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QWidget(parent)           // 先调用父类 QWidget 的构造函数
    , m_ledger(ledger)          // 绑定账本引用
    , m_catMan(catMan)          // 绑定分类管理器引用
{
    // 创建整个页面的用户界面（工具栏、流水树、分页栏等）
    setupUI();

    // 构造函数到此结束。此时界面已经搭建好，但流水树是空的。
    // 外部（通常是主窗口）会在合适的时机调用 refresh() 来加载数据。
}


// ============================================================================
// 界  面  搭  建  函  数
// ============================================================================
/*
 * 函  数: FlowPage::setupUI
 * 功  能: 创建并排列本页面的所有可视化控件。
 *
 * 界面结构概览（从上到下）:
 *   ┌──────────────────────────────────────────┐
 *   │   账目明细           （标题标签）        │
 *   ├──────────────────────────────────────────┤
 *   │  筛选: [开始日期][结束日期][类型▼][筛选]  │ ← 筛选工具栏
 *   │                              [ 添加记录]│
 *   ├──────────────────────────────────────────┤
 *   │  时间 │ 类型 │ 金额 │ 分类 │ 备注 │ 操作  │ ← 流水树表头
 *   │  ─────────────────────────────────────── │
 *   │   2026-06-19 ...                       │ ← 日期父节点
 *   │    ├ 09:30 │ 支出 │ -12.00 │ ...        │ ← 流水子节点
 *   │    └ 14:00 │ 收入 │ +150.00│ ...        │
 *   │   2026-06-18 ...                       │
 *   │    └ ...                                 │
 *   ├──────────────────────────────────────────┤
 *   │           上一页   第 1/3 页   下一页   │ ← 分页栏
 *   └──────────────────────────────────────────┘
 *
 * 布局说明:
 *   使用 QVBoxLayout 作为主布局（垂直排列），其中嵌入：
 *     - 标题 QLabel
 *     - 工具栏 QFrame（内含 QHBoxLayout 水平排列的筛选控件和按钮）
 *     - 流水树 QTreeWidget（设置拉伸因子为 1，占据剩余空间）
 *     - 分页栏 QHBoxLayout
 */
void FlowPage::setupUI()
{
    // ---- 步骤1：创建主布局 ----
    // QVBoxLayout 将其中的子控件从上到下排列
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // setContentsMargins(左, 上, 右, 下)：设置布局四边留白
    // 24 像素的内边距让界面看起来更舒适，不会贴边显示
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // setSpacing(16)：设置子控件之间的垂直间距为 16 像素
    mainLayout->setSpacing(16);

    // ---- 步骤2：页面标题 ----
    // 创建一个标签显示" 账目明细"
    QLabel *title = new QLabel(" 账目明细");

    // setStyleSheet 使用类似 CSS 的语法设置控件外观
    // font-size: 22px → 字体大小
    // font-weight: bold → 加粗
    // color: #2C3E50 → 深蓝灰色文字
    // background: transparent → 背景透明（继承父控件的背景）
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #2C3E50; "
                         "background: transparent;");

    // 将标题添加到主布局的顶部
    mainLayout->addWidget(title);

    // ====================================================================
    // 步骤3：筛选工具栏
    // ====================================================================
    // 使用 QFrame 作为工具栏的容器，这样可以给它设置独立的背景和圆角
    QFrame *toolbar = new QFrame;

    // setProperty("class", "card") 与全局样式表配合，使工具栏具有"卡片"外观
    toolbar->setProperty("class", "card");

    // 工具栏内部使用水平布局，控件从左到右排列
    QHBoxLayout *toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setSpacing(12);  // 控件之间的水平间距

    // --- 3a. 筛选标签 ---
    QLabel *filterLabel = new QLabel("筛选:");
    filterLabel->setStyleSheet("font-weight: bold; background: transparent;");
    toolLayout->addWidget(filterLabel);

    // --- 3b. 起始日期选择框（默认本月1日）---
    QDate today = QDate::currentDate();
    m_filterStart = new QDateEdit(QDate(today.year(), today.month(), 1));
    m_filterStart->setCalendarPopup(true);
    m_filterStart->setDisplayFormat("yyyy-MM-dd");
    m_filterStart->setMinimumWidth(160);
    toolLayout->addWidget(new QLabel("从:"));
    toolLayout->addWidget(m_filterStart);

    // --- 3c. 结束日期选择框（默认今天）---
    m_filterEnd = new QDateEdit(today);
    m_filterEnd->setCalendarPopup(true);
    m_filterEnd->setDisplayFormat("yyyy-MM-dd");
    m_filterEnd->setMinimumWidth(160);
    toolLayout->addWidget(new QLabel("至:"));
    toolLayout->addWidget(m_filterEnd);

    // --- 3d. 类型筛选下拉框 ---
    m_filterType = new QComboBox;
    m_filterType->addItems({"全部", "支出", "收入"});
    toolLayout->addWidget(m_filterType);

    // --- 3e. "筛选"按钮 ---
    QPushButton *btnFilter = new QPushButton("筛选");
    btnFilter->setStyleSheet(
        "QPushButton { background: #3498DB; color: white; padding: 8px 16px; }"
        "QPushButton:hover { background: #2980B9; }");
    toolLayout->addWidget(btnFilter);
    connect(btnFilter, &QPushButton::clicked, this, &FlowPage::onFilterChanged);

    // --- 3f. 弹性空间 + 添加按钮 ---
    toolLayout->addStretch();

    m_btnAdd = new QPushButton(" 添加记录");
    m_btnAdd->setStyleSheet(
        "QPushButton { background: #27AE60; color: white; padding: 10px 20px; "
        "font-size: 14px; }"
        "QPushButton:hover { background: #219A52; }");
    toolLayout->addWidget(m_btnAdd);
    connect(m_btnAdd, &QPushButton::clicked, this, &FlowPage::onAdd);

    mainLayout->addWidget(toolbar);

    // ====================================================================
    // 步骤4：流水树（QTreeWidget）—— 页面的核心
    // ====================================================================
    m_tree = new QTreeWidget;
    m_tree->setHeaderLabels({"日期", "ID", "类型", "金额", "分类", "备注"});
    m_tree->setRootIsDecorated(true);
    m_tree->setIndentation(20);
    m_tree->setAnimated(true);
    m_tree->setAlternatingRowColors(false);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_tree->header()->setSectionResizeMode(2, QHeaderView::Interactive);
    m_tree->header()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_tree->header()->setSectionResizeMode(4, QHeaderView::Interactive);
    m_tree->header()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_tree->header()->resizeSection(0, 150);
    m_tree->header()->resizeSection(1, 50);
    m_tree->header()->resizeSection(2, 100);
    m_tree->header()->resizeSection(3, 200);
    m_tree->header()->resizeSection(4, 100);
    m_tree->setStyleSheet(
        "QTreeWidget::item { padding: 6px 4px; border-bottom: 1px solid #F0F3F7; }"
        "QTreeWidget::item:hover { background-color: #F5F7FA; }");

    // 双击流水记录行 → 弹出编辑对话框
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int) {
        if (item && item->parent()) {  // 只处理子节点（流水行），忽略日期头
            int id = item->data(1, Qt::UserRole).toInt();
            if (id > 0) onEdit(id);
        }
    });

    mainLayout->addWidget(m_tree, 1);

}


// ============================================================================
// 公  共  接  口
// ============================================================================
void FlowPage::refresh()
{
    loadPage();
}


// ============================================================================
// 筛  选  槽  函  数
// ============================================================================
void FlowPage::onFilterChanged()
{
    loadPage();
}


// ============================================================================
// 数  据  加  载  函  数  （本文件最核心的函数）
// ============================================================================
/*
 * 函  数: FlowPage::loadPage
 * 功  能: 从账本中读取指定日期范围内的流水，按类型过滤、日期分组、
 *         分页计算后渲染到 QTreeWidget。
 */
void FlowPage::loadPage()
{
    m_tree->clear();

    QString startDate = m_filterStart->date().toString("yyyy-MM-dd");
    QString endDate   = m_filterEnd->date().toString("yyyy-MM-dd");
    QString typeFilter = m_filterType->currentText();

    // 从账本获取指定日期范围内的所有流水
    std::vector<Record> filtered;
    auto rangeTxns = m_ledger.getRecordsByDateRange(
        startDate.toStdString(), endDate.toStdString());

    // 按类型过滤
    for (const auto& t : rangeTxns) {
        if (typeFilter == "支出" && t.type != RecordType::EXPENSE) continue;
        if (typeFilter == "收入" && t.type != RecordType::INCOME) continue;
        filtered.push_back(t);
    }

    // 按日期分组
    std::map<std::string, std::vector<Record>> grouped;
    for (const auto& t : filtered) {
        grouped[t.date].push_back(t);
    }

    // 获取所有日期并倒序排列
    std::vector<std::string> dates;
    for (const auto& [date, _] : grouped) {
        dates.push_back(date);
    }
    std::sort(dates.begin(), dates.end(), std::greater<std::string>());

    // 渲染全部日期组（滚动浏览，不再分页）
    for (int di = 0; di < (int)dates.size(); di++) {
        const std::string& date = dates[di];
        const auto& txns = grouped[date];

        // 计算当日汇总
        double dayIncome = 0;
        double dayExpense = 0;
        for (const auto& t : txns) {
            if (t.type == RecordType::INCOME)
                dayIncome += t.amount;
            else
                dayExpense += t.amount;
        }

        // 创建母行（日期头部）：仅显示日期和笔数，其余栏为空
        QTreeWidgetItem *dateItem = new QTreeWidgetItem;
        dateItem->setText(0, QString::fromStdString(date));
        dateItem->setText(1, QString("共%1笔").arg(txns.size()));
        dateItem->setBackground(0, QColor("#F5F7FA"));
        QFont dateFont;
        dateFont.setBold(true);
        dateFont.setPointSize(11);
        dateItem->setFont(0, dateFont);
        dateItem->setSizeHint(0, QSize(0, 36));
        m_tree->addTopLevelItem(dateItem);

        // 创建子行（流水明细）：日期栏留空，其余栏显示数据
        for (const auto& t : txns) {
            QTreeWidgetItem *txItem = new QTreeWidgetItem;
            txItem->setText(1, QString::number(t.id));
            txItem->setText(2, QString::fromStdString(typeToChinese(t.type)));
            txItem->setText(3, QString(t.type == RecordType::INCOME ? "+%1" : "-%1")
                                   .arg(t.amount, 0, 'f', 2));
            txItem->setText(4, QString::fromStdString(t.category));
            txItem->setText(5, QString::fromStdString(t.note));

            QColor txColor = (t.type == RecordType::INCOME)
                ? QColor("#27AE60") : QColor("#E74C3C");
            txItem->setForeground(3, txColor);
            txItem->setForeground(5, QColor("#95A5A6"));
            txItem->setData(1, Qt::UserRole, t.id);

            dateItem->addChild(txItem);
        }

        dateItem->setExpanded(true);
    }
}


// ============================================================================
// 添  加  流  水  槽  函  数
// ============================================================================
void FlowPage::onAdd()
{
    FlowDialog dlg(m_catMan, this);

    if (dlg.exec() == QDialog::Accepted) {
        m_ledger.addRecord(dlg.getRecord());
        refresh();
        if (auto *mw = window()) {
            QMetaObject::invokeMethod(mw, "refreshAll", Qt::QueuedConnection);
        }
    }
}


// ============================================================================
// 编  辑  流  水  槽  函  数
// ============================================================================
void FlowPage::onEdit(int id)
{
    Record* t = m_ledger.findRecord(id);
    if (!t) return;

    FlowDialog dlg(m_catMan, *t, this);

    if (dlg.exec() == QDialog::Accepted) {
        m_ledger.updateRecord(id, dlg.getRecord());
        refresh();
        if (auto *mw = window()) {
            QMetaObject::invokeMethod(mw, "refreshAll", Qt::QueuedConnection);
        }
    }
}


// ============================================================================
// 删  除  流  水  槽  函  数
// ============================================================================
void FlowPage::onDelete(int id)
{
    Record* t = m_ledger.findRecord(id);
    if (!t) return;

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("确认删除");
    msgBox.setText(QString("确定要删除以下记录吗？\n\n日期: %1\n金额: %2\n分类: %3")
                       .arg(QString::fromStdString(t->date))
                       .arg(t->amount, 0, 'f', 2)
                       .arg(QString::fromStdString(t->category)));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setStyleSheet(
        "QMessageBox { background: white; }"
        "QPushButton { min-width: 80px; }");

    if (msgBox.exec() == QMessageBox::Yes) {
        m_ledger.deleteRecord(id);
        refresh();
        if (auto *mw = window()) {
            QMetaObject::invokeMethod(mw, "refreshAll", Qt::QueuedConnection);
        }
    }
}


