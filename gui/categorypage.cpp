/*
 * ============================================================================
 * 文件名: categorypage.cpp
 * 模块:   分类管理页面（GUI实现文件）
 * 功能:   实现分类管理页面的全部逻辑：
 *         1. 左右两列展示支出和收入分类列表
 *         2. 新增自定义分类（输入名称 + 合法性校验 + 持久化）
 *         3. 删除自定义分类（选中校验 + 预设检查 + 使用中检查 + 持久化）
 * 编码:   UTF-8
 *
 * 核心设计原则:
 *   - 预设分类（系统内置）不可删除 —— 保证基本记账功能始终可用
 *   - 正在被流水记录使用的分类不可删除 —— 防止出现"悬挂引用"导致数据不一致
 *   - 所有变更通过 Ledger::save() 立即持久化 —— 崩溃后数据不丢失
 *   - CategoryManager 是分类数据的唯一真相来源（Single Source of Truth）
 * ============================================================================
 */

#include "categorypage.h"
#include "category.h"       // RecordType 枚举（INCOME / EXPENSE）的定义
#include "ledger.h"         // Ledger 类 —— 提供 save() 持久化接口
#include <QVBoxLayout>      // 垂直布局 —— 页面内从上到下堆叠控件
#include <QHBoxLayout>      // 水平布局 —— 左右两列并排、输入行中的控件排列
#include <QFrame>           // 框架控件 —— 作为卡片容器（通过CSS class "card" 渲染圆角白底）
#include <QMessageBox>      // 消息框 —— 用于向用户展示操作结果（警告、提示、错误）
#include <QApplication>     // 应用程序类 —— 提供全局属性访问

// ============================================================================
// 构造函数
//
// 参数 ledger: 账本对象引用 —— 在添加/删除分类成功后调用其save()方法持久化
// 参数 catMan: 分类管理器引用 —— 提供所有分类相关的增删查接口
// 参数 parent: 父级窗口指针 —— Qt父子所有权体系管理此控件的生命周期
//
// 初始化顺序:
//   1. 调用 QWidget(parent) 将自身嵌入Qt窗口树
//   2. 成员初始化列表保存 m_ledger 和 m_catMan 的引用
//      （注意: 引用成员必须在初始化列表中绑定，且绑定后不可更改指向）
//   3. 调用 setupUI() 创建并布局所有界面控件
//
// 注意: 构造函数中不加载分类数据。数据加载由外部调用 refresh() 执行，
//       这允许主窗口在合适的时机（页面首次显示前）再加载数据，
//       避免构造阶段的不必要数据库访问。
// ============================================================================
CategoryPage::CategoryPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QWidget(parent), m_ledger(ledger), m_catMan(catMan)
{
    setupUI();
}

// ============================================================================
// setupUI — 构建分类管理页面的完整界面
//
// ==================== 两列布局设计 ====================
// 页面采用左右等宽的两列布局：
//   左列: 支出分类管理（红色系标题）
//   右列: 收入分类管理（绿色系标题）
// 每列的结构完全相同（从上到下）:
//   ┌────────────────────┐
//   │  标题（/ + 类型名） │
//   │  ┌──────────────┐  │
//   │  │ QListWidget  │  │  ← 可滚动的分类列表，
//   │  │ (占主要空间)  │  │     显示每个分类的名称+状态标签
//   │  └──────────────┘  │
//   │  [输入框] [ 添加] │  ← 添加新分类的操作行
//   │  [ 删除选中]     │  ← 删除选中分类的按钮
//   └────────────────────┘
//
// 伸缩因子（stretch factor）分配:
//   - QListWidget 获得伸缩因子 1 —— 它在垂直方向上占据所有多余空间
//   - 不需要伸缩因子的控件（标题、按钮等）保持其自然高度
//   这样的分配保证列表在窗口放大时获得更多空间，而按钮不会跟着变大
// ============================================================================
void CategoryPage::setupUI()
{
    // ======== 创建主垂直布局 ========
    // 主布局将所有内容从上到下排列: 标题 → 说明文字 → 两列卡片
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);       // 四周24px内边距
    mainLayout->setSpacing(16);                            // 子控件间距16px

    // ======== 页面标题 ========
    QLabel *title = new QLabel(QStringLiteral("\U0001F3F7 分类管理"));
    title->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #2C3E50; "
        "background: transparent;");                       // 透明背景，继承页面底色
    mainLayout->addWidget(title);

    // ======== 说明文字（提示预设分类和自定义分类的区别） ========
    // 这段文字解释页面规则，帮助用户理解为什么有些分类不能删除
    QLabel *subtitle = new QLabel(QStringLiteral(
        "预设分类不可删除。正在使用的自定义分类受保护。"));
    subtitle->setStyleSheet(
        "color: #95A5A6; font-size: 12px; background: transparent;");
    mainLayout->addWidget(subtitle);

    // ======== 创建左右两列的水平布局 ========
    // 该布局包含两个等宽的 QFrame 卡片（支出列 + 收入列）
    QHBoxLayout *columnsLayout = new QHBoxLayout;
    columnsLayout->setSpacing(16);

    // ================================================================
    //  左列: 支出分类管理
    // ================================================================
    QFrame *expenseFrame = new QFrame;
    // setProperty("class", "card") 为控件设置CSS类名
    // 全局样式表中定义了 .card 的样式（白色背景、圆角、阴影等）
    // 相当于在CSS中写: .card { background: white; border-radius: 12px; ... }
    expenseFrame->setProperty("class", "card");
    QVBoxLayout *expenseLayout = new QVBoxLayout(expenseFrame);

    // ---- 列标题:  支出分类 ----
    QLabel *expenseTitle = new QLabel(QStringLiteral("\U0001F4C9 支出分类"));
    expenseTitle->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #E74C3C; "  // 红色 —— 与支出主题色一致
        "background: transparent; padding-bottom: 8px;");
    expenseLayout->addWidget(expenseTitle);

    // ---- QListWidget: 支出分类列表 ----
    // QListWidget 是Qt提供的列表控件，每一项是一个 QListWidgetItem。
    // 它可以包含文本、图标，并通过 setData() 存储任意自定义数据。
    // 用户点击某一项时，该项成为"当前项"（currentItem），
    // 后续的"删除选中"操作读取当前项进行删除。
    m_expenseList = new QListWidget;
    m_expenseList->setStyleSheet(
        // QListWidget::item: 设置每个列表项的样式
        // padding: 上下10px 左右14px —— 给条目足够的可点击区域
        "QListWidget::item { padding: 10px 14px; }"
        // QListWidget::item:selected: 被选中项的样式
        // 浅红色背景 #FDEDEC，配合暗色文字，视觉反馈清晰
        "QListWidget::item:selected { background: #FDEDEC; color: #2C3E50; }");
    expenseLayout->addWidget(m_expenseList, 1);             // 伸缩因子1 —— 占据剩余空间

    // ---- 添加新支出分类的输入行 ----
    QHBoxLayout *expenseAddRow = new QHBoxLayout;

    m_expenseInput = new QLineEdit;
    // setPlaceholderText: 当输入框为空时显示浅灰色提示文字（placeholder）
    // 用户一开始输入任何字符，提示文字自动消失
    m_expenseInput->setPlaceholderText(QStringLiteral("新支出分类名称..."));
    expenseAddRow->addWidget(m_expenseInput, 1);            // 伸缩因子1 —— 输入框占主要宽度

    // " 添加"按钮 —— 红色系（与支出主题一致）
    QPushButton *btnAddExpense = new QPushButton(QStringLiteral(" 添加"));
    btnAddExpense->setStyleSheet(
        "QPushButton { background: #E74C3C; color: white; padding: 8px 16px; }"
        // :hover 伪状态 —— 鼠标悬停时颜色变深，提供交互反馈
        "QPushButton:hover { background: #CB4335; }");
    // 信号-槽连接: 按钮clicked信号 → onAddExpenseCategory槽
    // 当用户点击按钮时，Qt自动调用 onAddExpenseCategory() 方法
    connect(btnAddExpense, &QPushButton::clicked, this, &CategoryPage::onAddExpenseCategory);
    expenseAddRow->addWidget(btnAddExpense);
    expenseLayout->addLayout(expenseAddRow);

    // ---- 删除支出分类按钮 ----
    QPushButton *btnDelExpense = new QPushButton(QStringLiteral("\U0001F5D1 删除选中"));
    btnDelExpense->setStyleSheet(
        // 非填充样式：浅灰背景 + 红色边框 + 红色文字 —— 表达"危险操作"的视觉语言
        "QPushButton { background: #ECF0F1; color: #E74C3C; padding: 8px; "
        "border: 1px solid #E74C3C; border-radius: 6px; }"
        // 悬停时背景变为浅红色 #FDEDEC，进一步强化"这是一个删除操作"的暗示
        "QPushButton:hover { background: #FDEDEC; }");
    connect(btnDelExpense, &QPushButton::clicked, this, &CategoryPage::onDeleteExpenseCategory);
    expenseLayout->addWidget(btnDelExpense);

    // 将支出列卡片加入两列布局，伸缩因子1（与收入列等宽）
    columnsLayout->addWidget(expenseFrame, 1);

    // ================================================================
    //  右列: 收入分类管理（结构与左列完全对称）
    // ================================================================
    QFrame *incomeFrame = new QFrame;
    incomeFrame->setProperty("class", "card");
    QVBoxLayout *incomeLayout = new QVBoxLayout(incomeFrame);

    // ---- 列标题:  收入分类 ----
    QLabel *incomeTitle = new QLabel(QStringLiteral("\U0001F4C8 收入分类"));
    incomeTitle->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #27AE60; "  // 绿色 —— 与收入主题色一致
        "background: transparent; padding-bottom: 8px;");
    incomeLayout->addWidget(incomeTitle);

    // ---- QListWidget: 收入分类列表 ----
    m_incomeList = new QListWidget;
    m_incomeList->setStyleSheet(
        "QListWidget::item { padding: 10px 14px; }"
        // 收入列表选中项为浅绿色背景 #EAFAF1 —— 与收入主题色呼应
        "QListWidget::item:selected { background: #EAFAF1; color: #2C3E50; }");
    incomeLayout->addWidget(m_incomeList, 1);

    // ---- 添加新收入分类的输入行 ----
    QHBoxLayout *incomeAddRow = new QHBoxLayout;

    m_incomeInput = new QLineEdit;
    m_incomeInput->setPlaceholderText(QStringLiteral("新收入分类名称..."));
    incomeAddRow->addWidget(m_incomeInput, 1);

    QPushButton *btnAddIncome = new QPushButton(QStringLiteral(" 添加"));
    btnAddIncome->setStyleSheet(
        "QPushButton { background: #27AE60; color: white; padding: 8px 16px; }"
        "QPushButton:hover { background: #219A52; }");     // 悬停时颜色加深
    connect(btnAddIncome, &QPushButton::clicked, this, &CategoryPage::onAddIncomeCategory);
    incomeAddRow->addWidget(btnAddIncome);
    incomeLayout->addLayout(incomeAddRow);

    // ---- 删除收入分类按钮 ----
    QPushButton *btnDelIncome = new QPushButton(QStringLiteral("\U0001F5D1 删除选中"));
    btnDelIncome->setStyleSheet(
        "QPushButton { background: #ECF0F1; color: #E74C3C; padding: 8px; "
        "border: 1px solid #E74C3C; border-radius: 6px; }"
        "QPushButton:hover { background: #FDEDEC; }");
    connect(btnDelIncome, &QPushButton::clicked, this, &CategoryPage::onDeleteIncomeCategory);
    incomeLayout->addWidget(btnDelIncome);

    columnsLayout->addWidget(incomeFrame, 1);
    mainLayout->addLayout(columnsLayout, 1);                // 伸缩因子1 —— 两列占主空间
}

// ============================================================================
// refresh — 刷新页面数据
// 这是一个公开接口，由主窗口在每次切换到分类管理页时调用。
// 确保用户看到的始终是最新的分类列表状态。
// ============================================================================
void CategoryPage::refresh()
{
    loadCategories();
}

// ============================================================================
// loadCategories — 从 CategoryManager 加载全部分类并填充两个列表
//
// ==================== 预设分类 vs 自定义分类 ====================
//
// 预设分类（Preset Category）:
//   - 程序内置的分类，如"餐饮"、"交通"、"购物"、"工资"等
//   - 在 CategoryManager 初始化时自动创建
//   - 标记 isPreset = true
//   - 特点：不可删除 —— 保证基础记账功能始终可用
//   - 在列表中以灰色文字显示（视觉上与自定义分类区分）
//
// 自定义分类（Custom Category）:
//   - 用户通过本页面的"添加"按钮创建的分类
//   - 添加后由 Ledger::save() 持久化到JSON文件
//   - 标记 isPreset = false
//   - 可以删除，但有一个前提条件：该分类未被任何流水记录使用
//
// ==================== "使用中"保护机制 ====================
//
// isInUse 标志是一个安全网，防止用户删除正在被流水记录引用的分类。
// 举例场景:
//   用户创建了"宠物"分类并记录了3笔支出
//   → "宠物"出现在 inUseExpense 集合中（因为存在引用它的流水记录）
//   → 用户尝试删除"宠物" → 列表项的 isInUse = true
//   → 删除槽函数检查到 isInUse 为 true → 弹出警告，拒绝删除
//
// 这种保护是必要的，因为如果删除了一个仍在使用的分类：
//   1. 现有流水记录会出现"孤儿引用" —— 指向一个不存在的分类
//   2. 统计页面的饼图可能无法正确归类这些流水
//   3. 数据完整性被破坏
//
// ==================== QListWidgetItem 的数据存储 ====================
//
// Qt::UserRole 是Qt中"用户自定义数据角色"的起始值（值为 256）。
// 每个 QListWidgetItem 可以存储多个"角色"的数据:
//   - Qt::DisplayRole (0): 显示的文本（setText 实际设置的就是这个角色）
//   - Qt::UserRole (256): 第1个自定义数据 —— 存储分类名称（QString）
//   - Qt::UserRole + 1 (257): 第2个自定义数据 —— 存储 isPreset 标志（bool）
//   - Qt::UserRole + 2 (258): 第3个自定义数据 —— 存储 isInUse 标志（bool）
//
// 这些自定义数据在界面上不可见，但可以在删除操作时通过
// item->data(Qt::UserRole + N) 读取，用于权限判断。
// ============================================================================
void CategoryPage::loadCategories()
{
    // ---- 清空两个列表（移除所有旧的 QListWidgetItem） ----
    // clear() 会删除所有条目对象（Qt负责释放内存）
    m_expenseList->clear();
    m_incomeList->clear();

    // ---- 获取"正在被使用"的分类集合 ----
    // CategoryManager::getInUseCategories 扫描所有流水记录，收集被引用的分类名
    // 返回类型是 std::set<std::string> —— 集合（不含重复元素）
    auto inUseExpense = m_catMan.getInUseCategories(RecordType::EXPENSE);
    auto inUseIncome  = m_catMan.getInUseCategories(RecordType::INCOME);

    // ================================================================
    //  填充支出分类列表
    // ================================================================
    auto expenseCats = m_catMan.getCategories(RecordType::EXPENSE);
    for (const auto& cat : expenseCats) {
        // ---- 判断分类状态 ----
        // isPreset: CategoryManager::isPreset 查询该分类是否在预设列表中
        bool isPreset = m_catMan.isPreset(RecordType::EXPENSE, cat);

        // isInUse: 检查该分类名是否出现在"使用中"集合中
        // find() 返回迭代器，!= end() 表示找到了
        bool isInUse  = (inUseExpense.find(cat) != inUseExpense.end());

        // ---- 构造显示文本 ----
        // 格式: "分类名称  [预设/自定义]  (使用中)"
        // 例如: "餐饮  [预设]"                   （预设，未使用）
        //        "宠物  [自定义]  (使用中)"      （自定义，正被使用）
        QString display = QString::fromStdString(cat);
        if (isPreset)
            display += QStringLiteral("  [预设]");     // 预设标记
        else
            display += QStringLiteral("  [自定义]");   // 自定义标记
        if (isInUse)
            display += QStringLiteral("  (使用中)");   // 使用中标记

        // ---- 创建列表项 ----
        QListWidgetItem *item = new QListWidgetItem(display);

        // 预设分类使用灰色文字，在视觉上与自定义分类区分
        if (isPreset) {
            item->setForeground(QColor("#95A5A6"));    // 浅灰色前景
        }

        // ---- 将元数据存储到列表项的自定义数据角色中 ----
        // 这些数据在界面不可见，但 onDelete... 槽函数读取它们进行权限判断
        item->setData(Qt::UserRole, QString::fromStdString(cat));  // 角色0: 分类名称
        item->setData(Qt::UserRole + 1, isPreset);                 // 角色1: 是否预设
        item->setData(Qt::UserRole + 2, isInUse);                  // 角色2: 是否使用中

        // 将列表项添加到 QListWidget 中
        m_expenseList->addItem(item);
    }

    // ================================================================
    //  填充收入分类列表（逻辑与支出列表完全相同）
    // ================================================================
    auto incomeCats = m_catMan.getCategories(RecordType::INCOME);
    for (const auto& cat : incomeCats) {
        bool isPreset = m_catMan.isPreset(RecordType::INCOME, cat);
        bool isInUse  = (inUseIncome.find(cat) != inUseIncome.end());

        QString display = QString::fromStdString(cat);
        if (isPreset) display += QStringLiteral("  [预设]");
        else display += QStringLiteral("  [自定义]");
        if (isInUse) display += QStringLiteral("  (使用中)");

        QListWidgetItem *item = new QListWidgetItem(display);
        if (isPreset) {
            item->setForeground(QColor("#95A5A6"));
        }
        item->setData(Qt::UserRole, QString::fromStdString(cat));
        item->setData(Qt::UserRole + 1, isPreset);
        item->setData(Qt::UserRole + 2, isInUse);
        m_incomeList->addItem(item);
    }
}

// ============================================================================
// onAddExpenseCategory — 添加新的支出自定义分类
//
// 添加流程:
//  第1步: 读取输入框文本并去除首尾空白（trimmed）
//         → 防止用户只输入空格，或名称前后带有无意义的空白字符
//  第2步: 空值检查
//         → 如果trimmed后为空，弹出"请输入分类名称"警告框，return终止
//  第3步: 调用 CategoryManager::addCustomCategory(EXPENSE, name)
//         → 内部检查: 名称是否已存在（含预设名称）、名称是否合法
//         → 返回 true = 添加成功，false = 失败（重复或无效）
//  第4a步（成功）: 调用 m_ledger.save() 持久化分类数据到JSON文件
//                  → 然后调用 loadCategories() 刷新列表显示
//                  → 清空输入框（准备下一次输入）
//  第4b步（失败）: 弹出"分类名称已存在或无效"警告框
//                  → 输入框不清空，方便用户修正后重试
//
// ==================== 数据持久化 ====================
// m_ledger.save() 将当前内存中的全部分类数据（包含新增的自定义分类）
// 序列化为JSON格式并写入文件。这样程序重启后，自定义分类仍然存在。
// 典型的JSON结构:
//   "categories": {
//     "expense": ["餐饮", "交通", "购物", "宠物"],
//     "income":  ["工资", "理财", "兼职"]
//   }
// 其中"宠物"是用户通过此函数添加的自定义分类。
// ============================================================================
void CategoryPage::onAddExpenseCategory()
{
    // ---- 第1步: 读取并清理输入 ----
    // text() 返回输入框的原始文本
    // trimmed() 返回去除首尾空白字符（空格、制表符、换行等）后的文本
    QString name = m_expenseInput->text().trimmed();

    // ---- 第2步: 空值校验 ----
    if (name.isEmpty()) {
        // QMessageBox::warning: 显示一个带警告图标和"确定"按钮的对话框
        // 参数: (父窗口, 窗口标题, 提示文字)
        QMessageBox::warning(this,
                             QStringLiteral("输入错误"),
                             QStringLiteral("请输入分类名称。"));
        return;                                          // 提前返回，不执行后续添加逻辑
    }

    // ---- 第3步: 调用 CategoryManager 添加分类 ----
    // addCustomCategory 内部逻辑:
    //   1. 检查该名称是否已被（预设或自定义）分类占用
    //   2. 检查名称是否合法（非空、无非法字符等）
    //   3. 如果通过，将新分类加入内部数据结构并返回 true
    if (m_catMan.addCustomCategory(RecordType::EXPENSE, name.toStdString())) {
        // ---- 第4a步: 添加成功 ----
        m_ledger.save();                                  // 立即持久化到文件
        loadCategories();                                 // 刷新列表显示（新分类出现在列表中）
        m_expenseInput->clear();                          // 清空输入框
    } else {
        // ---- 第4b步: 添加失败 ----
        // 最常见的失败原因: 分类名与已有分类重复
        QMessageBox::warning(this,
                             QStringLiteral("添加失败"),
                             QStringLiteral("分类名称已存在或无效。"));
        // 输入框保留用户刚才输入的文本，方便修改后重试
    }
}

// ============================================================================
// onAddIncomeCategory — 添加新的收入自定义分类
//
// 逻辑与 onAddExpenseCategory 完全对称，区别仅在于:
//   - 输入框: m_incomeInput 而不是 m_expenseInput
//   - 记录类型: RecordType::INCOME 而不是 EXPENSE
//
// 这种对称设计是故意的 —— 虽然存在代码重复，但保持了两个方向的操作
// 完全独立，便于后续如果收入/支出分类需要不同的添加逻辑时可以独立修改。
// ============================================================================
void CategoryPage::onAddIncomeCategory()
{
    QString name = m_incomeInput->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("输入错误"),
                             QStringLiteral("请输入分类名称。"));
        return;
    }

    if (m_catMan.addCustomCategory(RecordType::INCOME, name.toStdString())) {
        m_ledger.save();                                  // 持久化
        loadCategories();                                 // 刷新列表
        m_incomeInput->clear();                           // 清空输入框
    } else {
        QMessageBox::warning(this,
                             QStringLiteral("添加失败"),
                             QStringLiteral("分类名称已存在或无效。"));
    }
}

// ============================================================================
// onDeleteExpenseCategory — 删除选中的支出分类
//
// 删除流程（带三级安全校验）:
//  第1步: 获取当前选中的列表项
//         QListWidget::currentItem() 返回用户点击选中的 QListWidgetItem*
//         如果用户未选中任何项，返回 nullptr
//
//  第2步（校验1 —— 是否选中）:
//         如果 currentItem 为 nullptr → 弹出提示"请先选中要删除的分类"
//         → return 终止
//
//  第3步: 读取列表项中存储的元数据
//         - isPreset: 从 Qt::UserRole+1 读取（由 loadCategories 存入）
//         - isInUse:  从 Qt::UserRole+2 读取
//
//  第4步（校验2 —— 预设检查）:
//         如果 isPreset == true → 弹出"预设分类不可删除"
//         → return 终止
//
//  第5步（校验3 —— 使用中检查）:
//         如果 isInUse == true → 弹出"该分类正在被使用中，请先删除或修改相关流水记录"
//         → return 终止
//
//  第6步: 所有校验通过，执行删除
//         - 从 Qt::UserRole 读取分类名称
//         - 调用 CategoryManager::removeCustomCategory(EXPENSE, name)
//         - 调用 m_ledger.save() 持久化变更
//         - 调用 loadCategories() 刷新列表
//
// ==================== 三级校验的设计意图 ====================
// 校验1（选中检查）: 防止空指针解引用（读取 null->data() 会导致崩溃）
// 校验2（预设检查）: 保证基础功能不可破坏（如不能删除"餐饮"导致无法记录餐饮支出）
// 校验3（使用中检查）: 防止数据不一致（如删除了"宠物"但仍有3笔宠物支出的流水记录）
//
// 只有全部校验通过，才真正执行删除 —— 这种"先校验再操作"的模式称为
// 防御性编程（Defensive Programming），是GUI应用减少用户误操作风险的标准做法。
// ============================================================================
void CategoryPage::onDeleteExpenseCategory()
{
    // ---- 第1步: 获取当前选中的列表项 ----
    auto *item = m_expenseList->currentItem();

    // ---- 校验1: 未选中任何项 ----
    if (!item) {
        QMessageBox::information(this,
                                 QStringLiteral("提示"),
                                 QStringLiteral("请先选中要删除的分类。"));
        return;
    }

    // ---- 第3步: 读取元数据 ----
    // toBool(): QVariant → bool 的转换
    // 由于 loadCategories 中确实存储了 bool 值，转换总是成功的
    bool isPreset = item->data(Qt::UserRole + 1).toBool();
    bool isInUse  = item->data(Qt::UserRole + 2).toBool();

    // ---- 校验2: 预设分类不可删除 ----
    if (isPreset) {
        QMessageBox::warning(this,
                             QStringLiteral("无法删除"),
                             QStringLiteral("预设分类不可删除。"));
        return;
    }

    // ---- 校验3: 正在使用的自定义分类不可删除 ----
    if (isInUse) {
        QMessageBox::warning(this,
                             QStringLiteral("无法删除"),
                             // 引导用户: 先处理关联的流水记录，再回来删除分类
                             QStringLiteral("该分类正在被使用中，请先删除或修改相关流水记录。"));
        return;
    }

    // ---- 第6步: 执行删除 ----
    QString name = item->data(Qt::UserRole).toString();   // 读取分类名称

    // removeCustomCategory 从 CategoryManager 的内部数据结构中移除该分类
    // 返回值: true=删除成功, false=失败（理论上这里总是成功，因为校验已通过）
    if (m_catMan.removeCustomCategory(RecordType::EXPENSE, name.toStdString())) {
        m_ledger.save();                                  // 持久化删除操作到文件
        loadCategories();                                 // 刷新列表 —— 被删的分类消失
    }
}

// ============================================================================
// onDeleteIncomeCategory — 删除选中的收入分类
//
// 逻辑与 onDeleteExpenseCategory 完全对称，区别仅在于:
//   - 列表控件: m_incomeList 而不是 m_expenseList
//   - 记录类型: RecordType::INCOME 而不是 EXPENSE
//
// 详细注释请参见 onDeleteExpenseCategory 函数。
// ============================================================================
void CategoryPage::onDeleteIncomeCategory()
{
    auto *item = m_incomeList->currentItem();

    // ---- 校验1: 未选中 ----
    if (!item) {
        QMessageBox::information(this,
                                 QStringLiteral("提示"),
                                 QStringLiteral("请先选中要删除的分类。"));
        return;
    }

    bool isPreset = item->data(Qt::UserRole + 1).toBool();
    bool isInUse  = item->data(Qt::UserRole + 2).toBool();

    // ---- 校验2: 预设保护 ----
    if (isPreset) {
        QMessageBox::warning(this,
                             QStringLiteral("无法删除"),
                             QStringLiteral("预设分类不可删除。"));
        return;
    }

    // ---- 校验3: 使用中保护 ----
    if (isInUse) {
        QMessageBox::warning(this,
                             QStringLiteral("无法删除"),
                             QStringLiteral("该分类正在被使用中，请先删除或修改相关流水记录。"));
        return;
    }

    // ---- 执行删除 ----
    QString name = item->data(Qt::UserRole).toString();
    if (m_catMan.removeCustomCategory(RecordType::INCOME, name.toStdString())) {
        m_ledger.save();                                  // 持久化
        loadCategories();                                 // 刷新列表
    }
}
