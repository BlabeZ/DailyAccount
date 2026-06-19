/*
 * ============================================================================
 * 文件名: categorypage.h
 * 模块:   分类管理页面（GUI头文件）
 * 功能:   声明分类管理页面类 CategoryPage。
 *         该页面以左右两列布局展示支出分类和收入分类，
 *         支持新增自定义分类和删除未被使用的自定义分类。
 * 编码:   UTF-8
 *
 * 设计要点:
 *   1. 预设分类（如"餐饮"、"交通"、"工资"等系统内置分类）不可删除
 *   2. 正在被流水记录使用的自定义分类不可删除（防止数据不一致）
 *   3. 分类数据通过 CategoryManager 管理，通过 Ledger::save() 持久化
 *   4. 使用 QListWidget 展示分类列表，通过自定义数据角色（Qt::UserRole）
 *      存储每个分类项的元数据（名称、是否预设、是否使用中）
 * ============================================================================
 */

#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H

#include <QWidget>       // Qt窗口部件基类
#include <QListWidget>   // 列表控件 —— 以垂直滚动列表的方式展示每个分类条目
#include <QLineEdit>     // 单行文本输入框 —— 用于输入新分类的名称
#include <QPushButton>   // 按钮控件 —— "添加"和"删除选中"操作
#include <QLabel>        // 标签控件 —— 显示页面标题和说明文字

// 前向声明 —— 仅声明类名，不包含具体定义，
// 避免头文件循环依赖，同时减少编译时间
class Ledger;
class CategoryManager;

// ============================================================================
// 类名: CategoryPage
// 作用: **分类管理页面** —— 允许用户查看、新增和删除收支分类。
//       作为主窗口 QStackedWidget 中的一个页面存在。
// 存在理由:
//       1. 用户可能需要自定义分类来匹配自己的记账习惯（如添加"宠物"分类）
//       2. 提供清晰的分类概览：哪些是系统预设的（不可删），哪些是自定义的，
//          哪些正在被使用（受保护）
//       3. 通过受保护的删除逻辑，防止用户误删导致数据不一致
//       4. 所有变更通过 Ledger::save() 持久化到JSON文件，重启程序后依然有效
//
// 数据流:
//   CategoryPage ←→ CategoryManager（分类数据） ←→ Ledger（持久化）
//   用户操作 → CategoryManager::addCustomCategory / removeCustomCategory
//           → Ledger::save() → JSON文件
// ============================================================================
class CategoryPage : public QWidget {
    Q_OBJECT

public:
    // -------------------------------------------------------------------------
    // 构造函数
    // 参数 ledger: Ledger（账本）对象的引用 —— 用于调用 save() 持久化分类变更
    // 参数 catMan: CategoryManager（分类管理器）对象的引用 ——
    //              提供分类的增删查接口（addCustomCategory, removeCustomCategory,
    //              getCategories, isPreset, getInUseCategories 等）
    // 参数 parent: 父级窗口指针（默认nullptr）
    //
    // 初始化操作:
    //   1. 调用 QWidget(parent) 建立Qt父子关系
    //   2. 成员初始化列表保存 m_ledger 和 m_catMan 引用
    //   3. 函数体内调用 setupUI() 构建所有界面控件
    //
    // 注意: 构造函数不加载分类数据 —— 数据由外部在页面显示前调用 refresh() 加载
    // -------------------------------------------------------------------------
    explicit CategoryPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent = nullptr);

    // -------------------------------------------------------------------------
    // 方法名: refresh
    // 作用:   重新从 CategoryManager 读取所有分类，刷新两个 QListWidget 的显示
    // 调用时机:
    //          - 从其他页面切换到分类管理页时（由主窗口调用）
    //          - 添加或删除分类成功后（由 onAdd... / onDelete... 槽函数内部调用）
    // 说明:   内部委托给 loadCategories()
    // -------------------------------------------------------------------------
    void refresh();

private slots:
    // -------------------------------------------------------------------------
    // 以下四个槽函数分别处理两列（支出/收入）的添加和删除操作。
    // 槽函数（slot）是Qt信号槽机制中的"接收端" —— 当按钮被点击时，
    // 按钮发出的 clicked() 信号触发对应的槽函数执行。
    // 它们都是私有成员，因为只在本类内部通过信号连接使用。
    // -------------------------------------------------------------------------

    // 槽函数: 响应支出列的"+ 添加"按钮点击
    void onAddExpenseCategory();

    // 槽函数: 响应收入列的"+ 添加"按钮点击
    void onAddIncomeCategory();

    // 槽函数: 响应支出列的" 删除选中"按钮点击
    void onDeleteExpenseCategory();

    // 槽函数: 响应收入列的" 删除选中"按钮点击
    void onDeleteIncomeCategory();

private:
    // -------------------------------------------------------------------------
    // 方法名: setupUI
    // 作用:   一次性构建页面的完整界面结构（控件创建 + 布局安排）
    // 调用:   仅在构造函数中调用一次
    //
    // 创建的界面结构:
    //   ┌─────────────────────────────────────────────────────┐
    //   │   分类管理                                         │
    //   │  预设分类不可删除。正在使用的自定义分类受保护。        │
    //   ├────────────────────────┬────────────────────────────┤
    //   │   支出分类            │   收入分类                │
    //   │  ┌─────────────────┐   │  ┌─────────────────────┐   │
    //   │  │ 餐饮  [预设]     │   │  │ 工资  [预设]         │   │
    //   │  │ 交通  [预设]     │   │  │ 兼职  [自定义] (使用中)│   │
    //   │  │ 宠物  [自定义]    │   │  │ 理财  [预设]         │   │
    //   │  │ ...              │   │  │ ...                  │   │
    //   │  └─────────────────┘   │  └─────────────────────┘   │
    //   │  [输入新分类...] [添加] │  [输入新分类...] [添加]   │
    //   │  [ 删除选中]        │  [ 删除选中]            │
    //   └────────────────────────┴────────────────────────────┘
    // -------------------------------------------------------------------------
    void setupUI();

    // -------------------------------------------------------------------------
    // 方法名: loadCategories
    // 作用:   从 CategoryManager 拉取最新分类数据，更新两个 QListWidget
    //
    // 执行流程:
    //   第1步: 清空两个 QListWidget（m_expenseList 和 m_incomeList）
    //   第2步: 通过 CategoryManager 获取当前正在使用的分类集合
    //          （getInUseCategories）—— 用于判断哪些分类受删除保护
    //   第3步: 遍历支出分类:
    //          a) 通过 CategoryManager::isPreset 判断是否为预设分类
    //          b) 检查是否在"使用中"集合中
    //          c) 构造显示文本: "分类名  [预设/自定义]  (使用中)"
    //          d) 创建 QListWidgetItem，通过 setData(Qt::UserRole+N)
    //             存储元数据（分类名、isPreset标志、isInUse标志）
    //          e) 预设分类使用灰色前景色以示区分
    //   第4步: 对收入分类重复第3步的相同逻辑
    //
    // QListWidgetItem 的数据存储说明:
    //   每个列表项通过 Qt::UserRole + N 自定义数据角色存储附加信息:
    //     Qt::UserRole + 0: 分类名称 (QString)
    //     Qt::UserRole + 1: 是否为预设分类 (bool)
    //     Qt::UserRole + 2: 是否正被使用 (bool)
    //   这些数据在删除操作时被读取，用于权限校验
    // -------------------------------------------------------------------------
    void loadCategories();

    // ---------- 成员变量 ----------

    // m_ledger: 对数据层 Ledger 对象的引用。
    //           在添加/删除分类成功后调用 m_ledger.save() 将变更持久化。
    //           Ledger 内部将分类列表序列化为JSON并写入文件。
    Ledger& m_ledger;

    // m_catMan: 对 CategoryManager 对象的引用。
    //           提供分类管理的核心接口:
    //           - getCategories(type) → 获取某类型的所有分类名列表
    //           - isPreset(type, name) → 判断是否为预设分类
    //           - getInUseCategories(type) → 获取被流水记录引用的分类集合
    //           - addCustomCategory(type, name) → 添加自定义分类
    //           - removeCustomCategory(type, name) → 删除自定义分类
    CategoryManager& m_catMan;

    // m_expenseList: 支出分类列表控件 —— 位于页面左列
    //                每个条目（QListWidgetItem）显示一个分类及其状态标签
    QListWidget *m_expenseList;

    // m_incomeList: 收入分类列表控件 —— 位于页面右列，结构与支出列表对称
    QListWidget *m_incomeList;

    // m_expenseInput: 新支出分类名称的输入框 —— 与[+ 添加]按钮配对
    //                 位于支出列列表的下方
    QLineEdit   *m_expenseInput;

    // m_incomeInput: 新收入分类名称的输入框 —— 与[+ 添加]按钮配对
    //                位于收入列列表的下方
    QLineEdit   *m_incomeInput;
};

#endif // CATEGORYPAGE_H
