/*
 * ============================================================================
 * 文件名: statisticspage.h
 * 模块:   统计页面（GUI头文件）
 * 功能:   声明统计页面相关类。界面由 statisticspage.ui 定义。
 *         自定义图表控件（PieChartWidget、BarChartWidget）在代码中创建。
 * 编码:   UTF-8
 * ============================================================================
 */

#ifndef STATISTICSPAGE_H
#define STATISTICSPAGE_H

#include <QWidget>
#include "ui_statisticspage.h"

class Ledger;
class CategoryManager;
class PieChartWidget;
class BarChartWidget;

// ==================== 自定义饼图控件 ====================
class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit PieChartWidget(QWidget *parent = nullptr);
    void setData(const std::vector<std::pair<QString, double>>& data,
                 const QStringList& colors);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    std::vector<std::pair<QString, double>> m_data;
    QStringList m_colors;
    double m_total = 0;
};

// ==================== 自定义柱状图控件 ====================
class BarChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit BarChartWidget(QWidget *parent = nullptr);
    void setData(const std::vector<std::pair<QString, double>>& data,
                 const QString& positiveLabel, const QString& negativeLabel);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    std::vector<std::pair<QString, double>> m_data;
    QString m_posLabel;
    QString m_negLabel;
    double m_maxVal = 0;
};

// ==================== 统计页面主控件 ====================
class StatisticsPage : public QWidget {
    Q_OBJECT

public:
    explicit StatisticsPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent = nullptr);
    void refresh();

private slots:
    void onRangeChanged();

private:
    void loadCharts();

    Ledger& m_ledger;
    CategoryManager& m_catMan;

    Ui::StatisticsPage *ui;
    PieChartWidget *m_expensePie;
    PieChartWidget *m_incomePie;
    BarChartWidget *m_monthlyBar;
};

#endif // STATISTICSPAGE_H
