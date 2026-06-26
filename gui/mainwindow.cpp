/*
 * ===========================================================================
 * 文件名称：mainwindow.cpp
 * 所属模块：GUI 图形用户界面 - 主窗口
 * 文件编码：UTF-8（兼容 GB2312 中文字符）
 * 功能描述：
 *     本文件实现了应用程序的「主窗口」类 MainWindow。主窗口是整个程序的
 *     最外层容器，负责管理窗口的布局结构、侧边栏导航、页面切换逻辑以及
 *     底部的状态栏信息展示。
 *
 *     主窗口的布局分为三个主要区域：
 *     1. 左侧边栏（Sidebar）—— 包含应用标题、导航按钮和退出按钮
 *     2. 右侧内容区 —— 使用 QStackedWidget 实现多页面堆叠切换
 *        包含四个子页面：概览、账目、统计、分类
 *     3. 底部状态栏（StatusBar）—— 实时显示总收入、总支出和结余
 *
 *     本文件包含以下核心方法的实现：
 *     - setupUI()：构建整个窗口的用户界面布局
 *     - setupSidebar()：创建左侧导航栏的按钮
 *     - setupStatusBar()：创建底部状态栏的标签
 *     - switchToPage()：切换当前显示的页面
 *     - setNavButtonActive()：高亮当前选中的导航按钮
 *     - updateStatusBar()：刷新状态栏中的金额数据
 *     - refreshAll()：一次性刷新所有页面的数据
 *
 * 依赖关系：
 *     本文件依赖于 DashboardPage、FlowPage、StatisticsPage、
 *     CategoryPage 四个页面组件，以及 Ledger 数据管理类。
 * ===========================================================================
 */

#include "mainwindow.h"
#include "homepage.h"
#include "dashboardpage.h"
#include "flowpage.h"
#include "statisticspage.h"
#include "categorypage.h"
#include "otherpage.h"
#include "ledger.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QApplication>

/*
 * ===========================================================================
 * 构造函数：MainWindow
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     初始化主窗口对象。接收外部传入的账本数据和分类管理器引用，
 *     存储为成员变量，然后依次调用 setupUI() 构建界面、
 *     设置默认选中第一个导航按钮、更新状态栏数据。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     ledger  - 账本对象（Ledger）的引用，提供所有流水数据的访问接口。
 *               主窗口不拥有该对象，仅持有引用。
 *     catMan  - 分类管理器（CategoryManager）的引用，提供分类信息的访问。
 *     parent  - Qt 父子对象体系中的父级控件指针，默认为 nullptr，
 *               表示该窗口为顶层窗口。
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 将父指针传递给 QMainWindow 基类构造函数
 *     2. 保存账本和分类管理器的引用到成员变量 m_ledger 和 m_catMan
 *     3. 调用 setupUI() 构建全部界面元素
 *     4. 调用 setNavButtonActive(0) 高亮第一个导航按钮（概览页）
 *     5. 调用 updateStatusBar() 初始化状态栏显示的金额
 * ===========================================================================
 */
MainWindow::MainWindow(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QMainWindow(parent), m_ledger(ledger), m_catMan(catMan)
{
    setupUI();                // 搭建主窗口的全部界面结构
    // 启动时显示首页（索引0），不选中任何导航按钮
    updateStatusBar();        // 初始化底部状态栏的金额显示
}

/*
 * ===========================================================================
 * 析构函数：~MainWindow
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     销毁主窗口对象。Qt 的父子对象机制会自动递归销毁所有子控件
 *     （如侧边栏、页面、状态栏等），因此析构函数体为空。
 *     成员变量 m_ledger 和 m_catMan 为引用类型，不会被析构。
 * ---------------------------------------------------------------------------
 * 注意事项：
 *     由于使用了 Qt 父子对象树，不需要手动 delete 子控件。
 *     当父对象被销毁时，所有子对象也会被自动销毁。
 * ===========================================================================
 */
MainWindow::~MainWindow() {}

/*
 * ===========================================================================
 * 方法：setupUI
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     构建主窗口的完整用户界面布局。这是整个 GUI 界面结构定义的核心
 *     方法，所有可见的界面元素都在此方法中被创建和排列。
 *
 *     整体布局采用「左 - 右」水平分割结构：
 *     ┌─────────────────────────────────────────────────┐
 *     │ ┌──────────┐  ┌──────────────────────────────┐  │
 *     │ │ 侧边栏   │  │  内容区域（QStackedWidget）  │  │
 *     │ │          │  │  ┌────────────────────────┐  │  │
 *     │ │ 标题     │  │  │ 当前显示的页面         │  │  │
 *     │ │ 导航按钮 │  │  │（概览/账目/统计/分类） │  │  │
 *     │ │          │  │  └────────────────────────┘  │  │
 *     │ │ 退出按钮 │  │                              │  │
 *     │ └──────────┘  └──────────────────────────────┘  │
 *     ├─────────────────────────────────────────────────┤
 *     │ 状态栏（总收入 | 总支出 | 结余）                 │
 *     └─────────────────────────────────────────────────┘
 *
 *     左侧边栏宽度固定为 200 像素，右侧内容区占据剩余全部空间。
 *     状态栏位于窗口最底部，由 QMainWindow 的 QStatusBar 管理。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 设置窗口标题和最小/默认尺寸
 *     2. 创建中心控件（centralWidget），作为一切界面元素的父容器
 *     3. 创建水平主布局（QHBoxLayout），将窗口分为左右两部分
 *     4. 构建左侧边栏：应用标题 → 分隔线 → 导航按钮 → 弹性空间 → 退出按钮
 *     5. 构建右侧内容区：QStackedWidget 中依次添加四个功能页面
 *     6. 调用 setupStatusBar() 构建底部状态栏
 * ===========================================================================
 */
void MainWindow::setupUI()
{
    /*
     * 步骤 1：设置窗口标题和尺寸
     * setWindowTitle 定义标题栏显示的文本
     * setMinimumSize 确保窗口不会缩小到布局错乱
     * resize 设置窗口首次显示时的默认大小
     */
    setWindowTitle(" 小工具");
    setMinimumSize(1100, 700);  // 宽度不低于 1100 像素，高度不低于 700 像素
    resize(1200, 750);           // 默认打开时的大小：宽 1200，高 750

    // 步骤 2：创建中心控件
    // QMainWindow 需要一个中心控件（centralWidget）来容纳所有界面内容
    // 所有其他控件都必须放置在这个中心控件之内
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    /*
     * 步骤 3：创建水平主布局
     * QHBoxLayout 将子控件按从左到右的顺序水平排列
     * setContentsMargins(0,0,0,0) 取消布局四周的默认空白边距
     * setSpacing(0) 取消子控件之间的默认间隔
     * 这样做是为了让侧边栏和内容区紧密贴合，没有多余缝隙
     */
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    /*
     * =======================================================================
     * 步骤 4：构建左侧边栏（Sidebar）
     * =======================================================================
     * 侧边栏是一个固定宽度（200 像素）的 QFrame，背景为白色，
     * 右侧有一条浅灰色的分割线用于视觉上与内容区分离。
     *
     * 侧边栏内部从上到下的布局顺序：
     *   ① 应用标题（" 日常记账"）
     *   ② 水平分隔线
     *   ③ 四个导航按钮（概览、账目、统计、分类）
     *   ④ 弹性伸展空间（将退出按钮推到底部）
     *   ⑤ 退出程序按钮
     *
     * 使用 QVBoxLayout 实现垂直方向从上到下的排列。
     */
    QFrame *sidebar = new QFrame;
    sidebar->setFixedWidth(200);  // 固定宽度 200 像素，不允许拉伸或缩小
    sidebar->setStyleSheet(
        "QFrame { background-color: #FFFFFF; border-right: 1px solid #E8ECF1; }");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12, 20, 12, 20);  // 上下 20px 内边距，左右 12px
    sideLayout->setSpacing(6);  // 子控件之间的垂直间距为 6 像素

    // ① 应用标题
    // 使用 QLabel 显示标题，设置较大的加粗字体和深色文字
    QLabel *titleLabel = new QLabel(" 小工具");
    titleLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #2C3E50; "
        "padding: 8px 12px 20px 12px; background: transparent;");
    sideLayout->addWidget(titleLabel);

    // ② 水平分隔线
    // QFrame::HLine 创建一个横向的分割线，用于视觉上将标题与按钮分开
    QFrame *line1 = new QFrame;
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("color: #E8ECF1; background: transparent;");
    sideLayout->addWidget(line1);
    sideLayout->addSpacing(10);  // 分隔线下方留 10 像素的额外空白

    // ③ 调用 setupSidebar() 创建四个导航按钮
    setupSidebar(sideLayout);

    // ④ 弹性空间
    // addStretch() 添加一个可伸缩的空白区域，它会占据所有剩余空间
    // 这样就把退出按钮"推"到了侧边栏的最底部
    sideLayout->addStretch();

    // ⑤ 退出按钮
    // 使用红色边框和文字表示"退出"操作，鼠标悬停时背景变为浅红色
    QPushButton *btnExit = new QPushButton("  退出程序");
    btnExit->setProperty("class", "nav");
    btnExit->setStyleSheet(
        "QPushButton { background: transparent; color: #E74C3C; border: 1px solid #E74C3C; "
        "border-radius: 8px; padding: 12px 16px; text-align: left; font-size: 14px; }"
        "QPushButton:hover { background: #FDEDEC; }");
    sideLayout->addWidget(btnExit);
    // 连接信号-槽：点击退出按钮 → 调用 QApplication::quit() 退出整个程序
    connect(btnExit, &QPushButton::clicked, qApp, &QApplication::quit);

    // 将侧边栏添加到主布局的最左侧
    mainLayout->addWidget(sidebar);

    /*
     * =======================================================================
     * 步骤 5：构建右侧内容区域（使用 QStackedWidget）
     * =======================================================================
     * QStackedWidget 是一个「堆叠页面容器」，它同时持有多个子页面（Widget），
     * 但在同一时刻只显示其中一个。通过 setCurrentIndex(int) 方法可以
     * 切换当前显示的页面，索引从 0 开始。
     *
     * 这种设计类似于"标签页"（Tab）的概念，但切换由我们自己编写导航按钮
     * 来控制，而不是使用 Qt 内置的 QTabWidget。
     *
     * 四个页面按如下顺序添加：
     *   索引 0 → 概览页（DashboardPage）
     *   索引 1 → 账目页（FlowPage）
     *   索引 2 → 统计页（StatisticsPage）
     *   索引 3 → 分类页（CategoryPage）
     */
    m_stackedWidget = new QStackedWidget;
    m_stackedWidget->setStyleSheet("background-color: #F5F7FA;");  // 浅灰蓝色背景

    // 创建首页和五个功能页面实例
    m_homePage         = new HomePage;
    m_dashboardPage    = new DashboardPage(m_ledger, m_catMan);
    m_flowPage  = new FlowPage(m_ledger, m_catMan);
    m_statisticsPage   = new StatisticsPage(m_ledger, m_catMan);
    m_categoryPage     = new CategoryPage(m_ledger, m_catMan);
    m_otherPage        = new OtherPage(m_ledger);

    // 将页面按固定顺序添加到堆叠容器中
    // 索引 0：启动首页 —— 显示"记账本"欢迎页
    m_stackedWidget->addWidget(m_homePage);           // 0
    // 索引 1：概览页 —— 显示汇总卡片、最近流水、分类分布
    m_stackedWidget->addWidget(m_dashboardPage);      // 1
    // 索引 2：账目页 —— 显示流水列表、添加/编辑流水
    m_stackedWidget->addWidget(m_flowPage);           // 2
    // 索引 3：统计页 —— 显示图表和统计分析
    m_stackedWidget->addWidget(m_statisticsPage);     // 3
    // 索引 4：分类页 —— 管理收入和支出的分类
    m_stackedWidget->addWidget(m_categoryPage);       // 4
    // 索引 5：其他功能页 —— 数据导出等辅助功能
    m_stackedWidget->addWidget(m_otherPage);          // 5

    // 将堆叠容器添加到主布局的右侧，参数 '1' 表示 stretch factor
    // stretch factor 设为 1 意味着内容区会占据所有剩余的水平空间
    mainLayout->addWidget(m_stackedWidget, 1);

    // 步骤 6：创建底部状态栏
    setupStatusBar();
}

/*
 * ===========================================================================
 * 方法：setupSidebar
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     在侧边栏中创建导航按钮。通过一个循环遍历预定义的导航项列表，
 *     为每个导航项创建一个 QPushButton，并将其与对应的页面索引关联。
 *     当用户点击某个导航按钮时，会触发页面切换。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     sideLayout - 侧边栏的垂直布局（QVBoxLayout）指针。
 *                  新创建的按钮将被添加到这个布局中。
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 核心技术点：
 *     1. 信号-槽连接（Signal-Slot）：
 *        每个按钮的 clicked 信号连接到一个 lambda 表达式（匿名函数）。
 *        lambda 捕获了 pageIndex 变量，当按钮被点击时，
 *        调用 switchToPage(pageIndex) 切换到对应页面。
 *
 *     2. Lambda 捕获：
 *        [this, pageIndex] 表示 lambda 函数可以访问当前类（MainWindow）
 *        的成员，并且按值捕获 pageIndex 变量。
 *        使用按值捕获是因为 pageIndex 是在循环中定义的局部变量，
 *        如果按引用捕获，循环结束后所有按钮都会引用同一个变量
 *        （其值为最后一次循环的值），造成行为错误。
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 定义导航项结构体 NavItem，包含按钮显示的文本和图标
 *     2. 创建一个包含四个导航项（概览、账目、统计、分类）的列表
 *     3. 在循环中为每个导航项：
 *        - 创建一个 QPushButton 并设置显示文本
 *        - 设置鼠标悬停时变为手型光标
 *        - 将按钮添加到侧边栏布局
 *        - 将按钮指针保存到 m_navButtons 向量中（用于后续高亮管理）
 *        - 连接 clicked 信号到 lambda，实现页面切换
 * ===========================================================================
 */
void MainWindow::setupSidebar(QVBoxLayout *sideLayout)
{
    // ===================================================================
    // 一级按钮：「日常记账」展开/折叠
    // ===================================================================
    m_navGroupBtn = new QPushButton("▼ 日常记账");
    m_navGroupBtn->setCursor(Qt::PointingHandCursor);
    m_navGroupBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #2C3E50; border: none; "
        "border-radius: 8px; padding: 12px 16px; text-align: left; "
        "font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #E8EDF2; }");
    connect(m_navGroupBtn, &QPushButton::clicked, this, &MainWindow::toggleNavGroup);
    sideLayout->addWidget(m_navGroupBtn);

    // ===================================================================
    // 二级导航容器（初始展开）
    // ===================================================================
    m_navGroupContainer = new QWidget;
    m_navGroupContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *groupLayout = new QVBoxLayout(m_navGroupContainer);
    groupLayout->setContentsMargins(12, 0, 0, 0);  // 左侧缩进12px表示层级
    groupLayout->setSpacing(2);

    struct NavItem {
        QString text;
        QString icon;
    };

    std::vector<NavItem> navs = {
        {"  概览", ""},   // → 切换到索引 1（DashboardPage）
        {"  账目", ""},   // → 切换到索引 2（FlowPage）
        {"  统计", ""},   // → 切换到索引 3（StatisticsPage）
        {"  分类", ""},   // → 切换到索引 4（CategoryPage）
        {"  其他", ""}    // → 切换到索引 5（OtherPage）
    };

    for (size_t i = 0; i < navs.size(); i++) {
        QPushButton *btn = new QPushButton(navs[i].text);
        btn->setProperty("class", "nav");
        btn->setCursor(Qt::PointingHandCursor);
        groupLayout->addWidget(btn);  // 添加到容器布局而非侧边栏
        m_navButtons.push_back(btn);

        int pageIndex = static_cast<int>(i) + 1;  // +1 因为索引 0 是入场首页
        connect(btn, &QPushButton::clicked, this, [this, pageIndex]() {
            switchToPage(pageIndex);
        });
    }

    sideLayout->addWidget(m_navGroupContainer);
}

/*
 * ===========================================================================
 * 方法：toggleNavGroup
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     切换"日常记账"导航分组的展开/折叠状态。
 *     折叠时隐藏二级导航按钮，展开时显示。
 * ---------------------------------------------------------------------------
 */
void MainWindow::toggleNavGroup()
{
    bool visible = m_navGroupContainer->isVisible();
    m_navGroupContainer->setVisible(!visible);
    // 更新箭头指示符
    m_navGroupBtn->setText(visible ? "▶ 日常记账" : "▼ 日常记账");
}

/*
 * ===========================================================================
 * 方法：setupStatusBar
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     初始化主窗口的底部状态栏。状态栏由 QMainWindow 内置的 QStatusBar
 *     管理。本方法创建三个 QLabel 标签，分别用于显示：
 *       - 总收入（m_statusIncome）
 *       - 总支出（m_statusExpense）
 *       - 结余（m_statusBalance）
 *
 *     状态栏的样式设置了白色背景、顶部边框线以及文字字体大小。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 关于状态栏布局：
 *     - addWidget() 将标签添加到状态栏的左侧区域，按添加顺序从左到右排列
 *     - addPermanentWidget() 将标签添加到状态栏的右侧区域（永久区域），
 *       通常用于显示需要始终可见的信息
 *     - 在本应用中，结余标签被放在右侧永久区域，使其在视觉上更突出
 * ===========================================================================
 */
void MainWindow::setupStatusBar()
{
    // 获取 QMainWindow 内置的状态栏对象
    QStatusBar *sb = statusBar();

    /*
     * 设置状态栏的 CSS 样式
     * QStatusBar 整体：白色背景，顶部有浅灰色分割线（1px），上下内边距 6px，左右 16px
     * QStatusBar::item：移除子项（标签）的边框
     */
    sb->setStyleSheet(
        "QStatusBar { background: #FFFFFF; border-top: 1px solid #E8ECF1; "
        "padding: 6px 16px; font-size: 13px; }"
        "QStatusBar::item { border: none; }");

    // 创建三个状态标签
    // m_statusIncome  —— 显示总收入金额
    // m_statusExpense —— 显示总支出金额
    // m_statusBalance —— 显示结余金额（收入 - 支出）
    m_statusIncome  = new QLabel;
    m_statusExpense = new QLabel;
    m_statusBalance = new QLabel;

    // 将收入和支出标签添加到状态栏的左侧区域（按添加顺序排列）
    sb->addWidget(m_statusIncome);
    sb->addWidget(m_statusExpense);

    // 将结余标签添加到状态栏的右侧永久区域
    // addPermanentWidget 使得该标签始终显示在状态栏的最右侧
    sb->addPermanentWidget(m_statusBalance);
}

/*
 * ===========================================================================
 * 方法：switchToPage
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     将当前显示的内容切换到指定的页面。该方法同时执行两个操作：
 *       1. 更新侧边栏导航按钮的高亮状态（当前页面对应的按钮高亮显示）
 *       2. 切换 QStackedWidget 当前显示的页面
 *       3. 调用目标页面的 refresh() 方法刷新数据
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     index - 目标页面的索引（整数，取值范围 0~3）
 *             0 = 概览页（DashboardPage）
 *             1 = 账目页（FlowPage）
 *             2 = 统计页（StatisticsPage）
 *             3 = 分类页（CategoryPage）
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 调用 setNavButtonActive(index) 更新按钮高亮
 *     2. 调用 QStackedWidget::setCurrentIndex(index) 切换显示的页面
 *     3. 根据索引调用对应页面的 refresh() 方法刷新数据
 *        （确保切换页面时显示的是最新的数据）
 * ===========================================================================
 */
void MainWindow::switchToPage(int index)
{
    // 步骤 1：高亮侧边栏中对应索引的导航按钮
    // 页面索引 1~5 对应导航按钮索引 0~4（索引0=首页无对应按钮）
    setNavButtonActive(index - 1);

    // 步骤 2：切换 QStackedWidget 当前显示的页面
    // QStackedWidget 会自动隐藏之前的页面，显示索引为 index 的页面
    m_stackedWidget->setCurrentIndex(index);

    // 步骤 3：根据页面索引刷新对应页面的数据
    // 使用 switch 语句确保每个分支都处理正确的页面
    switch (index) {
    case 0: /* 启动首页 —— 无数据需刷新 */   break;
    case 1: m_dashboardPage->refresh();   break;  // 刷新概览页
    case 2: m_flowPage->refresh();        break;  // 刷新账目页
    case 3: m_statisticsPage->refresh();  break;  // 刷新统计页
    case 4: m_categoryPage->refresh();    break;  // 刷新分类页
    case 5: m_otherPage->refresh();       break;  // 刷新其他功能页
    }
}

/*
 * ===========================================================================
 * 方法：setNavButtonActive
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     设置侧边栏导航按钮的「活跃状态」。只有当前选中页面对应的按钮
 *     被标记为活跃（active="true"），其余按钮标记为非活跃（active="false"）。
 *     通过 CSS 属性选择器 [active="true"] 配合外部样式表，可以定义
 *     不同的视觉样式（如高亮背景、加粗字体等）。
 *
 *     由于 Qt 的样式表不会自动在属性变化时重新应用，因此在修改属性后
 *     需要手动调用 unpolish() 和 polish() 强制刷新样式。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     index - 需要设为活跃的按钮索引（0~3），其余按钮将被设为非活跃
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 技术细节：
 *     Qt 样式表的属性选择器：
 *       QPushButton[active="true"]  { 活跃样式 }
 *       QPushButton[active="false"] { 默认样式 }
 *
 *     unpolish() + polish() 的作用：
 *       Qt 的样式机制会在控件首次显示时计算样式。修改动态属性
 *       （如 setProperty 修改的 custom property）后，样式不会自动
 *       更新。需要先调用 style()->unpolish() 移除缓存的样式，然后
 *       调用 style()->polish() 重新应用样式。
 * ===========================================================================
 */
void MainWindow::setNavButtonActive(int index)
{
    // 遍历所有导航按钮
    for (int i = 0; i < (int)m_navButtons.size(); i++) {
        // 将当前按钮的 active 属性设置为 "true" 或 "false"
        // 如果按钮索引等于目标索引 → "true"（活跃状态，高亮显示）
        // 否则 → "false"（非活跃状态，默认样式）
        m_navButtons[i]->setProperty("active", i == index ? "true" : "false");

        /*
         * 强制刷新按钮样式
         * 1. unpolish() —— 清除当前按钮的缓存样式（将按钮恢复到未应用样式表的状态）
         * 2. polish()   —— 重新应用样式表（Qt 会重新计算并应用所有匹配的样式规则）
         * 这一步是必需的，因为 setProperty 修改的是动态属性，
         * Qt 不会自动重新评估样式表。
         */
        m_navButtons[i]->style()->unpolish(m_navButtons[i]);
        m_navButtons[i]->style()->polish(m_navButtons[i]);
    }
}

/*
 * ===========================================================================
 * 方法：updateStatusBar
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     刷新底部状态栏中显示的三个金额数据：总收入、总支出、结余。
 *     从账本（Ledger）对象中获取最新的金额数据，然后使用 HTML 富文本
 *     格式化更新状态栏标签的显示内容。
 *
 *     状态栏使用 HTML 富文本（Qt::RichText 通过 setText 自动识别 HTML）
 *     来实现：
 *       - 不同颜色区分收入（绿色）和支出（红色）
 *       - 结余根据正负自动选择绿色或红色
 *       - 粗体强调金额数字
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 技术细节：
 *     HTML 富文本在 QLabel 中的使用：
 *       - 使用 <span style='...'> 内联样式标签设置文字颜色、粗细等
 *       - color 属性接受十六进制颜色值（如 #27AE60 为绿色）
 *       - font-weight:bold 使文字加粗
 *       - QString::arg() 用于格式化数字为保留两位小数的字符串
 *
 *     颜色编码规则：
 *       - #27AE60（绿色）：用于收入和正结余，表示"正向"财务状态
 *       - #E74C3C（红色）：用于支出和负结余，表示"支出"或"亏损"状态
 *       - #3498DB（蓝色）：本文件中用于正结余（也可以使用绿色保持一致）
 * ===========================================================================
 */
void MainWindow::updateStatusBar()
{
    /*
     * 从账本对象获取最新的财务汇总数据
     * getTotalIncome()  —— 返回所有流水中类型为"收入"的金额总和
     * getTotalExpense() —— 返回所有流水中类型为"支出"的金额总和
     * getBalance()      —— 返回总收入减去总支出的结果（结余）
     */
    double income  = m_ledger.getTotalIncome();
    double expense = m_ledger.getTotalExpense();
    double balance = m_ledger.getBalance();

    /*
     * 设置总收入标签的 HTML 内容
     * 格式示例：  "总收入  +1,234.56"
     * 颜色：#27AE60（绿色）—— 收入使用绿色表示
     * .arg(income, 0, 'f', 2) 表示：
     *   - 参数 0：字段宽度（0 表示不限制，自动调整）
     *   - 参数 'f'：浮点数格式
     *   - 参数 2：保留两位小数
     */
    m_statusIncome->setText(
        QString("  总收入  <span style='color:#27AE60;font-weight:bold;'>+%1</span>   ")
            .arg(income, 0, 'f', 2));

    /*
     * 设置总支出标签的 HTML 内容
     * 格式示例：  "总支出  -567.89"
     * 颜色：#E74C3C（红色）—— 支出使用红色表示，前面加负号
     */
    m_statusExpense->setText(
        QString("总支出  <span style='color:#E74C3C;font-weight:bold;'>-%1</span>   ")
            .arg(expense, 0, 'f', 2));

    /*
     * 设置结余标签的 HTML 内容
     * 格式示例：  "结余  +666.67"（正结余，绿色）或 "结余  -100.00"（负结余，红色）
     *
     * 使用三元表达式实现条件逻辑：
     *   balance >= 0 时：
     *     - 颜色使用 "#27AE60"（绿色）
     *     - 金额前加 "+" 号
     *   balance < 0 时：
     *     - 颜色使用 "#E74C3C"（红色）
     *     - 金额前不加号（负数自带 "-" 号）
     *
     * 注意：.arg() 调用顺序很重要，%1 对应第一个 arg，%2 对应第二个，%3 对应第三个
     */
    m_statusBalance->setText(
        QString("结余  <span style='color:%1;font-weight:bold;'>%2%3</span>   ")
            .arg(balance >= 0 ? "#27AE60" : "#E74C3C")  // %1 — 根据正负选择颜色
            .arg(balance >= 0 ? "+" : "")                // %2 — 正数时加 "+" 号
            .arg(balance, 0, 'f', 2));                   // %3 — 结余金额（保留两位小数）
}

/*
 * ===========================================================================
 * 方法：refreshAll
 * ---------------------------------------------------------------------------
 * 功能描述：
 *     一次性刷新所有页面的数据并更新状态栏。通常用于以下场景：
 *       - 用户添加、修改或删除了流水记录后
 *       - 用户修改了分类设置后
 *       - 需要确保整个界面显示的是最新数据时
 *
 *     该方法依次调用四个页面的 refresh() 方法，然后更新状态栏。
 *     每个页面的 refresh() 方法会从账本中重新读取数据并更新显示。
 * ---------------------------------------------------------------------------
 * 参数说明：
 *     无参数
 * ---------------------------------------------------------------------------
 * 返回结果：
 *     无返回值（void）
 * ---------------------------------------------------------------------------
 * 执行步骤：
 *     1. 刷新概览页 —— 更新汇总卡片、最近流水列表、分类分布
 *     2. 刷新账目页 —— 更新流水记录表格
 *     3. 刷新统计页 —— 更新统计图表
 *     4. 刷新分类页 —— 更新分类列表
 *     5. 更新状态栏 —— 更新总收入、总支出、结余的显示
 * ===========================================================================
 */
void MainWindow::refreshAll()
{
    m_dashboardPage->refresh();
    m_flowPage->refresh();
    m_statisticsPage->refresh();
    m_categoryPage->refresh();
    m_otherPage->refresh();
    updateStatusBar();
}
