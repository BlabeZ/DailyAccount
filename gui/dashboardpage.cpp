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
#include "budget.h"
#include "goal.h"
#include <QHeaderView>
#include <QDate>
#include <QVBoxLayout>

DashboardPage::DashboardPage(Ledger& ledger, CategoryManager& catMan,
                             BudgetManager& budgetMan, GoalManager& goalMan,
                             QWidget *parent)
    : QWidget(parent)
    , m_ledger(ledger)
    , m_catMan(catMan)
    , m_budgetMan(budgetMan)
    , m_goalMan(goalMan)
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

    // 插入预算概览区域（在汇总卡片和底部双栏之间）
    // mainLayout: [0]pageTitle [1]cardsRow [2]bottomRow
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        // 预算概览占位（将在refreshBudgetOverview中填充）
        QWidget *budgetPlaceholder = new QWidget;
        budgetPlaceholder->setObjectName("budgetOverview");
        budgetPlaceholder->setStyleSheet("background: transparent;");
        budgetPlaceholder->setLayout(new QVBoxLayout);
        budgetPlaceholder->layout()->setContentsMargins(0, 0, 0, 0);
        mainLayout->insertWidget(2, budgetPlaceholder);

        // 储蓄目标占位（在底部双栏之后）
        QWidget *goalPlaceholder = new QWidget;
        goalPlaceholder->setObjectName("goalCards");
        goalPlaceholder->setStyleSheet("background: transparent;");
        goalPlaceholder->setLayout(new QVBoxLayout);
        goalPlaceholder->layout()->setContentsMargins(0, 0, 0, 0);
        mainLayout->addWidget(goalPlaceholder);
    }
}

void DashboardPage::refresh()
{
    refreshSummary();
    refreshRecentRecords();
    refreshBudgetOverview();
    refreshCategoryBreakdown();
    refreshGoalCards();
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

void DashboardPage::refreshBudgetOverview()
{
    QWidget *container = this->findChild<QWidget*>("budgetOverview");
    if (!container) return;
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(container->layout());

    // 清空旧内容
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    QDate today = QDate::currentDate();
    std::string month = today.toString("yyyy-MM").toStdString();
    Budget overallB = m_budgetMan.getBudget(month, "OVERALL");

    if (overallB.amount <= 0.0) {
        container->setVisible(false);
        return;
    }
    container->setVisible(true);

    double pct = overallB.percentage();
    QString barColor = (pct >= 100) ? "#E74C3C" : (pct >= 80) ? "#F39C12" : "#27AE60";

    // 标题行
    QHBoxLayout *titleRow = new QHBoxLayout;
    QLabel *titleLabel = new QLabel("💰 本月预算");
    titleLabel->setStyleSheet("font-size: 15px; font-weight: bold; background: transparent; "
                               "padding: 0;");
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    layout->addLayout(titleRow);

    // 进度条卡片
    QWidget *card = new QWidget;
    card->setStyleSheet("background: white; border: 1px solid #E8ECF1; "
                         "border-radius: 10px; padding: 16px;");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(8);

    // 信息行
    QHBoxLayout *infoRow = new QHBoxLayout;
    QString spentText = QString("已花费 <b style='color:#E74C3C;'>¥%1</b>")
                            .arg(overallB.spent, 0, 'f', 2);
    QLabel *spentLabel = new QLabel(spentText);
    spentLabel->setStyleSheet("font-size: 13px; background: transparent;");
    QString budgetText = QString("预算 <b>¥%1</b>").arg(overallB.amount, 0, 'f', 2);
    QLabel *budgetLabel = new QLabel(budgetText);
    budgetLabel->setStyleSheet("font-size: 13px; color: #7F8C8D; background: transparent;");
    QString remainText = QString("剩余 <b style='color:%1;'>¥%2</b>")
                             .arg(overallB.remaining() >= 0 ? "#27AE60" : "#E74C3C")
                             .arg(overallB.remaining(), 0, 'f', 2);
    QLabel *remainLabel = new QLabel(remainText);
    remainLabel->setStyleSheet("font-size: 13px; background: transparent;");

    infoRow->addWidget(spentLabel);
    infoRow->addStretch();
    infoRow->addWidget(budgetLabel);
    infoRow->addStretch();
    infoRow->addWidget(remainLabel);
    cardLayout->addLayout(infoRow);

    // 进度条
    QWidget *barBg = new QWidget;
    barBg->setFixedHeight(24);
    barBg->setStyleSheet("background: #ECF0F1; border-radius: 12px; border: none;");
    QHBoxLayout *barBgLayout = new QHBoxLayout(barBg);
    barBgLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *barFill = new QWidget;
    barFill->setFixedHeight(24);
    barFill->setStyleSheet(QString("background: %1; border-radius: 12px; border: none;").arg(barColor));
    int fillPct = (int)(pct > 100 ? 100 : pct);
    barBgLayout->addWidget(barFill, fillPct);
    barBgLayout->addStretch(100 - fillPct);
    cardLayout->addWidget(barBg);

    // 百分比标签
    QLabel *pctLabel = new QLabel(QString("%1%").arg(pct, 0, 'f', 0));
    pctLabel->setStyleSheet(QString("font-size: 13px; font-weight: bold; color: %1; "
                                     "background: transparent;").arg(barColor));
    pctLabel->setAlignment(Qt::AlignRight);
    cardLayout->addWidget(pctLabel);

    layout->addWidget(card);
}

void DashboardPage::refreshGoalCards()
{
    QWidget *container = this->findChild<QWidget*>("goalCards");
    if (!container) return;
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(container->layout());

    // 清空旧内容
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    const auto& goals = m_goalMan.getAllGoals();
    if (goals.empty()) {
        container->setVisible(false);
        return;
    }
    container->setVisible(true);
    layout->setSpacing(12);

    // 标题
    QLabel *titleLabel = new QLabel("🎯 储蓄目标");
    titleLabel->setStyleSheet("font-size: 15px; font-weight: bold; background: transparent; "
                               "padding: 0; margin-top: 4px;");
    layout->addWidget(titleLabel);

    // 目标卡片容器
    QWidget *cardsContainer = new QWidget;
    cardsContainer->setStyleSheet("background: transparent;");
    QHBoxLayout *cardsRow = new QHBoxLayout(cardsContainer);
    cardsRow->setContentsMargins(0, 0, 0, 0);
    cardsRow->setSpacing(16);

    int showCount = std::min(3, (int)goals.size());

    for (int i = 0; i < showCount; i++) {
        const auto& g = goals[i];
        double pct = g.progressPercent();
        QString cardColor = (pct >= 100) ? "#27AE60" : (pct >= 50) ? "#3498DB" : "#F39C12";

        QWidget *card = new QWidget;
        card->setStyleSheet("background: white; border: 1px solid #E8ECF1; "
                             "border-radius: 10px; padding: 14px;");
        QVBoxLayout *cLayout = new QVBoxLayout(card);
        cLayout->setSpacing(8);

        // 目标名称
        QLabel *name = new QLabel(QString::fromStdString(g.name));
        name->setStyleSheet("font-size: 13px; font-weight: bold; background: transparent;");

        // 进度条
        QWidget *barBg = new QWidget;
        barBg->setFixedHeight(16);
        barBg->setStyleSheet("background: #ECF0F1; border-radius: 8px; border: none;");
        QHBoxLayout *barBgLayout = new QHBoxLayout(barBg);
        barBgLayout->setContentsMargins(0, 0, 0, 0);
        QWidget *barFill = new QWidget;
        barFill->setFixedHeight(16);
        barFill->setStyleSheet(QString("background: %1; border-radius: 8px; border: none;").arg(cardColor));
        int fillPct = (int)(pct > 100 ? 100 : pct);
        barBgLayout->addWidget(barFill, fillPct);
        barBgLayout->addStretch(100 - fillPct);

        // 金额信息
        QString amtText = QString("¥%1 / ¥%2  (%3%)")
                              .arg(g.currentSaved, 0, 'f', 0)
                              .arg(g.targetAmount, 0, 'f', 0)
                              .arg(pct, 0, 'f', 0);
        QLabel *amount = new QLabel(amtText);
        amount->setStyleSheet("font-size: 11px; color: #7F8C8D; background: transparent;");

        // 每月需存提示
        double monthly = g.monthlyNeeded();
        QLabel *hint = new QLabel;
        if (monthly > 0 && !g.isCompleted()) {
            QString hintStr = QString("还需每月存 ¥%1 · 约 ¥%2/天")
                                  .arg(monthly, 0, 'f', 0)
                                  .arg(g.dailyNeeded(), 0, 'f', 0);
            hint->setText(hintStr);
            hint->setStyleSheet("font-size: 11px; color: #95A5A6; background: transparent;");
        } else if (g.isCompleted()) {
            hint->setText("✅ 已达成!");
            hint->setStyleSheet("font-size: 11px; color: #27AE60; font-weight: bold; "
                                 "background: transparent;");
        }

        cLayout->addWidget(name);
        cLayout->addWidget(barBg);
        cLayout->addWidget(amount);
        if (hint) cLayout->addWidget(hint);

        cardsRow->addWidget(card, 1);
    }
    layout->addWidget(cardsContainer);
}
