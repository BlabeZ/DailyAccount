/*
 * ===========================================================================
 * 文件名称: otherpage.cpp
 * 所属模块: gui
 * 功能描述: OtherPage 实现 —— 界面由 otherpage.ui 定义。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "otherpage.h"
#include "ledger.h"
#include "goal.h"
#include "record.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QInputDialog>
#include <QListWidget>
#include <set>
#include <vector>

OtherPage::OtherPage(Ledger& ledger, GoalManager& goalMan, QWidget *parent)
    : QWidget(parent)
    , m_ledger(ledger)
    , m_goalMan(goalMan)
    , ui(new Ui::OtherPage)
{
    ui->setupUi(this);
    ui->featureStack->setCurrentIndex(0);

    // 信号连接
    connect(ui->btnExportEnter, &QPushButton::clicked, this, &OtherPage::showExportDetail);
    connect(ui->btnClearEnter, &QPushButton::clicked, this, &OtherPage::showClearDetail);
    connect(ui->btnExportBack, &QPushButton::clicked, this, &OtherPage::showFeatureList);
    connect(ui->btnClearBack, &QPushButton::clicked, this, &OtherPage::showFeatureList);
    connect(ui->btnExportData, &QPushButton::clicked, this, &OtherPage::onExportData);
    connect(ui->btnClearData, &QPushButton::clicked, this, &OtherPage::onClearData);

    // 添加储蓄目标入口卡片到功能列表页
    QVBoxLayout *featureListLayout = ui->featureListPage->findChild<QVBoxLayout*>("featureListLayout");
    if (featureListLayout) {
        // 在spacer之前插入目标卡片
        QFrame *goalCard = new QFrame;
        goalCard->setStyleSheet("QFrame { background: white; border-radius: 10px; "
                                 "border: 1px solid #E8ECF1; padding: 20px; }");
        QHBoxLayout *goalCardLayout = new QHBoxLayout(goalCard);
        QLabel *goalIcon = new QLabel("🎯");
        goalIcon->setStyleSheet("font-size: 28px; background: transparent;");
        goalCardLayout->addWidget(goalIcon);
        QVBoxLayout *goalTextLayout = new QVBoxLayout;
        QLabel *goalTitle = new QLabel("储蓄目标");
        goalTitle->setStyleSheet("font-size: 16px; font-weight: bold; background: transparent;");
        QLabel *goalDesc = new QLabel("设定储蓄目标，追踪进度，自动计算每月需存金额");
        goalDesc->setStyleSheet("font-size: 12px; color: #7F8C8D; background: transparent;");
        goalTextLayout->addWidget(goalTitle);
        goalTextLayout->addWidget(goalDesc);
        goalCardLayout->addLayout(goalTextLayout);

        QPushButton *btnGoalEnter = new QPushButton("进入 →");
        btnGoalEnter->setCursor(Qt::PointingHandCursor);
        btnGoalEnter->setStyleSheet("QPushButton { background: #9B59B6; color: white; "
                                     "border-radius: 6px; padding: 10px 20px; } "
                                     "QPushButton:hover { background: #8E44AD; }");
        connect(btnGoalEnter, &QPushButton::clicked, this, &OtherPage::showGoalDetail);
        goalCardLayout->addWidget(btnGoalEnter);

        // 插入到spacer之前（倒数第一个item是spacer）
        int spacerIdx = featureListLayout->count() - 1;
        featureListLayout->insertWidget(spacerIdx, goalCard);
    }

    // 添加目标管理详情页
    QWidget *goalDetailPage = new QWidget;
    QVBoxLayout *goalDetailLayout = new QVBoxLayout(goalDetailPage);
    goalDetailLayout->setSpacing(16);
    goalDetailLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *btnGoalBack = new QPushButton("← 返回");
    btnGoalBack->setCursor(Qt::PointingHandCursor);
    btnGoalBack->setStyleSheet("QPushButton { background: transparent; color: #3498DB; "
                                "border: none; font-size: 14px; padding: 0; } "
                                "QPushButton:hover { color: #2980B9; }");
    connect(btnGoalBack, &QPushButton::clicked, this, &OtherPage::showFeatureList);
    goalDetailLayout->addWidget(btnGoalBack);

    QLabel *goalDetailTitle = new QLabel("🎯 储蓄目标管理");
    goalDetailTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2C3E50; "
                                    "background: transparent;");
    goalDetailLayout->addWidget(goalDetailTitle);

    QLabel *goalDetailDesc = new QLabel("设定储蓄目标，系统会根据您的收支结余自动追踪进度。");
    goalDetailDesc->setWordWrap(true);
    goalDetailDesc->setStyleSheet("font-size: 13px; color: #555; background: transparent;");
    goalDetailLayout->addWidget(goalDetailDesc);

    // 目标列表
    m_goalListWidget = new QListWidget;
    m_goalListWidget->setStyleSheet("QListWidget::item { padding: 12px 14px; "
                                     "border-bottom: 1px solid #F0F3F7; } "
                                     "QListWidget::item:selected { background: #EBF5FB; }");
    goalDetailLayout->addWidget(m_goalListWidget, 1);

    // 按钮行
    QHBoxLayout *goalBtnRow = new QHBoxLayout;
    QPushButton *btnAddGoal = new QPushButton("➕ 添加目标");
    btnAddGoal->setMinimumHeight(40);
    btnAddGoal->setStyleSheet("QPushButton { background: #9B59B6; color: white; "
                               "border-radius: 8px; padding: 10px 24px; font-size: 14px; } "
                               "QPushButton:hover { background: #8E44AD; }");
    connect(btnAddGoal, &QPushButton::clicked, this, &OtherPage::onAddGoal);

    QPushButton *btnDelGoal = new QPushButton("🗑 删除选中");
    btnDelGoal->setMinimumHeight(40);
    btnDelGoal->setStyleSheet("QPushButton { background: #ECF0F1; color: #E74C3C; "
                               "border: 1px solid #E74C3C; border-radius: 8px; "
                               "padding: 10px 24px; font-size: 14px; } "
                               "QPushButton:hover { background: #FDEDEC; }");
    connect(btnDelGoal, &QPushButton::clicked, this, &OtherPage::onDeleteGoal);

    goalBtnRow->addWidget(btnAddGoal);
    goalBtnRow->addWidget(btnDelGoal);
    goalBtnRow->addStretch();
    goalDetailLayout->addLayout(goalBtnRow);

    ui->featureStack->addWidget(goalDetailPage);  // 索引 3
}

void OtherPage::refresh()
{
    if (m_goalListWidget) loadGoalList();
}

void OtherPage::showFeatureList()   { ui->featureStack->setCurrentIndex(0); }
void OtherPage::showExportDetail()  { ui->featureStack->setCurrentIndex(1); }
void OtherPage::showClearDetail()   { ui->featureStack->setCurrentIndex(2); }

void OtherPage::onExportData()
{
    const auto& records = m_ledger.getAllRecords();
    if (records.empty()) {
        QMessageBox::information(this, "提示", "当前没有任何记账记录，无法导出。");
        return;
    }
    QString content = formatRecords();
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString fileName = QString("记账明细-至%1.txt").arg(today);
    QString desktopPath = getDesktopPath();
    if (desktopPath.isEmpty()) {
        QMessageBox::warning(this, "导出失败", "无法获取桌面文件夹路径。");
        return;
    }
    QString fullPath = desktopPath + "/" + fileName;
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建文件: " + fullPath);
        return;
    }
    QTextStream stream(&file);
    stream.setGenerateByteOrderMark(true);
    stream << content;
    file.close();
    QMessageBox::information(this, "导出成功",
        QString("文件已保存到桌面:\n%1\n共 %2 条记录").arg(fileName).arg(records.size()));
}

QString OtherPage::getDesktopPath() const
{
    QString p = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (!p.isEmpty() && QDir(p).exists()) return QDir::toNativeSeparators(p);
    p = qgetenv("USERPROFILE");
    if (!p.isEmpty() && QDir(p + "/Desktop").exists()) return QDir::toNativeSeparators(p + "/Desktop");
    p = qgetenv("HOME");
    if (!p.isEmpty() && QDir(p + "/Desktop").exists()) return QDir::toNativeSeparators(p + "/Desktop");
    return QString();
}

QString OtherPage::formatRecords() const
{
    const auto& records = m_ledger.getAllRecords();
    QString out;
    out += "══════════ 记账明细导出 ══════════\n";
    out += QString("导出时间: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    out += QString("总收入: +¥%1  总支出: -¥%2  结余: %3¥%4  共 %5 条\n\n")
               .arg(m_ledger.getTotalIncome(), 0, 'f', 2)
               .arg(m_ledger.getTotalExpense(), 0, 'f', 2)
               .arg(m_ledger.getBalance() >= 0 ? "+" : "")
               .arg(m_ledger.getBalance(), 0, 'f', 2)
               .arg(records.size());

    std::set<std::string> dateSet;
    for (const auto& r : records) dateSet.insert(r.date);
    std::vector<std::string> dates(dateSet.rbegin(), dateSet.rend());

    for (const auto& date : dates) {
        std::vector<Record> dayRecs;
        double in = 0, ex = 0;
        for (const auto& r : records) {
            if (r.date == date) {
                dayRecs.push_back(r);
                if (r.type == RecordType::INCOME) in += r.amount; else ex += r.amount;
            }
        }
        out += QString("━━━ %1  收入+¥%2  支出-¥%3  净额%4¥%5  共%6笔 ━━━\n")
                   .arg(QString::fromStdString(date)).arg(in, 0, 'f', 2).arg(ex, 0, 'f', 2)
                   .arg(in - ex >= 0 ? "+" : "").arg(in - ex, 0, 'f', 2).arg(dayRecs.size());
        for (const auto& r : dayRecs) {
            QString amt = (r.type == RecordType::INCOME) ? QString("+%1").arg(r.amount, 0, 'f', 2)
                                                          : QString("-%1").arg(r.amount, 0, 'f', 2);
            out += QString("  %1  %2  %3  %4\n")
                       .arg(QString::fromStdString(typeToChinese(r.type)))
                       .arg(amt).arg(QString::fromStdString(r.category))
                       .arg(QString::fromStdString(r.note));
        }
        out += "\n";
    }
    return out;
}

void OtherPage::onClearData()
{
    auto confirm = [this](int step) -> bool {
        QMessageBox msg(this);
        msg.setIcon(step == 3 ? QMessageBox::Critical : QMessageBox::Warning);
        msg.setWindowTitle(QString("清除数据 - 第%1次确认").arg(step));
        if (step == 1) msg.setText("⚠️ 确定要清除所有记账数据吗？\n此操作不可撤销！");
        else if (step == 2) msg.setText("⚠️ 所有记账记录和自定义分类将被永久删除。\n是否继续？");
        else msg.setText("🚫 这是最后一次确认。\n清除后数据无法恢复！");
        msg.setStandardButtons(QMessageBox::NoButton);
        QPushButton *btnY = msg.addButton(step == 3 ? "确认清除" : "继续", QMessageBox::YesRole);
        msg.addButton("取消", QMessageBox::NoRole);
        btnY->setStyleSheet(QString("QPushButton { background: %1; color: white; border-radius: 6px; padding: 8px 20px; min-width: 90px; %2 }")
                            .arg(step == 3 ? "#C0392B" : "#E74C3C")
                            .arg(step == 3 ? "font-weight: bold;" : ""));
        msg.exec();
        return msg.clickedButton() == btnY;
    };

    if (!confirm(1)) return;
    if (!confirm(2)) return;
    if (!confirm(3)) return;

    const auto& all = m_ledger.getAllRecords();
    for (int i = (int)all.size() - 1; i >= 0; i--)
        m_ledger.deleteRecord(all[i].id);

    QMessageBox::information(this, "完成", "✅ 所有数据已清除。");
}

// ==================== 储蓄目标管理 ====================

void OtherPage::showGoalDetail()
{
    ui->featureStack->setCurrentIndex(3);
    loadGoalList();
}

void OtherPage::loadGoalList()
{
    if (!m_goalListWidget) return;
    m_goalListWidget->clear();

    const auto& goals = m_goalMan.getAllGoals();
    for (const auto& g : goals) {
        double pct = g.progressPercent();
        QString display = QString("%1  |  目标: ¥%2  |  已存: ¥%3  |  进度: %4%  |  截止: %5")
                              .arg(QString::fromStdString(g.name))
                              .arg(g.targetAmount, 0, 'f', 0)
                              .arg(g.currentSaved, 0, 'f', 0)
                              .arg(pct, 0, 'f', 0)
                              .arg(QString::fromStdString(g.deadline));

        if (g.isCompleted()) {
            display += "  ✅";
        } else if (g.monthlyNeeded() > 0) {
            display += QString("  [需月存 ¥%1]").arg(g.monthlyNeeded(), 0, 'f', 0);
        }

        QListWidgetItem *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, g.id);

        if (g.isCompleted()) {
            item->setForeground(QColor("#27AE60"));
        } else if (pct >= 50) {
            item->setForeground(QColor("#3498DB"));
        }
        m_goalListWidget->addItem(item);
    }

    if (goals.empty()) {
        QListWidgetItem *item = new QListWidgetItem(
            QString::fromUtf8("暂无储蓄目标。点击「添加目标」来创建。"));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setForeground(QColor("#95A5A6"));
        m_goalListWidget->addItem(item);
    }
}

void OtherPage::onAddGoal()
{
    bool ok;
    QString name = QInputDialog::getText(this, QString::fromUtf8("添加储蓄目标"),
        QString::fromUtf8("目标名称（如「年底旅行基金」）:"), QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    double target = QInputDialog::getDouble(this, "目标金额",
        "目标金额（元）:", 10000.00, 0.01, 99999999.99, 2, &ok);
    if (!ok || target <= 0) return;

    QString deadline = QInputDialog::getText(this, "截止日期",
        "截止日期（YYYY-MM-DD）:\n例如: 2026-12-31",
        QLineEdit::Normal,
        QDate::currentDate().addYears(1).toString("yyyy-MM-dd"), &ok);
    if (!ok || deadline.trimmed().isEmpty()) return;

    SavingsGoal g;
    g.name = name.trimmed().toStdString();
    g.targetAmount = target;
    g.deadline = deadline.trimmed().toStdString();

    m_goalMan.addGoal(g);
    loadGoalList();

    QMessageBox::information(this, QString::fromUtf8("添加成功"),
        QString::fromUtf8("储蓄目标「%1」已创建。\n目标金额: ¥%2\n截止日期: %3")
            .arg(name.trimmed()).arg(target, 0, 'f', 2).arg(deadline.trimmed()));
}

void OtherPage::onDeleteGoal()
{
    if (!m_goalListWidget) return;
    auto *item = m_goalListWidget->currentItem();
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) {
        QMessageBox::information(this, "提示", "请先选中要删除的目标。");
        return;
    }

    int id = item->data(Qt::UserRole).toInt();
    auto *goal = m_goalMan.findGoal(id);
    if (!goal) return;

    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("确认删除");
    msgBox.setText(QString::fromUtf8("确定要删除储蓄目标「%1」吗？")
                       .arg(QString::fromStdString(goal->name)));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if (msgBox.exec() != QMessageBox::Ok) return;

    m_goalMan.deleteGoal(id);
    loadGoalList();
}
