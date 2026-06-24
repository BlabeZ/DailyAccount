/*
 * ============================================================================
 * 文件名: statisticspage.cpp
 * 模块:   统计页面（GUI实现文件）
 * 功能:   实现三个自绘图表控件（饼图、柱状图）及统计页面的数据加载逻辑。
 *         本文件不依赖任何第三方图表库，所有可视化均由 Qt 的 QPainter
 *         （2D绘图引擎）完成。
 * 编码:   UTF-8
 *
 * 架构概览:
 *   StatisticsPage（统计页容器）
 *   ├── QHBoxLayout: 标题行[QLabel + QComboBox(时间范围)]
 *   ├── QHBoxLayout: 汇总数字[4个QLabel × HTML富文本]
 *   ├── QHBoxLayout: 图表行
 *   │   ├── QFrame(卡片1): PieChartWidget — 支出分类分布
 *   │   └── QFrame(卡片2): PieChartWidget — 收入分类分布
 *   └── QFrame(卡片3): BarChartWidget — 月度收支趋势
 * ============================================================================
 */

#include "statisticspage.h"
#include "ledger.h"
#include "category.h"
#include <QHBoxLayout>   // 水平布局 —— 将子控件从左到右排列
#include <QGridLayout>   // 网格布局 —— 按行列组织子控件（本文件未直接使用但保留以备扩展）
#include <QFrame>        // 框架控件 —— 用作卡片容器的外观边框
#include <QPainter>      // Qt 2D绘图核心类 —— 所有画笔、画刷、几何绘制均通过它完成
#include <QPainterPath>  // 绘图路径 —— 用于定义复杂形状（圆角矩形等）
#include <QDate>         // 日期类 —— 用于计算时间范围的起止日期
#include <cmath>         // C++标准数学库 —— std::min, std::max等
#include <algorithm>     // C++标准算法库 —— std::sort用于对分类数据排序

// ==================== PieChartWidget（饼图/甜甜圈图控件）====================

/*
 * 构造函数：初始化饼图控件的基本属性
 * 1. 设置最小尺寸为 280×250 像素 —— 保证即使在紧凑布局中也有足够的绘制区域
 *    （饼图的直径由控件的实际宽高取较小值决定，280x250确保至少能画直径250的圆）
 * 2. 设置样式表 background: transparent —— 让控件背景透明，
 *    使得外层卡片容器（QFrame）的背景色可以透过此控件显示，视觉上融为一体
 */
PieChartWidget::PieChartWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(280, 250);
    setStyleSheet("background: transparent;");
}

/*
 * setData — 向饼图控件注入数据并请求重绘
 *
 * 工作流程：
 *   第1步: 保存数据指针到成员变量 m_data（分类名称+金额的列表）
 *   第2步: 保存颜色列表到 m_colors（扇区颜色按索引循环使用）
 *   第3步: 清空 m_total 并遍历所有数据条目，累加金额得到总计
 *          （m_total 是计算每个扇区所占弧度的分母）
 *   第4步: 调用 QWidget::update() —— 此函数会向Qt事件队列投递一个
 *          QPaintEvent 请求。Qt在下一轮事件循环中调用 paintEvent()。
 *          注意：update() 是异步的，不会立即绘制；repaint() 则同步绘制。
 *
 * 参数 data:   数据列表 —— 每个元素是一对 <分类名称, 金额>
 * 参数 colors: 颜色列表 —— 如 "#E74C3C", "#3498DB", ...
 */
void PieChartWidget::setData(const std::vector<std::pair<QString, double>>& data,
                              const QStringList& colors) {
    m_data = data;
    m_colors = colors;
    m_total = 0;
    // 遍历所有数据项，累加得到总金额 —— 这是饼图"100%"的基准
    for (const auto& [name, val] : data) m_total += val;
    // 请求Qt重绘此控件 —— 下一步Qt会调用 paintEvent()
    update();
}

/*
 * paintEvent — 饼图的核心绘制函数（重写自 QWidget）
 *
 * ======================== QPainter 基础知识 ========================
 * QPainter 是Qt的2D绘图引擎。可以把它想象成一支"智能画笔"——它知道
 * 如何绘制各种几何图元（圆弧、矩形、椭圆、文本等），并对这些图元应用
 * 笔刷（QBrush，填充颜色/渐变）和笔（QPen，描边颜色/粗细/样式）。
 *
 * 每次调用 paintEvent 时，Qt框架已经完成了：
 *   1. 清空控件的绘图区域
 *   2. 准备一个与控件关联的 QPainter 对象
 *   3. 当 QPainter 对象构造完毕并关联到 this（即本控件），所有后续
 *      绘制操作均被渲染到此控件的表面上
 *
 * 抗锯齿（Antialiasing）：通过 setRenderHint(QPainter::Antialiasing) 启用。
 * 它让弧线边缘变得平滑（通过子像素混合），避免出现"锯齿"状的阶梯边缘。
 *
 * ==================== 甜甜圈饼图的绘制原理 ====================
 * 本函数绘制的是"甜甜圈"式饼图（donut chart）——顾名思义，中心是空的，
 * 像一个圆环被切分成扇区。实现方法：
 *   1. 先画完整饼图（多个扇区围绕圆心360度排列）
 *   2. 再在中心叠加一个与背景同色的白色圆形 —— 视觉上"挖空"了中心，
 *      形成了甜甜圈效果
 *
 * ==================== 弧（Arc）的角度系统 ====================
 * Qt的 drawPie 使用**1/16度**作为角度单位（这是历史原因，继承自早期
 * 图形系统的整数坐标表示法）。例如：
 *    360度  = 360 × 16 = 5760 单位
 *    90度   = 90  × 16 = 1440 单位
 *    1度    = 16 单位
 * 这样做的目的是让角度计算使用整数（int）即可精确表示，避免浮点误差。
 *
 * 起始角度: 90° × 16 = 1440 单位 —— 对应圆形的**12点钟方向（正上方）**。
 * 在Qt坐标系中，0° 是3点钟方向（正右方），角度沿顺时针增加，
 * 因此 90° 是6点钟方向… 但经过 arc 的特殊旋转后，90×16 实际对准12点。
 *
 * ==================== 绘制步骤详解 ====================
 *
 * 第0步（预处理）: 检查是否有数据。如果无数据或总金额 <= 0（全是零或负数），
 *                 绘制居中的灰色提示文字"暂无数据"，然后返回。
 *
 * 第1步: 创建 QPainter 对象 p(this)，将其绑定到当前控件。
 *        开启抗锯齿渲染，使饼图弧线边缘平滑。
 *
 * 第2步: 计算饼图的位置和尺寸。
 *        - side: 取控件宽度和高度的较小值，减去20像素边距 —— 这是饼图的直径
 *        - cx, cy: 控件的几何中心 —— 饼图的圆心
 *        - pieRect: 饼图的外接矩形 —— QRectF 是浮点精度矩形，
 *          左上角为 (cx - side/2, cy - side/2)，宽高均为 side
 *
 * 第3步: 遍历每个数据扇区，绘制对应的弧。
 *        - angle 变量从 90°×16（12点方向）开始，每画完一个扇区后
 *          加上该扇区的扫掠角度（sweep），实现角度**累积**。
 *          这是弧段连续拼接的关键——每个扇区紧接上一个扇区的结束位置。
 *        - sweep（扇区扫掠角）= (该扇区金额 / 总金额) × 360° × 16
 *          例如：某分类占总额的25%，则 sweep = 0.25 × 5760 = 1440 单位
 *        - 颜色分配: 按索引 i 对颜色列表长度取模 (i % count)，
 *          这样当数据项超过颜色数时，颜色会循环复用
 *        - setBrush 设置填充色，setPen 设置描边色（白色描边让扇区之间有
 *          清晰的分隔线，视觉上更易区分）
 *        - drawPie 绘制一个扇形（从 startAngle 开始，扫过 sweepAngle 角度）
 *
 * 第4步: 绘制中心白色圆形（实现甜甜圈效果）。
 *        - innerR = side / 5 —— 内圆半径为饼图直径的 1/5
 *        - innerRect: 以 (cx, cy) 为中心，边长为 innerR*2 的正方形区域
 *        - 用与页面背景色相同的 #F5F7FA 填充，无描边 ——
 *          这个白色圆"覆盖"在饼图中心之上，视觉上掏空了饼图
 */
void PieChartWidget::paintEvent(QPaintEvent *) {
    // -------- 特殊情况：无数据时显示提示文字 --------
    // 条件：数据为空 或 总金额非正（全为零或负数的无效状态）
    if (m_data.empty() || m_total <= 0) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);   // 启用抗锯齿，文字也会更平滑
        p.setPen(QColor("#BDC3C7"));                // 浅灰色文字
        p.setFont(QFont("Microsoft YaHei", 11));    // 微软雅黑 11pt
        // drawText: 在rect区域内居中绘制文本，Qt::AlignCenter做水平和垂直居中
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无数据"));
        return;
    }

    // -------- 创建QPainter并启用抗锯齿 --------
    QPainter p(this);                                // 创建绘图对象，绑定到本控件
    p.setRenderHint(QPainter::Antialiasing);         // 开启抗锯齿 —— 圆滑弧线边缘

    // -------- 计算饼图的几何参数 --------
    // side: 饼图直径 = min(宽, 高) - 20（左右各留10像素边距）
    int side = (int)(std::min(width(), height()) * 0.7) - 10;
    // cx, cy: 控件的中心坐标 —— 饼图的圆心位置
    int cx = side / 2 + 20;
    int cy = height() / 2;
    // pieRect: 饼图的外接矩形（QRectF使用浮点数，精度更高）
    // 左上角坐标 = (cx - side/2, cy - side/2)，宽 = 高 = side
    QRectF pieRect(cx - side / 2, cy - side / 2, side, side);

    // -------- 绘制饼图各扇区（角度累加循环）--------
    // angle: 当前扇区的起始角度，单位是 1/16 度
    // 初始值 90.0 * 16 = 1440，对应12点钟方向（正上方）
    // 使用double类型保证中间计算的高精度，传给drawPie时转为int
    double angle = 90.0 * 16; // 从12点方向开始

    // 循环遍历每一个数据项，画出对应的扇区
    for (size_t i = 0; i < m_data.size(); i++) {
        // ---- 计算当前扇区的扫掠角度 ----
        // 公式: (该分类金额 / 总金额) × 360° × 16
        // 例如: 总金额1000，当前分类250，则 sweep = 0.25 × 5760 = 1440
        double sweep = (m_data[i].second / m_total) * 360.0 * 16;

        // ---- 选择扇区颜色 ----
        // 按索引对颜色列表长度取模 —— 确保数据项超过颜色数时循环使用
        QColor color(m_colors[i % m_colors.size()]);
        p.setBrush(color);         // 设置填充画笔（扇区内部颜色）
        p.setPen(Qt::white);       // 设置描边画笔（扇区间隔线为白色）

        // ---- 绘制扇形 ----
        // drawPie(外接矩形, 起始角度(int), 扫掠角度(int))
        // 注意: angle和sweep从double转为int，截断误差可忽略
        p.drawPie(pieRect, (int)angle, (int)sweep);

        // ---- 角度累加 —— 下一个扇区从当前扇区的结束位置开始 ----
        angle += sweep;
    }

    // -------- 绘制甜甜圈中心孔（内圆覆盖） --------
    // 在饼图中心叠加一个与背景同色的白色圆，形成"甜甜圈"效果
    // innerR: 内圆半径 = 饼图直径的 1/5（约40~60像素）
    int innerR = side / 5;
    // innerRect: 内圆的外接正方形，以(cx, cy)为中心
    QRectF innerRect(cx - innerR, cy - innerR, innerR * 2, innerR * 2);
    p.setBrush(QColor("#F5F7FA"));   // 用页面背景色填充内圆
    p.setPen(Qt::NoPen);             // NoPen表示不描边 —— 内圆边缘不可见
    p.drawEllipse(innerRect);

    // 右侧图例：色块 + 分类名称 + 百分比
    int legendX = cx + side / 2 + 16;
    int legendY = cy - (int)m_data.size() * 13;
    p.setFont(QFont("Microsoft YaHei", 9));
    for (size_t i = 0; i < m_data.size(); i++) {
        int y = legendY + (int)i * 22;
        QColor color(m_colors[i % m_colors.size()]);
        p.setBrush(color);
        p.setPen(Qt::NoPen);
        p.drawRect(legendX, y, 12, 12);
        p.setPen(QColor("#2C3E50"));
        double pct = m_data[i].second / m_total * 100.0;
        p.drawText(legendX + 18, y + 11,
                   QString("%1 (%2%)").arg(m_data[i].first).arg(pct, 0, 'f', 1));
    }        // 绘制内圆（椭圆在正方形区域 = 正圆）
    // 此时内圆"覆盖"在饼图扇区的中心部分之上，形成从外向内看——
    // 各扇区的颜色占据外环，中心是一块白色圆形区域（甜甜圈孔）
}

// ==================== BarChartWidget（柱状图控件）====================

/*
 * 构造函数：初始化柱状图控件
 * 最小尺寸设为 400×250 —— 宽度需要容纳多个月的柱子对，
 * 高度保证柱状区域不至于太矮而丧失可读性
 */
BarChartWidget::BarChartWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(240, 150);
    setStyleSheet("background: transparent;");
}

/*
 * setData — 向柱状图控件注入数据
 *
 * 参数 data:          数据列表 —— 月份和金额交替存储
 *                     格式: [("2025-01", 收入), ("2025-01", 支出),
 *                           ("2025-02", 收入), ("2025-02", 支出), ...]
 *                     即每个月份对应连续两个元素：收入在前，支出在后
 * 参数 positiveLabel: 图例中的正项名称（如"收入"）
 * 参数 negativeLabel: 图例中的负项名称（如"支出"）
 *
 * 处理逻辑:
 *   1. 保存数据、图例标签
 *   2. 遍历数据找出最大金额 —— 用于确定Y轴上限
 *   3. 将最大值乘以 1.15 —— 在最高的柱子顶部留出约柱高15%的空白，
 *      避免柱顶触碰到图表顶部边缘，也为数值标签留出空间
 *   4. 防御性检查: 如果最大值仍 <= 0，设为1（避免除零等异常）
 *   5. 调用 update() 触发重绘
 */
void BarChartWidget::setData(const std::vector<std::pair<QString, double>>& data,
                              const QString& positiveLabel, const QString& negativeLabel) {
    m_data = data;
    m_posLabel = positiveLabel;
    m_negLabel = negativeLabel;

    // 查找数据中的最大金额 —— 作为Y轴的刻度上限
    m_maxVal = 0;
    for (const auto& [name, val] : data) {
        if (val > m_maxVal) m_maxVal = val;
    }
    // 最大值扩大1.15倍，确保最高柱子上方有适当的空白空间
    m_maxVal *= 1.15; // 留出顶部空间

    // 防御: 如果所有数据为0或空，保证 m_maxVal 至少为1，避免除零错误
    if (m_maxVal <= 0) m_maxVal = 1;

    update();
}

/*
 * paintEvent — 柱状图的核心绘制函数
 *
 * ==================== 坐标系统详解 ====================
 * Qt/计算机图形学的坐标系与数学坐标系不同：
 *   - 原点 (0,0) 在**左上角**
 *   - X轴：向右为正
 *   - Y轴：向**下**为正（这与纸面数学坐标系相反！）
 *
 * 因此，在绘制柱状图时需要进行坐标转换：
 *   - 图表区域的"底部"（Y轴基线）= marginTop + chartH
 *   - 柱子的"高度"从底部向上延伸 —— 柱子顶部的Y坐标 = (底部Y) - (柱子高度)
 *   - 数值越大 → 柱子越高 → 柱子顶部的Y坐标越小
 *
 * ==================== 边距（Margin）设计 ====================
 * marginLeft  = 60px —— 左侧留白，给Y轴刻度数值提供空间
 * marginRight = 20px —— 右侧留白，防止最右柱子贴边
 * marginTop   = 20px —— 顶部留白（图例位于此区域的右侧）
 * marginBottom= 40px —— 底部留白，给月份标签提供空间
 *
 * 实际绘图区域（chartW × chartH）=
 *   (width - marginLeft - marginRight) × (height - marginTop - marginBottom)
 *
 * ==================== 柱子定位公式 ====================
 * 设共有 n 个数据项（n 总是偶数，因为每月有收入+支出两柱），
 * 每月一对柱子视为一个"组"。
 *
 * barSpacing = chartW / (n × 2)
 *   —— 每组（一个月份的2根柱子）占据 2×barSpacing 的宽度
 *   —— barSpacing 的一半 = 每组内部的柱子间距
 *
 * barWidth = barSpacing × 1.2
 *   —— 单根柱子的宽度略大于柱间距，相邻月份之间有微小间隙
 *
 * 第 i 根柱子的X坐标（左边缘）:
 *   x = marginLeft + barSpacing/2 + i × barSpacing × 2
 *   —— barSpacing/2: 距离图表左边缘的初始偏移
 *   —— i × barSpacing × 2: 每组占据 2×barSpacing，
 *      其中 i=0 是第一组的第一柱(收入)，i=1 是第一组的第二柱(支出)
 *
 * ==================== Y轴参考线 ====================
 * 在绘图区域内画出4条水平线（将高度4等分），形成5个刻度层次。
 * 每条线从 marginLeft 延伸到 marginLeft + chartW。
 * 线条颜色为浅灰 #E8ECF1，不抢视觉焦点。
 *
 * ==================== 绘制步骤详解 ====================
 */
void BarChartWidget::paintEvent(QPaintEvent *) {
    // -------- 创建QPainter，开启抗锯齿 --------
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // -------- 定义字体 --------
    // labelFont: 用于月份标签，9pt
    // valFont:   用于柱顶数值标签，8pt（略小以免拥挤）
    QFont labelFont("Microsoft YaHei", 9);
    QFont valFont("Microsoft YaHei", 8);

    // -------- 计算边距和绘图区域尺寸 --------
    int marginLeft = 60, marginRight = 20, marginTop = 20, marginBottom = 40;
    int chartW = width() - marginLeft - marginRight;   // 可用绘图宽度
    int chartH = height() - marginTop - marginBottom;   // 可用绘图高度

    // -------- 无数据时显示提示 --------
    if (m_data.empty() || chartW <= 0) {
        p.setPen(QColor("#BDC3C7"));
        p.setFont(QFont("Microsoft YaHei", 11));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无数据"));
        return;
    }

    // -------- 计算柱子宽度和间距 --------
    int n = (int)m_data.size();                        // 数据项总数（收入+支出交替）
    double barSpacing = chartW / (double)(n * 2);      // 每组（含2柱）的间距基准
    double barWidth = barSpacing * 0.35;               // 细柱宽度，约为间距的1/3

    // ======== 步骤1: 绘制Y轴水平参考线（4等分线） ========
    // 线条颜色为极淡的灰蓝色，形成背景网格效果但不喧宾夺主
    p.setPen(QColor("#E8ECF1"));
    for (int i = 0; i <= 4; i++) {
        // 从底部 (marginTop + chartH) 向上按 chartH/4 的步长移动
        // i=0 时 y = 底部（基线）
        // i=4 时 y = 顶部
        int y = marginTop + chartH - (chartH * i / 4);
        // 从图表左边界画到右边界
        p.drawLine(marginLeft, y, marginLeft + chartW, y);
    }

    // ======== 步骤2: 绘制柱子、标签和数值 ========
    // barColors: 交替使用的颜色列表 —— 收入用绿色(#27AE60)，支出用红色(#E74C3C)
    // 由于数据是收入/支出交替排列的，用 i%2 即可正确分配颜色
    QStringList barColors = {"#27AE60", "#E74C3C"};

    for (int i = 0; i < n; i++) {
        // ---- 计算当前柱子的高度（像素）----
        // 公式: (该数值 / 最大数值) × 图表高度
        // 例如: 当前值5000, m_maxVal=10000, chartH=200 → barH = 100像素
        double barH = (m_data[i].second / m_maxVal) * chartH;

        // ---- 计算柱子的X坐标 ----
        // barSpacing/2: 柱组起始偏移
        // i * barSpacing * 2: 每2个数据项（收入+支出）组成一个月份组
        double x = marginLeft + barSpacing / 2 + i * barSpacing * 2;

        // ---- 计算柱子的Y坐标（顶部）----
        // 底部Y = marginTop + chartH
        // 柱子从底部向上延伸 barH 像素
        // 注意: 因为Y轴向下为正，所以柱顶Y = 底部Y - 柱子高度
        double y = marginTop + chartH - barH;

        // ---- 绘制柱体（圆角矩形）----
        QColor barColor(barColors[i % 2]);       // 按索引交替取色
        p.setBrush(barColor);                    // 填充色
        p.setPen(Qt::NoPen);                     // 无边框，更清爽
        // drawRoundedRect: 绘制圆角矩形，最后两个参数是圆角半径(x, y)=4px
        p.drawRoundedRect(QRectF(x, y, barWidth, barH), 4, 4);

        // ---- 绘制柱体下方的标签（月份）----
        p.setPen(QColor("#7F8C8D"));             // 灰蓝色文字
        p.setFont(labelFont);
        // 标签区域位于图表底部下方
        // 水平居中 (Qt::AlignHCenter)，垂直靠上 (Qt::AlignTop)
        p.drawText(QRectF(x - barSpacing / 2, marginTop + chartH + 4,
                          barWidth + barSpacing, 18),
                   Qt::AlignHCenter | Qt::AlignTop, m_data[i].first);

        // ---- 绘制柱体上方的数值（金额）----
        p.setPen(QColor("#2C3E50"));             // 深灰色文字
        p.setFont(valFont);
        // 数值显示在柱子顶部上方18像素内的区域
        // 格式:  + 金额（保留0位小数，即整数显示）
        p.drawText(QRectF(x - barSpacing / 2, y - 18, barWidth + barSpacing, 16),
                   Qt::AlignHCenter | Qt::AlignBottom,
                   QString("%1").arg(m_data[i].second, 0, 'f', 0));
    }

    // ======== 步骤3: 绘制右上角图例 ========
    // 图例说明绿色柱 = 收入，红色柱 = 支出
    p.setFont(QFont("Microsoft YaHei", 9));

    // ---- 图例第一行：绿色方块 + "收入" ----
    int legendY = marginTop;
    p.setBrush(QColor("#27AE60"));               // 绿色小方块
    p.setPen(Qt::NoPen);
    p.drawRect(width() - 100, legendY, 12, 12);  // 右侧位置画12×12方块

    p.setPen(QColor("#2C3E50"));                 // 文字颜色
    // drawText的第三个参数是文本 —— 对齐方式通过 drawText 的 bounding rect
    // 和 alignment flag 控制
    p.drawText(width() - 84, legendY + 12, m_posLabel);

    // ---- 图例第二行：红色方块 + "支出" ----
    legendY += 22;                               // 向下移动22像素
    p.setBrush(QColor("#E74C3C"));               // 红色小方块
    p.setPen(Qt::NoPen);
    p.drawRect(width() - 100, legendY, 12, 12);  // 红色12×12方块

    p.setPen(QColor("#2C3E50"));
    p.drawText(width() - 84, legendY + 12, m_negLabel);
}

// ==================== StatisticsPage（统计页面主控件）====================

/*
 * 构造函数
 * 参数 ledger: 账本数据层引用 —— 提供流水记录的查询接口
 * 参数 catMan: 分类管理器引用 —— 提供分类信息
 * 参数 parent: 父级窗口（默认nullptr，通常由主窗口QStackedWidget设置）
 *
 * 初始化顺序:
 *   1. QWidget(parent) 调用基类构造函数，建立Qt父子所有权关系
 *   2. 成员初始化列表保存 ledger 和 catMan 的引用
 *   3. 函数体内调用 setupUI() 构建全部界面控件
 *
 * 注意: 构造函数中不加载数据 —— 数据加载由外部调用 refresh() 触发，
 *       或在页面首次显示时由主窗口调用
 */
StatisticsPage::StatisticsPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QWidget(parent), m_ledger(ledger), m_catMan(catMan)
{
    setupUI();
}

/*
 * setupUI — 构建页面的完整界面布局（仅在构造时调用一次）
 *
 * 布局结构（从上到下）：
 *   +------------------------------------------------------+
 *   |  统计分析 [#]                   时间范围: [本月  v]  |  ← 标题行
 *   +------------------------------------------------------+
 *   |   收入 +xxx   支出 -xxx   结余 ±xxx   笔数 xx   |  ← 汇总数字行
 *   +------------------------------------------------------+
 *   |  +-------------------+  +-------------------+        |
 *   |  |  支出分类分布      |  |  收入分类分布      |        |  ← 饼图行
 *   |  |  [甜甜圈饼图]     |  |  [甜甜圈饼图]     |        |
 *   |  +-------------------+  +-------------------+        |
 *   +------------------------------------------------------+
 *   |  +--------------------------------------------------+|
 *   |  |  月度收支趋势                                    ||
 *   |  |  [柱状图]                                        ||
 *   |  +--------------------------------------------------+|
 *   +------------------------------------------------------+  ← 柱状图卡片
 *
 * 使用 QFrame + CSS class "card" 实现卡片风格（圆角、阴影、白色背景）
 * 这些CSS样式在应用程序的全局样式表中定义
 */
void StatisticsPage::setupUI()
{
    // ======== 创建主垂直布局 ========
    // QVBoxLayout: 将子控件从上到下垂直堆叠
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(24, 24, 24, 24);   // 四周各留24px内边距
    m_mainLayout->setSpacing(16);                        // 子控件之间的间距为16px

    // ======== 第1行: 标题 + 时间范围选择器 ========
    QHBoxLayout *titleRow = new QHBoxLayout;              // 水平布局 —— 左右排列

    // 页面标题 "统计分析" —— 使用 emoji 图标作为视觉点缀
    QLabel *title = new QLabel(QStringLiteral("\U0001F4C8 统计分析"));
    title->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #2C3E50; "
        "background: transparent;");                     // 透明背景，继承卡片颜色
    titleRow->addWidget(title);

    // 添加弹性空间 —— 将后面的控件推到右侧
    titleRow->addStretch();

    // 时间范围下拉框 —— 用户选择统计数据的起止范围
    m_rangeCombo = new QComboBox;
    // 添加四个预设选项: 本月、近3个月、近1年、全部
    m_rangeCombo->addItems({QStringLiteral("本月"),
                            QStringLiteral("近3个月"),
                            QStringLiteral("近1年"),
                            QStringLiteral("全部")});
    m_rangeCombo->setFixedWidth(130);                    // 固定宽度，防止文字变化导致抖动

    // ---- 信号-槽连接 ----
    // 当用户改变下拉框选项时，QComboBox 发出 currentIndexChanged(int) 信号
    // 我们连接到 onRangeChanged() 槽函数，槽函数再调用 loadCharts() 重新加载数据
    // QOverload<int>::of(...): 因为 QComboBox 有两个重载的 currentIndexChanged
    // （一个带int参数，一个带QString参数），需要显式指定使用哪个版本
    connect(m_rangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StatisticsPage::onRangeChanged);

    // "时间范围:" 标签放在下拉框左侧
    titleRow->addWidget(new QLabel(QStringLiteral("时间范围:")));
    titleRow->addWidget(m_rangeCombo);

    m_mainLayout->addLayout(titleRow);                   // 将标题行添加到主布局

    // ======== 第2行: 四个汇总数字标签 ========
    QHBoxLayout *summaryRow = new QHBoxLayout;
    summaryRow->setSpacing(16);

    /*
     * 辅助lambda函数: createSummaryLabel
     * 作用: 创建一个统一风格的汇总数字标签
     * 参数 title: 标签的标题文字（用于标识，但实际内容由loadCharts动态设置）
     * 参数 color: 文字颜色（如绿色=收入、红色=支出、蓝色=结余、灰色=笔数）
     * 返回: 指向QLabel的指针
     *
     * 关键设置:
     *   - setTextFormat(Qt::RichText): 启用HTML富文本支持，
     *     使得 loadCharts() 中可以使用 <div>, <br> 等HTML标签来格式化文本
     *   - 字体18px 粗体 —— 数字需要醒目
     *   - 背景透明 —— 继承所在布局的背景
     */
    auto createSummaryLabel = [](const QString& title, const QString& color) -> QLabel* {
        QLabel *lbl = new QLabel;
        lbl->setStyleSheet(
            QString("font-size: 18px; font-weight: bold; color: %1; "
                    "background: transparent; padding: 8px 0;").arg(color));
        // 关键: 设置为RichText格式，否则HTML标签会被当作普通文本直接显示
        lbl->setTextFormat(Qt::RichText);
        return lbl;
    };

    // 创建四个汇总标签 —— 分别显示总收入、总支出、结余、流水笔数
    m_summaryIncome  = createSummaryLabel(QStringLiteral("收入"), "#27AE60");   // 绿色
    m_summaryExpense = createSummaryLabel(QStringLiteral("支出"), "#E74C3C");   // 红色
    m_summaryBalance = createSummaryLabel(QStringLiteral("结余"), "#3498DB");   // 蓝色
    m_summaryCount   = createSummaryLabel(QStringLiteral("笔数"), "#7F8C8D");   // 灰色

    // 将四个标签依次加入水平布局
    summaryRow->addWidget(m_summaryIncome);
    summaryRow->addWidget(m_summaryExpense);
    summaryRow->addWidget(m_summaryBalance);
    summaryRow->addWidget(m_summaryCount);
    summaryRow->addStretch();                            // 将标签组推到左侧
    m_mainLayout->addLayout(summaryRow);

    // ======== 第3行: 两个饼图卡片（支出 + 收入） ========
    QHBoxLayout *chartsRow = new QHBoxLayout;
    chartsRow->setSpacing(16);

    // ---- 左侧卡片: 支出分类分布 ----
    QFrame *expenseFrame = new QFrame;
    expenseFrame->setProperty("class", "card");          // CSS class "card" —— 应用卡片样式
    QVBoxLayout *expenseLayout = new QVBoxLayout(expenseFrame);

    QLabel *expenseTitle = new QLabel(QStringLiteral("支出分类分布"));
    expenseTitle->setStyleSheet(
        "font-size: 14px; font-weight: bold; padding: 0 0 8px 0; "
        "background: transparent;");
    expenseTitle->setAlignment(Qt::AlignCenter);          // 标题居中
    expenseLayout->addWidget(expenseTitle);

    m_expensePie = new PieChartWidget;                    // 创建饼图控件
    expenseLayout->addWidget(m_expensePie, 1);            // 伸缩因子1 —— 占据剩余空间
    chartsRow->addWidget(expenseFrame, 1);                // 两个卡片等宽（伸缩因子均为1）

    // ---- 右侧卡片: 收入分类分布 ----
    QFrame *incomeFrame = new QFrame;
    incomeFrame->setProperty("class", "card");
    QVBoxLayout *incomeLayout = new QVBoxLayout(incomeFrame);

    QLabel *incomeTitle = new QLabel(QStringLiteral("收入分类分布"));
    incomeTitle->setStyleSheet(
        "font-size: 14px; font-weight: bold; padding: 0 0 8px 0; "
        "background: transparent;");
    incomeTitle->setAlignment(Qt::AlignCenter);
    incomeLayout->addWidget(incomeTitle);

    m_incomePie = new PieChartWidget;                     // 创建饼图控件
    incomeLayout->addWidget(m_incomePie, 1);
    chartsRow->addWidget(incomeFrame, 1);

    m_mainLayout->addLayout(chartsRow, 1);                // 伸缩因子1 —— 饼图行占据主要空间

    // ======== 第4行: 月度趋势柱状图卡片 ========
    QFrame *barFrame = new QFrame;
    barFrame->setProperty("class", "card");
    QVBoxLayout *barLayout = new QVBoxLayout(barFrame);

    QLabel *barTitle = new QLabel(QStringLiteral("月度收支趋势"));
    barTitle->setStyleSheet(
        "font-size: 14px; font-weight: bold; padding: 0 0 8px 0; "
        "background: transparent;");
    barTitle->setAlignment(Qt::AlignCenter);
    barLayout->addWidget(barTitle);

    m_monthlyBar = new BarChartWidget;
    m_monthlyBar->setMinimumHeight(168);                  // 柱状图需要更多垂直空间
    barLayout->addWidget(m_monthlyBar, 1);
    m_mainLayout->addWidget(barFrame, 1);                 // 柱状图也获得伸缩因子1
}

/*
 * refresh — 刷新页面的所有数据展示
 * 这是一个公开接口，由主窗口在切换到统计页时调用
 */
void StatisticsPage::refresh()
{
    loadCharts();
}

/*
 * onRangeChanged — 响应时间范围下拉框的选项变化
 * 这是一个私有槽函数，当用户选择了不同的时间范围时自动被调用
 */
void StatisticsPage::onRangeChanged()
{
    loadCharts();
}

/*
 * loadCharts — 核心数据加载和图表更新函数
 *
 * ==================== 整体流程 ====================
 * 1. 读取用户选择的时间范围 → 计算起始日期
 * 2. 通过 Ledger 查询该范围内的所有流水记录
 * 3. 按三种维度汇总流水数据：
 *    a) 全局总计（总收入、总支出）
 *    b) 分类维度（各分类的收支汇总 → 饼图）
 *    c) 月度维度（每月的收支汇总 → 柱状图）
 * 4. 更新四个汇总标签
 * 5. 将汇总结果传入饼图和柱状图控件
 *
 * ==================== 日期范围筛选原理 ====================
 * QDate 是Qt提供的日期类，支持日期的加减运算和比较。
 *
 * 四种时间范围的计算方式:
 *   - "本月":  startDate = 本月1日
 *              例如今天是2025-06-19，startDate = 2025-06-01
 *   - "近3个月": startDate = today.addMonths(-3)
 *               addMonths(-3) 向前推3个月
 *   - "近1年":  startDate = today.addYears(-1)
 *               addYears(-1) 向前推1年
 *   - "全部":   startDate = 2000-01-01（一个极早的日期，涵盖所有数据）
 *
 * 计算出的 startDate 和 endDate（today）被转换为 "yyyy-MM-dd" 格式的字符串，
 * 传递给 Ledger::getRecordsByDateRange() 进行过滤。
 *
 * ==================== 汇总标签的HTML富文本说明 ====================
 * 因为 createSummaryLabel 中设置了 setTextFormat(Qt::RichText)，
 * 所以可以使用基础的HTML标签来格式化文本内容:
 *
 *   <div style='text-align:center;'>  —— 让整个块居中
 *   收入<br>                           —— <br> 是换行符
 *   +xxxx.xx                          —— 金额数字
 *   </div>
 *
 * 使用HTML富文本的好处:
 *   1. 可以在一个QLabel中实现多行文本（无需要多个标签垂直堆叠）
 *   2. 可以通过style属性控制文本对齐等样式
 *   3. 简化布局代码 —— 一个标签代替两行标签
 *
 * 注意: QLabel的RichText只支持HTML 4.0的一个有限子集，
 *       包括 <div>, <span>, <br>, <b>, <i>, <p> 等基本标签，
 *       不支持复杂的CSS（如flexbox, grid等）
 */
void StatisticsPage::loadCharts()
{
    // ======== 第1步: 计算时间范围 ========
    QString range = m_rangeCombo->currentText();          // 获取当前选中的范围选项文本
    QDate today = QDate::currentDate();                   // 获取今天的日期
    QDate startDate;                                      // 待计算的起始日期

    if (range == QStringLiteral("本月")) {
        // 本月: 从当前月的第1天开始
        // QDate(year, month, day) —— day=1 表示该月第一天
        startDate = QDate(today.year(), today.month(), 1);
    } else if (range == QStringLiteral("近3个月")) {
        // 近3个月: 从3个月前到今天
        // addMonths(负数) 返回新的QDate（不修改原对象）
        startDate = today.addMonths(-3);
    } else if (range == QStringLiteral("近1年")) {
        // 近1年: 从1年前的今天开始
        startDate = today.addYears(-1);
    } else {
        // 全部: 用一个足够早的日期作为起点，确保覆盖所有历史数据
        startDate = QDate(2000, 1, 1); // "全部" —— 2000年1月1日是一个合理的"最早"日期
    }

    // 将日期对象转换为标准格式的字符串 "yyyy-MM-dd"
    // 例如: 2025年6月19日 → "2025-06-19"
    QString startStr = startDate.toString("yyyy-MM-dd");
    QString endStr   = today.toString("yyyy-MM-dd");

    // ======== 第2步: 从数据层获取该时间范围内的所有流水 ========
    // Ledger::getRecordsByDateRange 按日期范围过滤流水记录
    // 参数为 string 格式的起止日期，返回符合条件的流水向量
    auto txns = m_ledger.getRecordsByDateRange(
        startStr.toStdString(), endStr.toStdString());

    // ======== 第3步: 多维度汇总计算 ========
    double totalIncome = 0, totalExpense = 0;              // 全局总计: 总收入、总支出

    // expenseByCat: 支出分类汇总 —— key=分类名称(str), value=该分类的总金额
    // incomeByCat:  收入分类汇总 —— 同上
    // 使用 std::map 是因为它自动按key排序（虽然不是必需的，但便于调试）
    std::map<std::string, double> expenseByCat, incomeByCat;

    // monthlyMap: 月度汇总 —— key=月份字符串("yyyy-MM"), value=pair<收入, 支出>
    // 例如: "2025-06" → {5000.0, 3200.0} 表示6月收入5000，支出3200
    std::map<std::string, std::pair<double, double>> monthlyMap;

    // 遍历所有流水记录，按类型分别累加到对应的汇总容器中
    for (const auto& t : txns) {
        if (t.type == RecordType::INCOME) {
            // ---- 收入流水 ----
            totalIncome += t.amount;                       // 累加总收入
            incomeByCat[t.category] += t.amount;           // 累加该分类的收入
        } else {
            // ---- 支出流水 ----
            totalExpense += t.amount;                      // 累加总支出
            expenseByCat[t.category] += t.amount;          // 累加该分类的支出
        }

        // ---- 月度汇总 ----
        // t.date 格式为 "yyyy-MM-dd"（如 "2025-06-19"）
        // .substr(0, 7) 截取前7个字符得到 "yyyy-MM"（如 "2025-06"）
        std::string month = t.date.substr(0, 7);

        // monthlyMap[month] 如果month键不存在，会自动创建一个
        // 默认值为 pair<double, double>(0, 0)
        if (t.type == RecordType::INCOME)
            monthlyMap[month].first += t.amount;           // first = 收入累加
        else
            monthlyMap[month].second += t.amount;          // second = 支出累加
    }

    // 计算结余 = 总收入 - 总支出
    double balance = totalIncome - totalExpense;

    // ======== 第4步: 更新四个汇总数字标签（HTML富文本） ========

    // ---- 收入标签: 绿色，显示 "收入<br>+xxxx.xx" ----
    // arg(totalIncome, 0, 'f', 2) 的含义:
    //   参数1(0) = 最小字段宽度（0表示不设最小宽度，自动）
    //   参数2('f') = 格式字符（f=定点格式，即普通小数）
    //   参数3(2) = 精度（保留2位小数）
    m_summaryIncome->setText(
        QString("<div style='text-align:center;'>收入<br>+%1</div>").arg(totalIncome, 0, 'f', 2));

    // ---- 支出标签: 红色，显示 "支出<br>-xxxx.xx" ----
    m_summaryExpense->setText(
        QString("<div style='text-align:center;'>支出<br>-%1</div>").arg(totalExpense, 0, 'f', 2));

    // ---- 结余标签: 蓝色，正数前加"+"号 ----
    // balance >= 0 ? "+" : "" —— 正结余显示"+"，负结余显示"-"（arg已含负号）
    m_summaryBalance->setText(
        QString("<div style='text-align:center;'>结余<br>%1%2</div>")
            .arg(balance >= 0 ? "+" : "").arg(balance, 0, 'f', 2));

    // ---- 笔数标签: 灰色，显示 "笔数<br>xx" ----
    // txns.size() 返回 size_t 类型，QString::arg 自动处理
    m_summaryCount->setText(
        QString("<div style='text-align:center;'>笔数<br>%1</div>").arg(txns.size()));

    // ======== 第5步: 构建支出饼图数据 ========
    // 用大括号创建局部作用域，内部的临时变量在离开作用域时自动销毁，避免命名冲突
    {
        // ---- 将 std::map 转为 std::vector 以便排序 ----
        std::vector<std::pair<QString, double>> pieData;   // 最终传给饼图的数据
        std::vector<std::pair<std::string, double>> sorted; // 临时: 用于排序

        // 将 expenseByCat 的内容复制到 sorted 向量中
        for (const auto& [cat, val] : expenseByCat) sorted.push_back({cat, val});

        // 按金额降序排列 —— 金额最大的分类扇区最先绘制（从12点方向顺时针）
        // Lambda比较器: a.second > b.second 表示"a的金额大于b的金额"
        // 排序使得大头分类在饼图中更加突出
        std::sort(sorted.begin(), sorted.end(),
            [](auto& a, auto& b) { return a.second > b.second; });

        // 将排序后的数据转为 QString 类型的键（Qt控件使用QString而非std::string）
        for (const auto& [cat, val] : sorted)
            pieData.push_back({QString::fromStdString(cat), val});

        // ---- 定义支出饼图的颜色方案 ----
        // 以红色系和暖色系为主，符合"支出"的视觉感受
        QStringList colors = {"#E74C3C", "#E67E22", "#F39C12", "#3498DB",
                              "#9B59B6", "#1ABC9C", "#2ECC71", "#F1C40F",
                              "#E91E63", "#00BCD4", "#FF5722", "#795548"};

        // 调用饼图控件的setData，传入数据+颜色，控件内部自动重绘
        m_expensePie->setData(pieData, colors);
    }

    // ======== 第6步: 构建收入饼图数据 ========
    // 逻辑与支出饼图完全相同，只是数据来源不同
    {
        std::vector<std::pair<QString, double>> pieData;
        std::vector<std::pair<std::string, double>> sorted;

        for (const auto& [cat, val] : incomeByCat) sorted.push_back({cat, val});

        // 同样按金额降序排列
        std::sort(sorted.begin(), sorted.end(),
            [](auto& a, auto& b) { return a.second > b.second; });

        for (const auto& [cat, val] : sorted)
            pieData.push_back({QString::fromStdString(cat), val});

        // ---- 定义收入饼图的颜色方案 ----
        // 以绿色系为主，符合"收入/正向"的视觉感受
        QStringList colors = {"#27AE60", "#2ECC71", "#1ABC9C", "#3498DB",
                              "#9B59B6", "#F39C12", "#E67E22", "#F1C40F"};

        m_incomePie->setData(pieData, colors);
    }

    // ======== 第7步: 构建月度趋势柱状图数据 ========
    {
        // ---- 将月度汇总 map 转为 vector 并按月份排序 ----
        // 月份字符串 "2025-01", "2025-02", ... 的自然字符串排序 = 时间排序
        std::vector<std::pair<std::string, std::pair<double, double>>> sortedMonthly;
        for (const auto& [month, vals] : monthlyMap) sortedMonthly.push_back({month, vals});
        std::sort(sortedMonthly.begin(), sortedMonthly.end());

        // ---- 只保留最近12个月的数据 ----
        // 如果月份数量超过12，删除前面的旧数据（erase从begin到end-12）
        // 例如: 有18个月的数据 → 删除前6个月，保留最近12个月
        // 这样做是因为柱状图空间有限，太多柱子会相互挤在一起无法辨认
        if (sortedMonthly.size() > 12) {
            sortedMonthly.erase(sortedMonthly.begin(),
                                sortedMonthly.end() - 12);
        }

        // ---- 将月度数据转换为柱状图所需格式: 收入/支出交替排列 ----
        // 柱状图的数据格式要求: [月1收入, 月1支出, 月2收入, 月2支出, ...]
        // 这样每个月份的两个柱子在图表上相邻显示，便于视觉对比
        std::vector<std::pair<QString, double>> barData;
        for (const auto& [month, vals] : sortedMonthly) {
            // 先加入该月的收入柱
            barData.push_back({QString::fromStdString(month), vals.first});
            // 再加入该月的支出柱
            barData.push_back({QString::fromStdString(month), vals.second});
        }

        // 传入数据及图例标签 "收入" / "支出"
        m_monthlyBar->setData(barData,
                              QStringLiteral("收入"),
                              QStringLiteral("支出"));
    }
}
