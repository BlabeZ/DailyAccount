/*
 * ============================================================================
 * 文件名: statisticspage.cpp
 * 模块:   统计页面（GUI实现文件）
 * 功能:   实现三个自绘图表控件及统计页面数据加载逻辑。
 *         界面框架由 statisticspage.ui 定义，图表控件在代码中创建。
 * 编码:   UTF-8
 * ============================================================================
 */

#include "statisticspage.h"
#include "ledger.h"
#include "category.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDate>
#include <cmath>
#include <algorithm>

// ==================== PieChartWidget ====================
PieChartWidget::PieChartWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(280, 250);
    setStyleSheet("background: transparent;");
}

void PieChartWidget::setData(const std::vector<std::pair<QString, double>>& data,
                              const QStringList& colors) {
    m_data = data;
    m_colors = colors;
    m_total = 0;
    for (const auto& [name, val] : data) m_total += val;
    update();
}

void PieChartWidget::paintEvent(QPaintEvent *) {
    if (m_data.empty() || m_total <= 0) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QColor("#BDC3C7"));
        p.setFont(QFont("Microsoft YaHei", 11));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无数据"));
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int side = (int)(std::min(width(), height()) * 0.7) - 10;
    int cx = side / 2 + 20;
    int cy = height() / 2;
    QRectF pieRect(cx - side / 2, cy - side / 2, side, side);
    double angle = 90.0 * 16;
    for (size_t i = 0; i < m_data.size(); i++) {
        double sweep = (m_data[i].second / m_total) * 360.0 * 16;
        QColor color(m_colors[i % m_colors.size()]);
        p.setBrush(color);
        p.setPen(Qt::white);
        p.drawPie(pieRect, (int)angle, (int)sweep);
        angle += sweep;
    }
    int innerR = side / 5;
    QRectF innerRect(cx - innerR, cy - innerR, innerR * 2, innerR * 2);
    p.setBrush(QColor("#F5F7FA"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(innerRect);
    // 右侧图例
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
    }
}

// ==================== BarChartWidget ====================
BarChartWidget::BarChartWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(240, 150);
    setStyleSheet("background: transparent;");
}

void BarChartWidget::setData(const std::vector<std::pair<QString, double>>& data,
                              const QString& positiveLabel, const QString& negativeLabel) {
    m_data = data;
    m_posLabel = positiveLabel;
    m_negLabel = negativeLabel;
    m_maxVal = 0;
    for (const auto& [name, val] : data) {
        if (val > m_maxVal) m_maxVal = val;
    }
    m_maxVal *= 1.15;
    if (m_maxVal <= 0) m_maxVal = 1;
    update();
}

void BarChartWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QFont labelFont("Microsoft YaHei", 9);
    QFont valFont("Microsoft YaHei", 8);
    int marginLeft = 60, marginRight = 20, marginTop = 20, marginBottom = 40;
    int chartW = width() - marginLeft - marginRight;
    int chartH = height() - marginTop - marginBottom;
    if (m_data.empty() || chartW <= 0) {
        p.setPen(QColor("#BDC3C7"));
        p.setFont(QFont("Microsoft YaHei", 11));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无数据"));
        return;
    }
    int n = (int)m_data.size();
    double barSpacing = chartW / (double)(n * 2);
    double barWidth = barSpacing * 0.35;
    p.setPen(QColor("#E8ECF1"));
    for (int i = 0; i <= 4; i++) {
        int y = marginTop + chartH - (chartH * i / 4);
        p.drawLine(marginLeft, y, marginLeft + chartW, y);
    }
    QStringList barColors = {"#27AE60", "#E74C3C"};
    for (int i = 0; i < n; i++) {
        double barH = (m_data[i].second / m_maxVal) * chartH;
        double x = marginLeft + barSpacing / 2 + i * barSpacing * 2;
        double y = marginTop + chartH - barH;
        QColor barColor(barColors[i % 2]);
        p.setBrush(barColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(x, y, barWidth, barH), 4, 4);
        p.setPen(QColor("#7F8C8D"));
        p.setFont(labelFont);
        p.drawText(QRectF(x - barSpacing / 2, marginTop + chartH + 4,
                          barWidth + barSpacing, 18),
                   Qt::AlignHCenter | Qt::AlignTop, m_data[i].first);
        p.setPen(QColor("#2C3E50"));
        p.setFont(valFont);
        p.drawText(QRectF(x - barSpacing / 2, y - 18, barWidth + barSpacing, 16),
                   Qt::AlignHCenter | Qt::AlignBottom,
                   QString("%1").arg(m_data[i].second, 0, 'f', 0));
    }
    p.setFont(QFont("Microsoft YaHei", 9));
    int legendY = marginTop;
    p.setBrush(QColor("#27AE60"));
    p.setPen(Qt::NoPen);
    p.drawRect(width() - 100, legendY, 12, 12);
    p.setPen(QColor("#2C3E50"));
    p.drawText(width() - 84, legendY + 12, m_posLabel);
    legendY += 22;
    p.setBrush(QColor("#E74C3C"));
    p.setPen(Qt::NoPen);
    p.drawRect(width() - 100, legendY, 12, 12);
    p.setPen(QColor("#2C3E50"));
    p.drawText(width() - 84, legendY + 12, m_negLabel);
}

// ==================== StatisticsPage ====================
StatisticsPage::StatisticsPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QWidget(parent)
    , m_ledger(ledger)
    , m_catMan(catMan)
    , ui(new Ui::StatisticsPage)
{
    ui->setupUi(this);

    // 替换占位控件为自定义图表
    m_expensePie = new PieChartWidget;
    ui->expensePieLayout->replaceWidget(ui->expensePiePlaceholder, m_expensePie);
    delete ui->expensePiePlaceholder;

    m_incomePie = new PieChartWidget;
    ui->incomePieLayout->replaceWidget(ui->incomePiePlaceholder, m_incomePie);
    delete ui->incomePiePlaceholder;

    m_monthlyBar = new BarChartWidget;
    m_monthlyBar->setMinimumHeight(168);
    ui->barLayout->replaceWidget(ui->barPlaceholder, m_monthlyBar);
    delete ui->barPlaceholder;

    connect(ui->rangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StatisticsPage::onRangeChanged);
}

void StatisticsPage::refresh() { loadCharts(); }
void StatisticsPage::onRangeChanged() { loadCharts(); }

void StatisticsPage::loadCharts()
{
    QString range = ui->rangeCombo->currentText();
    QDate today = QDate::currentDate();
    QDate startDate;

    if (range == QStringLiteral("本月"))
        startDate = QDate(today.year(), today.month(), 1);
    else if (range == QStringLiteral("近3个月"))
        startDate = today.addMonths(-3);
    else if (range == QStringLiteral("近1年"))
        startDate = today.addYears(-1);
    else
        startDate = QDate(2000, 1, 1);

    QString startStr = startDate.toString("yyyy-MM-dd");
    QString endStr   = today.toString("yyyy-MM-dd");
    auto txns = m_ledger.getRecordsByDateRange(startStr.toStdString(), endStr.toStdString());

    double totalIncome = 0, totalExpense = 0;
    std::map<std::string, double> expenseByCat, incomeByCat;
    std::map<std::string, std::pair<double, double>> monthlyMap;

    for (const auto& t : txns) {
        if (t.type == RecordType::INCOME) {
            totalIncome += t.amount;
            incomeByCat[t.category] += t.amount;
        } else {
            totalExpense += t.amount;
            expenseByCat[t.category] += t.amount;
        }
        std::string month = t.date.substr(0, 7);
        if (t.type == RecordType::INCOME)
            monthlyMap[month].first += t.amount;
        else
            monthlyMap[month].second += t.amount;
    }

    double balance = totalIncome - totalExpense;

    ui->summaryIncome->setText(
        QString("<div style='text-align:center;'>收入<br>+%1</div>").arg(totalIncome, 0, 'f', 2));
    ui->summaryExpense->setText(
        QString("<div style='text-align:center;'>支出<br>-%1</div>").arg(totalExpense, 0, 'f', 2));
    ui->summaryBalance->setText(
        QString("<div style='text-align:center;'>结余<br>%1%2</div>")
            .arg(balance >= 0 ? "+" : "").arg(balance, 0, 'f', 2));
    ui->summaryCount->setText(
        QString("<div style='text-align:center;'>笔数<br>%1</div>").arg(txns.size()));

    // 支出饼图
    {
        std::vector<std::pair<QString, double>> pieData;
        std::vector<std::pair<std::string, double>> sorted;
        for (const auto& [cat, val] : expenseByCat) sorted.push_back({cat, val});
        std::sort(sorted.begin(), sorted.end(),
            [](auto& a, auto& b) { return a.second > b.second; });
        for (const auto& [cat, val] : sorted)
            pieData.push_back({QString::fromStdString(cat), val});
        QStringList colors = {"#E74C3C", "#E67E22", "#F39C12", "#3498DB",
                              "#9B59B6", "#1ABC9C", "#2ECC71", "#F1C40F",
                              "#E91E63", "#00BCD4", "#FF5722", "#795548"};
        m_expensePie->setData(pieData, colors);
    }

    // 收入饼图
    {
        std::vector<std::pair<QString, double>> pieData;
        std::vector<std::pair<std::string, double>> sorted;
        for (const auto& [cat, val] : incomeByCat) sorted.push_back({cat, val});
        std::sort(sorted.begin(), sorted.end(),
            [](auto& a, auto& b) { return a.second > b.second; });
        for (const auto& [cat, val] : sorted)
            pieData.push_back({QString::fromStdString(cat), val});
        QStringList colors = {"#27AE60", "#2ECC71", "#1ABC9C", "#3498DB",
                              "#9B59B6", "#F39C12", "#E67E22", "#F1C40F"};
        m_incomePie->setData(pieData, colors);
    }

    // 月度柱状图
    {
        std::vector<std::pair<std::string, std::pair<double, double>>> sortedMonthly;
        for (const auto& [month, vals] : monthlyMap) sortedMonthly.push_back({month, vals});
        std::sort(sortedMonthly.begin(), sortedMonthly.end());
        if (sortedMonthly.size() > 12)
            sortedMonthly.erase(sortedMonthly.begin(), sortedMonthly.end() - 12);

        std::vector<std::pair<QString, double>> barData;
        for (const auto& [month, vals] : sortedMonthly) {
            barData.push_back({QString::fromStdString(month), vals.first});
            barData.push_back({QString::fromStdString(month), vals.second});
        }
        m_monthlyBar->setData(barData, QStringLiteral("收入"), QStringLiteral("支出"));
    }
}
