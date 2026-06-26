/*
 * ===========================================================================
 * 文件名称：dashboardpage.cpp
 * 所属模块：GUI 图形用户界面 - 概览页面
 * 功能描述：DashboardPage 实现。界面由 dashboardpage.ui 定义。
 * 编码格式：UTF-8
 * ===========================================================================
 */

#include "dashboardpage.h"
#include "ledger.h"
#include "category.h"
#include <QHeaderView>

DashboardPage::DashboardPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QWidget(parent)
    , m_ledger(ledger)
    , m_catMan(catMan)
    , ui(new Ui::DashboardPage)
{
    ui->setupUi(this);

    // 配置最近流水表格
    ui->recentTable->horizontalHeader()->setStretchLastSection(true);
    ui->recentTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->recentTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->recentTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->recentTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->recentTable->verticalHeader()->setVisible(false);
}

void DashboardPage::refresh()
{
    refreshSummary();
    refreshRecentRecords();
    refreshCategoryBreakdown();
}

void DashboardPage::refreshSummary()
{
    double income  = m_ledger.getTotalIncome();
    double expense = m_ledger.getTotalExpense();
    double balance = m_ledger.getBalance();

    auto cardStyle = [](const QString& icon, const QString& title,
                         const QString& amount, const QString& color) -> QString {
        return QString(
            "<div style='background:white; border-left:4px solid %1; border-radius:8px; "
            "padding:18px 16px; margin:0;'>"
            "<div style='font-size:13px; color:#7F8C8D; margin-bottom:6px;'>%2  %3</div>"
            "<div style='font-size:22px; font-weight:bold; color:%1;'>%4</div>"
            "</div>").arg(color, icon, title, amount);
    };

    ui->cardIncome->setText(
        cardStyle("", "本月收入", QString("+%1").arg(income, 0, 'f', 2), "#27AE60"));
    ui->cardExpense->setText(
        cardStyle("", "本月支出", QString("-%1").arg(expense, 0, 'f', 2), "#E74C3C"));
    ui->cardBalance->setText(
        cardStyle("", "本月结余",
                  QString("%1%2").arg(balance >= 0 ? "+" : "").arg(balance, 0, 'f', 2),
                  balance >= 0 ? "#3498DB" : "#E74C3C"));
}

void DashboardPage::refreshRecentRecords()
{
    const auto& all = m_ledger.getAllRecords();
    int count = std::min(10, (int)all.size());
    ui->recentTable->setRowCount(count);

    for (int i = 0; i < count; i++) {
        const auto& t = all[all.size() - 1 - i];
        auto setItem = [&](int col, const QString& text, const QString& color = "#2C3E50") {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setForeground(QColor(color));
            ui->recentTable->setItem(i, col, item);
        };

        setItem(0, QString::fromStdString(t.date), "#7F8C8D");
        bool isIncome = (t.type == RecordType::INCOME);
        setItem(1, isIncome ? "收入" : "支出", isIncome ? "#27AE60" : "#E74C3C");
        QString amt = QString(isIncome ? "+%1" : "-%1").arg(t.amount, 0, 'f', 2);
        setItem(2, amt, isIncome ? "#27AE60" : "#E74C3C");
        setItem(3, QString::fromStdString(t.category));
        setItem(4, QString::fromStdString(t.note), "#95A5A6");
    }
    ui->recentTable->resizeColumnsToContents();
}

void DashboardPage::refreshCategoryBreakdown()
{
    // 清空旧的分类分布内容
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->categoryBreakdown->layout());
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    auto stats = m_ledger.getCategorySummaries(RecordType::EXPENSE);
    double totalExpense = m_ledger.getTotalExpense();

    if (stats.empty() || totalExpense <= 0) {
        QLabel *empty = new QLabel("暂无支出数据");
        empty->setStyleSheet("color: #95A5A6; font-size: 13px; padding: 20px; "
                              "background: transparent;");
        empty->setAlignment(Qt::AlignCenter);
        layout->addWidget(empty);
        return;
    }

    int show = std::min(6, (int)stats.size());
    QStringList colors = {"#E74C3C", "#E67E22", "#F39C12", "#3498DB",
                          "#9B59B6", "#1ABC9C"};

    for (int i = 0; i < show; i++) {
        const auto& cs = stats[i];
        double pct = cs.percentage;

        QWidget *rowWidget = new QWidget;
        rowWidget->setStyleSheet("background: transparent;");
        QHBoxLayout *row = new QHBoxLayout(rowWidget);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(8);

        QLabel *nameLabel = new QLabel(QString::fromStdString(cs.category));
        nameLabel->setFixedWidth(60);
        nameLabel->setStyleSheet("font-size: 12px; background: transparent;");

        QString barColor = colors[i % colors.size()];
        QWidget *bar = new QWidget;
        bar->setFixedHeight(14);
        bar->setStyleSheet("background: #F0F3F7; border: none;");
        QHBoxLayout *barLayout = new QHBoxLayout(bar);
        barLayout->setContentsMargins(0, 0, 0, 0);
        QWidget *fill = new QWidget;
        fill->setFixedHeight(14);
        fill->setStyleSheet(QString("background: %1; border: none;").arg(barColor));
        barLayout->addWidget(fill, (int)pct);
        barLayout->addStretch(100 - (int)pct);

        QLabel *pctLabel = new QLabel(QString("%1%").arg(pct, 0, 'f', 1));
        pctLabel->setFixedWidth(40);
        pctLabel->setStyleSheet("font-size: 11px; color: #7F8C8D; background: transparent;");

        row->addWidget(nameLabel);
        row->addWidget(bar, 1);
        row->addWidget(pctLabel);

        layout->addWidget(rowWidget);
    }
}
