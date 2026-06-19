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
#include "ui_flowpage.h"

// ---------- 项目内部模块 ----------
#include "flowdialog.h" // 添加/编辑流水的对话框（本页面会弹出它）
#include "ledger.h"            // 账本类 —— 提供流水的增删改查接口
#include "category.h"          // 分类相关定义（RecordType 枚举、typeToChinese 等）

// ---------- Qt 框架头文件 ----------
#include <QHeaderView>    // 【表头控件】QTreeWidget 的列标题行
#include <QMessageBox>    // 【消息对话框】弹出提示、警告、确认等窗口
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
    , ui(new Ui::FlowPage)      // 初始化 UI
{
    // 加载由 Qt Designer / uic 生成的界面
    ui->setupUi(this);

    // 设置默认筛选日期：起始日期为本月1日，结束日期为今天
    QDate today = QDate::currentDate();
    ui->m_filterStart->setDate(QDate(today.year(), today.month(), 1));
    ui->m_filterEnd->setDate(today);

    // 配置流水树的表头：列宽和缩放模式
    ui->m_tree->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->m_tree->header()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->m_tree->header()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->m_tree->header()->setSectionResizeMode(3, QHeaderView::Interactive);
    ui->m_tree->header()->setSectionResizeMode(4, QHeaderView::Interactive);
    ui->m_tree->header()->setSectionResizeMode(5, QHeaderView::Stretch);
    ui->m_tree->header()->resizeSection(0, 150);
    ui->m_tree->header()->resizeSection(1, 50);
    ui->m_tree->header()->resizeSection(2, 100);
    ui->m_tree->header()->resizeSection(3, 200);
    ui->m_tree->header()->resizeSection(4, 100);

    // 双击流水行 → 弹出编辑对话框
    connect(ui->m_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int) {
        if (item && item->parent()) {  // 只处理子节点（流水行），忽略日期头
            int id = item->data(1, Qt::UserRole).toInt();
            if (id > 0) onEdit(id);
        }
    });

    // 筛选按钮 → 触发筛选
    connect(ui->btnFilter, &QPushButton::clicked, this, &FlowPage::onFilterChanged);

    // 添加记录按钮 → 弹出添加对话框
    connect(ui->m_btnAdd, &QPushButton::clicked, this, &FlowPage::onAdd);
}


// ============================================================================
// 析  构  函  数
// ============================================================================
FlowPage::~FlowPage()
{
    delete ui;
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
    ui->m_tree->clear();

    QString startDate = ui->m_filterStart->date().toString("yyyy-MM-dd");
    QString endDate   = ui->m_filterEnd->date().toString("yyyy-MM-dd");
    QString typeFilter = ui->m_filterType->currentText();

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
        ui->m_tree->addTopLevelItem(dateItem);

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


