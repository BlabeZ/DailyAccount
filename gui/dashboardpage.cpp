/*
 * ===========================================================================
 * 文件名称：dashboardpage.cpp
 * 所属模块：GUI 图形用户界面 - 概览页面
 * 文件编码：UTF-8（兼容 GB2312 中文字符）
 * 功能描述：
 *     本文件实现了「概览页面」类 DashboardPage 的全部方法。
 *     概览页面是用户打开程序后看到的主页面，提供财务状况的"一目了然"
 *     视图。页面的设计目标是让用户无需任何操作就能快速了解：
 *       - 本月赚了多少钱、花了多少钱、还剩多少钱（汇总卡片）
 *       - 最近有哪些流水记录（流水表格）
 *       - 钱主要花在了哪些类别上（分类分布进度条）
 *
 *     页面布局结构（从上到下）：
 *     ┌──────────────────────────────────────────────────┐
 *     │ 页面标题："概览"                                 │
 *     ├──────────┬──────────┬────────────────────────────┤
 *     │ 收入卡片 │ 支出卡片 │ 结余卡片                   │
 *     │ 本月收入 │ 本月支出 │ 本月结余                   │
 *     │ +xxxx  │ -xxxx  │ +xxxx                      │
 *     │ (绿色)  │ (红色)  │ (蓝/红色)                   │
 *     ├──────────┴──────────┴─────┬──────────────────────┤
 *     │ 最近流水记录表格（10条） │ 本月支出分布          │
 *     │ 日期|类型|金额|分类|备注 │ 餐饮  ████████ 45%  │
 *     │ ...                      │ 交通  █████    25%  │
 *     │                          │ 购物  ███      15%  │
 *     └──────────────────────────┴──────────────────────┘
 *
 * 依赖关系：
 *     本文件依赖于 Ledger（账本数据类）和 CategoryManager（分类管理器），
 *     通过这两个对象获取流水记录和分类统计信息。
 * ===========================================================================
 */

#include "dashboardpage.h"
#include "ledger.h"
#include "category.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QProgressBar>
#include <QHeaderView>
#include <QScrollArea>
#include <ctime>

/*
 * ===========================================================================
 * 构造函数：DashboardPage
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     初始化概览页面对象。首先将父指针传递给 QWidget 基类构造函数，
 *     然后保存账本和分类管理器的引用为成员变量，最后调用 setupUI()
 *     构建页面的全部界面元素。
 *
 *     构造完成后，页面上的控件已经全部创建并布局到位，但数据尚未填充。
 *     需要在外部调用 refresh() 方法来加载实际数据。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     ledger  - 账本对象（Ledger）的引用。页面通过它获取所有流水数据
 *               和统计信息。使用引用确保该对象在页面存在期间始终有效。
 *     catMan  - 分类管理器（CategoryManager）的引用。页面通过它获取
 *               分类的显示名称等信息。同样使用引用确保有效性。
 *     parent  - Qt 父子对象树中的父控件指针，默认为 nullptr。
 *               当设为 QStackedWidget 等容器时，页面会随父容器一起销毁。
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 调用 QWidget(parent) 基类构造，将页面嵌入 Qt 父子对象树
 *     2. 将 ledger 引用保存到 m_ledger 成员变量
 *     3. 将 catMan 引用保存到 m_catMan 成员变量
 *     4. 调用 setupUI() 搭建所有界面控件
 * ===========================================================================
 */
DashboardPage::DashboardPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QWidget(parent), m_ledger(ledger), m_catMan(catMan)
{
    setupUI();  // 构建页面上的所有界面元素（标题、卡片、表格、分布图）
}

/*
 * ===========================================================================
 * 方法：setupUI
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     构建概览页面的完整用户界面。这是页面布局定义的核心方法，
 *     所有可视控件都在此方法中被创建、设置样式并添加到布局中。
 *
 *     布局采用垂直方向为主、水平方向为辅助的嵌套布局结构：
 *       - 顶层：QVBoxLayout（垂直布局）
 *         - 页面标题行
 *         - 汇总卡片行（QHBoxLayout 水平布局，3 张卡片并排）
 *         - 底部双栏（QHBoxLayout 水平布局）
 *           - 左栏：最近流水记录表格（占 3/5 宽度）
 *           - 右栏：本月支出分类分布（占 2/5 宽度）
 *
 *     注意：本方法只创建控件的"外壳"和"结构"，不填充实际数据。
 *     数据的填充由 refresh() → refreshSummary() / refreshRecentRecords()
 *     / refreshCategoryBreakdown() 方法链完成。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ===========================================================================
 */
void DashboardPage::setupUI()
{
    /*
     * 步骤 1：创建顶层垂直布局（m_mainLayout）
     *
     * QVBoxLayout 将其子控件从上到下依次垂直排列。
     * 构造时的参数 'this' 表示将这个布局设置为本 QWidget 的布局管理器。
     *
     * setContentsMargins(24,24,24,24)：
     *   设置布局四周的内边距为 24 像素。
     *   顺序是：左、上、右、下（与 CSS 的上右下左不同，需注意）。
     *
     * setSpacing(20)：
     *   设置布局中子控件之间的默认垂直间距为 20 像素。
     *   这意味着：标题与卡片行之间、卡片行与底部区域之间各有 20 像素的间隔。
     */
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(24, 24, 24, 24);
    m_mainLayout->setSpacing(20);

    /*
     * 步骤 2：页面标题
     *
     * 使用大号加粗字体显示"概览"二字，让用户知道当前所处的页面。
     * 字体大小为 22 像素，颜色为深色（#2C3E50，接近黑色的深蓝灰色）。
     * background: transparent 确保标签不会有不透明的背景遮挡后面的颜色。
     */
    QLabel *pageTitle = new QLabel("概览");
    pageTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #2C3E50; "
                              "background: transparent; padding: 0;");
    m_mainLayout->addWidget(pageTitle);

    /*
     * ===================================================================
     * 步骤 3：汇总卡片行（水平布局，三张卡片并排显示）
     * ===================================================================
     *
     * 三张卡片的设计理念：
     *   用户打开应用后，最关心的是三个核心数字——赚了多少、花了多少、
     *   还剩多少。将它们放在页面最显眼的位置（顶部），用大号彩色字体
     *   和白色卡片背景突出显示，让用户一眼就能获取关键财务信息。
     *
     * 卡片的颜色设计逻辑：
     *   - 收入卡片：左侧绿色边框（#27AE60），金额绿色 —— 绿色给人以
     *     "增长、积极"的心理暗示，适合表示收入
     *   - 支出卡片：左侧红色边框（#E74C3C），金额红色 —— 红色给人以
     *     "警醒、支出"的心理暗示，提醒用户关注开销
     *   - 结余卡片：左侧蓝色边框（#3498DB），金额正数蓝色、负数红色 ——
     *     蓝色给人以"稳定、中性"的感觉，正结余用蓝色表示"稳健"，
     *     负结余用红色表示"赤字警告"
     *
     * 使用 QHBoxLayout 水平布局将三张卡片从左到右并排排列。
     * 每张卡片使用 stretch factor = 1，确保三张卡片等宽。
     */
    QHBoxLayout *cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(16);  // 卡片之间的水平间距为 16 像素

    /*
     * 创建三个 QLabel 作为汇总卡片的容器。
     * 这里并非使用 createSummaryCard() 方法创建 QFrame 卡片，
     * 而是直接用 QLabel + HTML 富文本的方式模拟卡片外观。
     * 这是一种简化的实现方式——通过 HTML/CSS 样式在 QLabel 中
     * 渲染出白色背景、彩色左边框、大号金额文字的卡片效果。
     *
     * 三个标签的内容由 refreshSummary() 方法填充。
     * 在 setupUI 阶段只创建空的 QLabel，等待后续数据刷新。
     */
    m_cardIncome  = new QLabel;   // 总收入卡片（绿色主题）
    m_cardExpense = new QLabel;   // 总支出卡片（红色主题）
    m_cardBalance = new QLabel;   // 结余卡片（蓝色/红色主题，根据正负动态切换）

    // 将三张卡片添加到水平布局中，stretch factor = 1 使它们等宽
    cardsLayout->addWidget(m_cardIncome, 1);
    cardsLayout->addWidget(m_cardExpense, 1);
    cardsLayout->addWidget(m_cardBalance, 1);
    m_mainLayout->addLayout(cardsLayout);

    /*
     * ===================================================================
     * 步骤 4：底部双栏区域（水平布局）
     * ===================================================================
     *
     * 底部区域使用 QHBoxLayout 水平分为左右两栏：
     *   - 左侧（stretch factor = 3，占 60% 宽度）：最近流水记录表格
     *   - 右侧（stretch factor = 2，占 40% 宽度）：本月支出分类分布
     *
     * 左右宽度比例为 3:2，因为流水表格有 5 列数据，需要更多空间，
     * 而分类分布只有进度条，相对紧凑。
     */
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(16);  // 左右两栏之间的间距为 16 像素

    /*
     * -------------------------------------------------------------------
     * 左栏：最近流水记录
     * -------------------------------------------------------------------
     * 使用 QFrame 作为容器，设置 CSS 类名为 "card"（通过外部样式表
     * 定义白色背景、圆角、阴影等卡片样式）。
     *
     * 内部使用 QVBoxLayout 垂直布局：
     *   - 区域标题（" 最近流水记录"）
     *   - QTableWidget 表格（5 列，动态行数）
     */
    QFrame *recentFrame = new QFrame;
    recentFrame->setProperty("class", "card");  // 设置 CSS 类名，关联外部样式表

    QVBoxLayout *recentLayout = new QVBoxLayout(recentFrame);

    // 区域小标题
    QLabel *recentTitle = new QLabel(" 最近流水记录");
    recentTitle->setStyleSheet("font-size: 15px; font-weight: bold; background: transparent; "
                                "padding-bottom: 8px;");
    recentLayout->addWidget(recentTitle);

    /*
     * 创建流水记录表格（QTableWidget）
     *
     * 构造函数参数：
     *   参数 1（0）：初始行数设为 0（行数由数据决定，在 refresh 时动态设置）
     *   参数 2（5）：列数固定为 5 列
     *
     * 5 列的内容和含义：
     *   列 0：日期（date）       —— 流水发生的日期，格式如 "2024-01-15"
     *   列 1：类型（type）       —— "收入" 或 "支出"
     *   列 2：金额（amount）     —— 流水金额，收入前加 "+"，支出前加 "-"
     *   列 3：分类（category）   —— 流水所属的分类名称，如 "工资"、"餐饮"
     *   列 4：备注（note）       —— 用户添加的备注信息
     */
    m_recentTable = new QTableWidget(0, 5);

    // 设置表格各列的标题（表头）
    m_recentTable->setHorizontalHeaderLabels({"日期", "类型", "金额", "分类", "备注"});

    /*
     * 设置表格各列的尺寸调整模式
     * ResizeToContents：列宽自动调整为刚好能容纳该列中最宽的内容
     * StretchLastSection：最后一列（备注列）自动拉伸填满剩余空间
     *
     * 列 0（日期）—— 根据内容自动调整宽度
     * 列 1（类型）—— 只有"收入"/"支出"两种值，宽度固定为内容宽度
     * 列 2（金额）—— 根据金额字符串长度自动调整
     * 列 3（分类）—— 根据分类名称长度自动调整
     * 列 4（备注）—— 使用 setStretchLastSection(true)，自动填满表格剩余宽度
     */
    m_recentTable->horizontalHeader()->setStretchLastSection(true);
    m_recentTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_recentTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_recentTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_recentTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    /*
     * 设置表格的交互行为
     * NoEditTriggers：禁止用户双击编辑表格单元格（只读表格）
     * SelectRows：点击单元格时选中整行（方便用户查看一行的完整信息）
     * verticalHeader()->setVisible(false)：隐藏左侧的行号标签（第 1, 2, 3...行）
     */
    m_recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentTable->verticalHeader()->setVisible(false);

    // 设置表格最小高度为 280 像素，确保即使数据较少也能保持合理的视觉大小
    m_recentTable->setMinimumHeight(280);

    recentLayout->addWidget(m_recentTable);
    bottomLayout->addWidget(recentFrame, 3);  // stretch factor = 3，占 60% 宽度

    /*
     * -------------------------------------------------------------------
     * 右栏：本月支出分布
     * -------------------------------------------------------------------
     * 同样使用 QFrame + "card" CSS 类名创建白色卡片容器。
     *
     * 内部使用 QVBoxLayout 垂直布局：
     *   - 区域标题（" 本月支出分布"）
     *   - m_categoryBreakdown 容器（动态生成的分类进度条行）
     *   - 弹性空间（addStretch，将内容推到顶部）
     *
     * m_categoryBreakdown 是一个空的 QWidget 容器，
     * 其内部的进度条控件由 refreshCategoryBreakdown() 方法动态创建。
     * 每次刷新时会先清空再重新生成，确保数据与状态同步。
     */
    QFrame *catFrame = new QFrame;
    catFrame->setProperty("class", "card");

    QVBoxLayout *catLayout = new QVBoxLayout(catFrame);

    // 区域小标题
    QLabel *catTitle = new QLabel(" 本月支出分布");
    catTitle->setStyleSheet("font-size: 15px; font-weight: bold; background: transparent; "
                              "padding-bottom: 8px;");
    catLayout->addWidget(catTitle);

    // 创建分类分布容器（内部进度条由 refreshCategoryBreakdown 动态生成）
    m_categoryBreakdown = new QWidget;
    m_categoryBreakdown->setStyleSheet("background: transparent;");

    // 为容器设置一个垂直布局，用于存放分类行
    QVBoxLayout *breakdownLayout = new QVBoxLayout(m_categoryBreakdown);
    breakdownLayout->setContentsMargins(0, 0, 0, 0);  // 取消内边距
    breakdownLayout->setSpacing(6);  // 分类行之间的垂直间距为 6 像素

    catLayout->addWidget(m_categoryBreakdown);

    // 添加弹性空间，将分类进度条推到区域顶部
    // 这样即使分类较少时，空白区域也会在下方而不是均匀分布
    catLayout->addStretch();

    bottomLayout->addWidget(catFrame, 2);  // stretch factor = 2，占 40% 宽度

    // 将底部双栏区域添加到页面主布局中
    // 参数 '1' 表示该区域的 stretch factor（垂直方向）
    // 让底部区域占据标题和卡片行之后的所有剩余垂直空间
    m_mainLayout->addLayout(bottomLayout, 1);
}

/*
 * ===========================================================================
 * 方法：createSummaryCard
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     工厂方法——创建一个带有标准化样式的汇总卡片控件。
 *     卡片的具体外观：
 *       - 白色背景（white）
 *       - 左侧有 4 像素宽的彩色边框（border-left），颜色由参数 color 指定
 *       - 8 像素圆角（border-radius: 8px）
 *       - 16 像素内边距（padding: 16px）
 *       - 内部包含两行文字：
 *         上排：图标 + 标题（灰色小字，13px）
 *         下排：金额数字（大号粗体彩色字，22px）
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     title  - 卡片标题文本，如"本月收入"、"本月支出"、"本月结余"
 *     amount - 金额显示文本，如"+1,234.56"、"-567.89"
 *     color  - 颜色十六进制代码，用于：
 *              (1) 卡片左侧边框颜色
 *              (2) 金额文字颜色
 *              例如："#27AE60"（绿色）、"#E74C3C"（红色）、"#3498DB"（蓝色）
 *     icon   - 图标字符（emoji），如""、、
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     返回 QWidget 指针，指向新创建的汇总卡片控件。
 *     调用者负责将此控件添加到布局中并管理其生命周期。
 *     实际类型为 QFrame，但返回基类指针以提供灵活性。
 * ---------------------------------------------------------------------------
 * 注意事项：
 *     此方法创建的卡片使用 QFrame + 子布局的方式实现。
 *     但在实际的 refreshSummary() 实现中，由于汇总卡片占位符是 QLabel，
 *     最终采用了 HTML 富文本方式在 QLabel 中模拟相同的外观，
 *     与本方法创建的 QFrame 卡片效果等效。
 *     本方法保留用于未来可能的扩展（例如改用 QFrame 方式渲染卡片）。
 * ===========================================================================
 */
QWidget* DashboardPage::createSummaryCard(const QString& title, const QString& amount,
                                           const QString& color, const QString& icon)
{
    /*
     * 创建卡片的外框（QFrame）
     *
     * QFrame 是 Qt 中的一个通用容器控件，常用于创建具有边框、背景色
     * 和圆角的视觉分组区域。这里用它作为汇总卡片的"画框"。
     *
     * setProperty("class", "card")：
     *   设置 CSS 类名为 "card"，与外部样式表关联。
     *   外部样式表可能有类似以下的定义：
     *     QFrame[class="card"] { background: white; border-radius: 8px; }
     *
     * setStyleSheet()：
     *   通过 QString::arg(color) 将颜色参数动态插入到样式表中。
     *   例如，当 color = "#27AE60" 时，生成的样式为：
     *     QFrame {
     *       background: white;
     *       border-left: 4px solid #27AE60;
     *       border-radius: 8px;
     *       padding: 16px;
     *     }
     *   border-left 创建了卡片左侧的彩色"强调条"。
     */
    QFrame *card = new QFrame;
    card->setProperty("class", "card");
    card->setStyleSheet(
        QString("QFrame { background: white; border-left: 4px solid %1; "
                "border-radius: 8px; padding: 16px; }").arg(color));

    /*
     * 创建卡片内部的垂直布局
     * 用于从上到下排列图标+标题 和 金额数字两行文字
     */
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(8);  // 两行文字之间的间距为 8 像素

    /*
     * 第一行：图标 + 标题
     * 例如："  本月收入"
     * 使用较小的灰色字体（13px，颜色 #7F8C8D）
     */
    QLabel *titleLabel = new QLabel(QString("%1  %2").arg(icon, title));
    titleLabel->setStyleSheet("font-size: 13px; color: #7F8C8D; background: transparent;");
    layout->addWidget(titleLabel);

    /*
     * 第二行：金额数字
     * 例如："+1,234.56"
     * 使用大号粗体字体（22px），颜色与左侧边框颜色一致
     * background: transparent 确保标签背景透明，显示卡片背景
     */
    QLabel *amountLabel = new QLabel(amount);
    amountLabel->setStyleSheet(
        QString("font-size: 22px; font-weight: bold; color: %1; background: transparent;")
            .arg(color));
    layout->addWidget(amountLabel);

    // 返回完整的卡片控件
    return card;
}

/*
 * ===========================================================================
 * 方法：refresh
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     刷新概览页面上所有数据区域的显示内容。该方法被 MainWindow 在以下
 *     时机调用：
 *       - 用户切换到此页面时（switchToPage 中）
 *       - 用户添加/修改/删除流水记录后（通过 refreshAll）
 *       - 程序需要强制刷新所有数据时
 *
 *     本方法作为"调度器"，将刷新任务分派给三个子方法：
 *       1. refreshSummary()           —— 刷新汇总卡片
 *       2. refreshRecentRecords() —— 刷新流水表格
 *       3. refreshCategoryBreakdown()  —— 刷新分类分布
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ===========================================================================
 */
void DashboardPage::refresh()
{
    refreshSummary();            // 步骤 1：更新顶部三张汇总卡片的金额数据
    refreshRecentRecords(); // 步骤 2：更新最近流水记录表格
    refreshCategoryBreakdown();  // 步骤 3：更新支出分类分布进度条
}

/*
 * ===========================================================================
 * 方法：refreshSummary
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     刷新顶部三张汇总卡片中显示的财务金额数据。
 *
 *     实现方式：使用 HTML 富文本（RichText）在 QLabel 中渲染卡片外观。
 *     虽然 setupUI 中我们使用 QLabel 作为卡片容器（而非 QFrame），
 *     但通过 HTML 内联样式可以在 QLabel 中实现相同的视觉效果：
 *       - 白色背景（background:white）
 *       - 左侧 4px 彩色边框（border-left:4px solid #颜色）
 *       - 8px 圆角（border-radius:8px）
 *       - 内边距（padding:18px 16px）
 *       - 两行文字：灰色小标题（13px）+ 彩色大号金额（22px bold）
 *
 *     使用 HTML 富文本的好处是可以在不创建额外控件的情况下实现
 *     复杂的图文混排样式，代码更简洁。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 从账本获取最新的收入、支出和结余金额
 *     2. 定义一个 lambda 函数 cardStyle() 生成 HTML 卡片代码
 *     3. 使用 cardStyle() 为三张卡片生成对应的 HTML 内容
 *     4. 将 HTML 内容设置到对应的 QLabel 中
 *     5. 设置 QLabel 的文本格式为 Qt::RichText（富文本模式）
 * ===========================================================================
 */
void DashboardPage::refreshSummary()
{
    /*
     * 步骤 1：从账本获取最新的汇总数据
     * getTotalIncome()  —— 获取所有流水中类型为"收入"的金额总和
     * getTotalExpense() —— 获取所有流水中类型为"支出"的金额总和
     * getBalance()      —— 获取总收入减去总支出的结余金额
     */
    double income  = m_ledger.getTotalIncome();
    double expense = m_ledger.getTotalExpense();
    double balance = m_ledger.getBalance();

    /*
     * 步骤 2：定义生成卡片 HTML 的 lambda 函数
     *
     * cardStyle 是一个 lambda（匿名函数），接收四个参数并返回一个 QString：
     *   - icon：  卡片的图标字符（如""）
     *   - title： 卡片标题（如"本月收入"）
     *   - amount：金额字符串（如"+1,234.56"）
     *   - color： 颜色十六进制值（如"#27AE60"），用于边框和金额颜色
     *
     * 返回值是完整的 HTML 代码字符串，使用 <div> 标签嵌套实现卡片外观：
     *
     *   <div style='...卡片样式...'>
     *     <div style='...标题样式...'>图标  标题</div>
     *     <div style='...金额样式...'>金额</div>
     *   </div>
     *
     * 外层 div：模拟白色卡片背景、彩色左边框、圆角、内边距
     * 内层 div 1：灰色小号字体显示标题
     * 内层 div 2：彩色大号粗体显示金额
     */
    auto cardStyle = [](const QString& icon, const QString& title,
                         const QString& amount, const QString& color) -> QString {
        return QString(
            "<div style='background:white; border-left:4px solid %1; border-radius:8px; "
            "padding:18px 16px; margin:0;'>"
            "<div style='font-size:13px; color:#7F8C8D; margin-bottom:6px;'>%2  %3</div>"
            "<div style='font-size:22px; font-weight:bold; color:%1;'>%4</div>"
            "</div>").arg(color, icon, title, amount);
    };

    /*
     * 步骤 3：为三张卡片生成 HTML 内容
     *
     * 收入卡片（m_cardIncome）：
     *   - 图标： 表示"上升、增长"的意象
     *   - 标题：本月收入
     *   - 金额格式：+xxxx.xx（正数前加"+"号）
     *   - 颜色：#27AE60（绿色）—— 绿色代表收入、增长、积极
     *   - .arg(income, 0, 'f', 2)：将 income 格式化为保留两位小数的字符串
     *     参数说明：0=不限制字段宽度，'f'=浮点数格式，2=保留两位小数
     */
    m_cardIncome->setText(
        cardStyle("", "本月收入", QString("+%1").arg(income, 0, 'f', 2), "#27AE60"));

    /*
     * 支出卡片（m_cardExpense）：
     *   - 图标： 表示"下降、减少"的意象
     *   - 标题：本月支出
     *   - 金额格式：-xxxx.xx（金额前加"-"号表示支出）
     *   - 颜色：#E74C3C（红色）—— 红色代表支出、需要注意
     */
    m_cardExpense->setText(
        cardStyle("", "本月支出", QString("-%1").arg(expense, 0, 'f', 2), "#E74C3C"));

    /*
     * 结余卡片（m_cardBalance）：
     *   - 图标： 表示"钱袋、财富"的意象
     *   - 标题：本月结余
     *   - 金额格式：正数 "+xxxx.xx" 或 负数 "-xxxx.xx"
     *   - 颜色：正数用 #3498DB（蓝色），负数用 #E74C3C（红色）
     *
     *   三元表达式逻辑：
     *     balance >= 0 ?
     *       前加 "+" 号、颜色为蓝色（#3498DB，表示稳健/盈余）
     *       :
     *       前不加号（负数自带 "-" 号）、颜色为红色（#E74C3C，表示赤字/警告）
     *
     *   .arg() 链式调用：
     *     第一个 .arg(balance >= 0 ? "+" : "") → 替换 %1（正负号）
     *     第二个 .arg(balance, 0, 'f', 2)      → 替换 %2（金额数值）
     */
    m_cardBalance->setText(
        cardStyle("", "本月结余",
                  QString("%1%2").arg(balance >= 0 ? "+" : "")
                                   .arg(balance, 0, 'f', 2),
                  balance >= 0 ? "#3498DB" : "#E74C3C"));

    /*
     * 步骤 4：设置文本格式为富文本
     *
     * Qt::RichText 告诉 QLabel 将内容作为 HTML 富文本解析和渲染。
     * 如果不设置此项，QLabel 会将 HTML 标签当作普通文本直接显示
     * （用户会看到 <div> 等标签文字，而不是渲染后的样式效果）。
     */
    m_cardIncome->setTextFormat(Qt::RichText);
    m_cardExpense->setTextFormat(Qt::RichText);
    m_cardBalance->setTextFormat(Qt::RichText);
}

/*
 * ===========================================================================
 * 方法：refreshRecentRecords
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     刷新"最近流水记录"表格中的数据。从账本中读取全部流水记录，
 *     按 ID 降序排列（ID 越大表示记录越新），取最近 10 条显示在表格中。
 *
 *     表格中每行根据记录类型使用不同颜色：
 *       - 收入行：类别和金额显示为绿色（#27AE60）
 *       - 支出行：类别和金额显示为红色（#E74C3C）
 *       - 日期列统一使用灰色（#7F8C8D）
 *       - 备注列统一使用较浅的灰色（#95A5A6）
 *
 *     这种颜色编码让用户可以"扫一眼"就区分收入和支出，提高信息
 *     获取效率。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 从账本获取全部流水记录
 *     2. 确定需要显示的行数（最多 10 行）
 *     3. 设置表格行数
 *     4. 从后往前遍历记录列表（实现降序），填充表格各行
 *     5. 调用 resizeColumnsToContents() 自动调整列宽
 * ===========================================================================
 */
void DashboardPage::refreshRecentRecords()
{
    /*
     * 步骤 1：获取全部流水记录
     * getAllRecords() 返回一个 const 引用到流水记录列表（vector<Record>）。
     * 列表中的记录按添加顺序排列（ID 递增，先添加的记录在前面）。
     */
    const auto& all = m_ledger.getAllRecords();

    /*
     * 步骤 2：计算需要显示的行数
     * std::min(10, (int)all.size()) 取 10 和实际记录数中的较小值。
     * 如果记录总数不足 10 条，有多少显示多少；
     * 如果超过 10 条，只显示最近 10 条。
     */
    int count = std::min(10, (int)all.size());

    // 步骤 3：设置表格的行数
    // 先设置行数，再逐行填充数据
    m_recentTable->setRowCount(count);

    /*
     * 步骤 4：按 ID 降序填充表格（从最新到最旧）
     *
     * 遍历方式：从列表末尾向前遍历
     *   all[all.size() - 1 - i] 的含义：
     *     - all.size() - 1：列表中最后一个元素的索引（最新的记录）
     *     - - i：每循环一次向前移动一位
     *   例如，i=0 时取最后一条（最新），i=1 时取倒数第二条，依此类推。
     *
     * 这种"倒序遍历"避免了创建临时列表或排序的开销，
     * 因为原列表本身就是按时间顺序（旧→新）排列的。
     */
    for (int i = 0; i < count; i++) {
        // 获取当前要显示的这条流水记录的引用
        const auto& t = all[all.size() - 1 - i];

        /*
         * 定义一个局部 lambda 函数 setItem，用于创建并设置表格单元格
         *
         * 参数：
         *   col   —— 列索引（0~4）
         *   text  —— 单元格显示的文本内容
         *   color —— 文本颜色（十六进制字符串），默认 "#2C3E50"（深色）
         *
         * 工作原理：
         *   1. 创建 QTableWidgetItem 对象（Qt 表格的最小数据单元）
         *   2. 设置文本颜色（setForeground）
         *   3. 将单元格放入表格的指定行和列（setItem）
         */
        auto setItem = [&](int col, const QString& text, const QString& color = "#2C3E50") {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setForeground(QColor(color));  // 设置文字颜色
            m_recentTable->setItem(i, col, item); // 将单元格填入表格第 i 行第 col 列
        };

        // 列 0：日期 —— 使用灰色（#7F8C8D）显示，区分于金额等关键信息
        // t.date 是 std::string 类型，需要转换为 QString
        setItem(0, QString::fromStdString(t.date), "#7F8C8D");

        // 列 1：记录类型 —— "收入"（绿色）或 "支出"（红色）
        // 使用 RecordType 枚举判断记录类型
        bool isIncome = (t.type == RecordType::INCOME);
        setItem(1, isIncome ? "收入" : "支出", isIncome ? "#27AE60" : "#E74C3C");

        // 列 2：金额 —— 收入前加"+"号，支出前加"-"号
        // 颜色同样根据类型选择绿色或红色
        // t.amount 是 double 类型，格式化为保留两位小数的字符串
        QString amt = QString(isIncome ? "+%1" : "-%1")
                          .arg(t.amount, 0, 'f', 2);
        setItem(2, amt, isIncome ? "#27AE60" : "#E74C3C");

        // 列 3：分类名称 —— 使用默认深色（#2C3E50）
        // t.category 是 std::string 类型
        setItem(3, QString::fromStdString(t.category));

        // 列 4：备注 —— 使用浅灰色（#95A5A6），降低视觉权重
        // t.note 是 std::string 类型
        setItem(4, QString::fromStdString(t.note), "#95A5A6");
    }

    /*
     * 步骤 5：调整列宽以适配内容
     * resizeColumnsToContents() 会自动将每列的宽度设置为刚好
     * 能容纳该列中最宽单元格的内容。调用此方法后表格看起来更紧凑整齐。
     */
    m_recentTable->resizeColumnsToContents();
}

/*
 * ===========================================================================
 * 方法：refreshCategoryBreakdown
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     刷新"本月支出分布"区域，以彩色进度条的形式展示各个支出分类的
 *     金额占比。进度条直观地展示了"钱花在了哪里"，帮助用户进行消费分析。
 *
 *     实现逻辑：
 *       1. 清空旧的分类分布内容（删除所有子控件和布局）
 *       2. 从账本获取按支出金额降序排列的分类统计
 *       3. 计算总支出金额
 *       4. 如果没有数据或总支出为零，显示"暂无支出数据"的空状态提示
 *       5. 如果有数据，取前 6 个分类，为每个分类创建一行：
 *          分类名称 | 彩色进度条 | 百分比数字
 *
 *     边界情况：
 *       - 无流水数据：显示空状态提示
 *       - 分类数不足 6 个：显示所有分类
 *       - 分类数超过 6 个：只显示前 6 个（占比最大的 6 个分类）
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 清空 m_categoryBreakdown 容器中的旧控件
 *     2. 从账本获取支出分类统计和总支出
 *     3. 判断是否有数据（无数据显示空状态）
 *     4. 确定显示的分类数量（最多 6 个）
 *     5. 为每个分类创建「名称 + 进度条 + 百分比」行
 *     6. 为每个分类分配不同的进度条颜色
 * ===========================================================================
 */
void DashboardPage::refreshCategoryBreakdown()
{
    /*
     * 步骤 1：清空旧的分类分布内容
     *
     * 为什么要清空？
     *   因为每次刷新时，分类的数量和名称可能发生变化（用户可能添加、
     *   删除或修改了分类，或流水记录发生了变化导致某些分类不再有支出）。
     *   如果不清空，旧的进度条会与新的一起显示，造成数据重复和混乱。
     *
     * 清理方法：
     *   1. 通过 qobject_cast 获取 m_categoryBreakdown 的布局管理器
     *      （在 setupUI 中我们为它设置了一个 QVBoxLayout）
     *   2. 循环调用 layout->takeAt(0) 逐个取出布局项
     *      takeAt(0) 每次取出索引 0 处的布局项并返回，同时将其从布局中移除。
     *      因为每次取索引 0，后面的项会前移，所以可以简单地循环直到返回 nullptr。
     *   3. 对每个布局项，先删除其中的 widget（子控件），再删除布局项本身
     */
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_categoryBreakdown->layout());
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        delete child->widget();  // 删除控件（QLabel、QProgressBar 等）
        delete child;            // 删除布局项（QLayoutItem 包装器）
    }

    /*
     * 步骤 2：从账本获取支出分类统计数据
     *
     * getCategorySummaries(RecordType::EXPENSE) 返回：
     *   一个 vector<CategorySummary>，其中每个元素包含：
     *     - category：分类名称（如"餐饮"、"交通"、"购物"）
     *     - amount：该分类的支出总金额
     *     - percentage：该分类占总支出的百分比
     *   列表按支出金额从高到低（降序）排列。
     *
     * getTotalExpense() 返回：所有流水中支出类型的金额总和
     */
    auto stats = m_ledger.getCategorySummaries(RecordType::EXPENSE);
    double totalExpense = m_ledger.getTotalExpense();

    /*
     * 步骤 3：处理空数据状态
     *
     * 当统计列表为空（stats.empty()）或总支出为零或负数时，
     * 说明当前没有任何支出记录，需要向用户展示一个友好的空状态提示。
     *
     * 空状态处理是良好 UI 设计的重要部分——与其显示一个空白的区域，
     * 不如告诉用户"为什么没有数据"或"数据会出现在这里"。
     */
    if (stats.empty() || totalExpense <= 0) {
        // 创建一个居中对齐的提示标签
        QLabel *empty = new QLabel("暂无支出数据");
        empty->setStyleSheet("color: #95A5A6; font-size: 13px; padding: 20px; "
                              "background: transparent;");
        empty->setAlignment(Qt::AlignCenter);  // 文字水平垂直居中
        layout->addWidget(empty);
        return;  // 提前返回，不执行后续的分类显示逻辑
    }

    /*
     * 步骤 4：确定需要显示的分类数量并准备颜色列表
     *
     * show 取 6 和 stats.size() 中的较小值：
     *   - 如果分类数少于 6 个 → 显示全部
     *   - 如果分类数超过 6 个 → 只显示前 6 个（占比最大，最重要的分类）
     *
     * 限制为 6 个是为了保持界面简洁——屏幕上同时显示太多进度条
     * 会让用户感到信息过载，反而不利于快速理解数据。
     *
     * colors 列表包含 6 种预设颜色，每个分类依次分配一种颜色：
     *   #E74C3C —— 红色
     *   #E67E22 —— 橙色
     *   #F39C12 —— 琥珀色/金黄
     *   #3498DB —— 蓝色
     *   #9B59B6 —— 紫色
     *   #1ABC9C —— 青绿色
     *
     * 这些颜色是经过挑选的"调色板"：彼此之间对比度足够高，
     * 在视觉上容易区分，同时色彩明快友好。
     */
    int show = std::min(6, (int)stats.size());
    QStringList colors = {"#E74C3C", "#E67E22", "#F39C12", "#3498DB",
                          "#9B59B6", "#1ABC9C"};

    /*
     * 步骤 5：为每个分类创建一行「名称 + 进度条 + 百分比」
     *
     * 每行的布局结构（HBoxLayout 水平布局）：
     *   ┌────────┬──────────────────────────┬──────┐
     *   │ 分类名 │ ████████████████         │ 45%  │
     *   │ (50px) │ 进度条 (stretch=1)       │(40px)│
     *   └────────┴──────────────────────────┴──────┘
     */
    for (int i = 0; i < show; i++) {
        const auto& cs = stats[i];   // 当前分类的统计信息
        double pct = cs.percentage;  // 该分类占总支出的百分比

        // 创建一个水平布局，作为当前分类的行容器
        QHBoxLayout *row = new QHBoxLayout;
        row->setSpacing(8);  // 分类名、进度条、百分比之间的间距为 8 像素

        /*
         * 子元素 1：分类名称标签
         * 宽度固定为 50 像素，确保所有分类名对齐。
         * 字体大小为 12px，使用默认深色文字。
         * cs.category 是 std::string 类型，需要转换为 QString。
         */
        QLabel *nameLabel = new QLabel(QString::fromStdString(cs.category));
        nameLabel->setFixedWidth(60);
        nameLabel->setStyleSheet("font-size: 12px; background: transparent;");

        /*
         * 子元素 2：简易进度条（QProgressBar）
         *
         * QProgressBar 是 Qt 的标准进度条控件，通常用于显示操作进度，
         * 但在这里被"复用"为数据可视化的柱状图。
         *
         * 设置说明：
         *   setRange(0, 100)：进度条范围设为 0 到 100，对应百分比
         *   setValue((int)pct)：将百分比值（可能带小数）转换为整数设置进度
         *   setTextVisible(false)：隐藏进度条上的文字（百分比数字由独立的
         *                          QLabel 显示，更美观可控）
         *   setFixedHeight(14)：固定高度为 14 像素（较细的条状，节省空间）
         *
         * 进度条样式：
         *   QProgressBar 整体：
         *     - background: #F0F3F7（浅灰蓝色背景，表示"未填充"的部分）
         *     - border: none（无边框）
         *     - border-radius: 7px（圆角，半径 = 高度的一半，实现完全圆角）
         *   QProgressBar::chunk（已填充的部分）：
         *     - background: 当前分类的颜色（如#E74C3C红色）
         *     - border-radius: 7px（与整体圆角一致）
         */
        QProgressBar *bar = new QProgressBar;
        bar->setRange(0, 100);
        bar->setValue((int)pct);
        bar->setTextVisible(false);
        bar->setFixedHeight(14);

        // 从颜色列表中获取当前分类的颜色（循环使用，防止越界）
        QString barColor = colors[i % colors.size()];

        // 通过样式表设置进度条的颜色
        // arg(barColor) 将颜色值插入到 QProgressBar::chunk 的 background 属性中
        bar->setStyleSheet(
            QString("QProgressBar { background: #F0F3F7; border: none; border-radius: 10px; }"
                    "QProgressBar::chunk { background: %1; border-radius: 10px; }")
                .arg(barColor));

        /*
         * 子元素 3：百分比标签
         * 宽度固定为 40 像素，显示格式化的百分比（保留一位小数）。
         * 字体大小为 11px，使用灰色（#7F8C8D）。
         *
         * 格式示例： "45.0%"
         * .arg(pct, 0, 'f', 1)：将 pct 格式化为保留一位小数的字符串
         */
        QLabel *pctLabel = new QLabel(QString("%1%").arg(pct, 0, 'f', 1));
        pctLabel->setFixedWidth(40);
        pctLabel->setStyleSheet("font-size: 11px; color: #7F8C8D; background: transparent;");

        /*
         * 将三个子元素添加到行布局中
         * 进度条使用 stretch factor = 1，表示它会占据分类名和百分比
         * 之间的所有剩余空间。分类名（50px）和百分比（40px）的宽度
         * 是固定的，进度条会自动伸缩适应可用宽度。
         */
        row->addWidget(nameLabel);   // 分类名称（固定 50px 宽）
        row->addWidget(bar, 1);      // 进度条（stretch=1，占据剩余全部空间）
        row->addWidget(pctLabel);    // 百分比数字（固定 40px 宽）

        // 将整行添加到分类分布容器的布局中
        layout->addLayout(row);
    }
}
