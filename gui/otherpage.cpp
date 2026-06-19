/*
 * ===========================================================================
 * 文件名称: otherpage.cpp
 * 所属模块: gui
 * 功能描述: OtherPage 实现 —— 功能入口列表 + 二级详情页。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "otherpage.h"
#include "ledger.h"
#include "record.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <set>
#include <vector>

OtherPage::OtherPage(Ledger& ledger, QWidget *parent)
    : QWidget(parent), m_ledger(ledger)
{
    setupUI();
}

void OtherPage::refresh() {}

void OtherPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *title = new QLabel("🔧 其他功能");
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #2C3E50; background: transparent;");
    mainLayout->addWidget(title);
    mainLayout->addSpacing(16);

    m_stack = new QStackedWidget;
    m_stack->addWidget(createFeatureListPage());   // 0
    m_stack->addWidget(createExportDetailPage());   // 1
    m_stack->addWidget(createClearDetailPage());    // 2
    m_stack->setCurrentIndex(0);
    mainLayout->addWidget(m_stack, 1);
}

// ========== 首页：功能入口列表 ==========
QWidget* OtherPage::createFeatureListPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(12);

    // 导出数据入口
    QFrame *card1 = new QFrame;
    card1->setProperty("class", "card");
    card1->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #E8ECF1; padding: 20px; }");
    QHBoxLayout *row1 = new QHBoxLayout(card1);
    QLabel *icon1 = new QLabel("📤");
    icon1->setStyleSheet("font-size: 28px; background: transparent;");
    row1->addWidget(icon1);
    QVBoxLayout *text1 = new QVBoxLayout;
    QLabel *t1 = new QLabel("数据导出");
    t1->setStyleSheet("font-size: 16px; font-weight: bold; background: transparent;");
    QLabel *d1 = new QLabel("将所有记录导出为TXT文件保存到桌面");
    d1->setStyleSheet("font-size: 12px; color: #7F8C8D; background: transparent;");
    text1->addWidget(t1);
    text1->addWidget(d1);
    row1->addLayout(text1, 1);
    QPushButton *btn1 = new QPushButton("进入 →");
    btn1->setStyleSheet("QPushButton { background: #3498DB; color: white; border-radius: 6px; padding: 10px 20px; } QPushButton:hover { background: #2980B9; }");
    btn1->setCursor(Qt::PointingHandCursor);
    connect(btn1, &QPushButton::clicked, this, &OtherPage::showExportDetail);
    row1->addWidget(btn1);
    layout->addWidget(card1);

    // 清除数据入口
    QFrame *card2 = new QFrame;
    card2->setProperty("class", "card");
    card2->setStyleSheet("QFrame { background: white; border-radius: 10px; border: 1px solid #E8ECF1; padding: 20px; }");
    QHBoxLayout *row2 = new QHBoxLayout(card2);
    QLabel *icon2 = new QLabel("🗑️");
    icon2->setStyleSheet("font-size: 28px; background: transparent;");
    row2->addWidget(icon2);
    QVBoxLayout *text2 = new QVBoxLayout;
    QLabel *t2 = new QLabel("清除数据");
    t2->setStyleSheet("font-size: 16px; font-weight: bold; color: #E74C3C; background: transparent;");
    QLabel *d2 = new QLabel("永久删除所有记录和自定义分类");
    d2->setStyleSheet("font-size: 12px; color: #7F8C8D; background: transparent;");
    text2->addWidget(t2);
    text2->addWidget(d2);
    row2->addLayout(text2, 1);
    QPushButton *btn2 = new QPushButton("进入 →");
    btn2->setStyleSheet("QPushButton { background: #E74C3C; color: white; border-radius: 6px; padding: 10px 20px; } QPushButton:hover { background: #CB4335; }");
    btn2->setCursor(Qt::PointingHandCursor);
    connect(btn2, &QPushButton::clicked, this, &OtherPage::showClearDetail);
    row2->addWidget(btn2);
    layout->addWidget(card2);

    layout->addStretch();
    return page;
}

// ========== 二级页：数据导出 ==========
QWidget* OtherPage::createExportDetailPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(16);

    // 返回按钮
    QPushButton *backBtn = new QPushButton("← 返回");
    backBtn->setStyleSheet("QPushButton { background: transparent; color: #3498DB; border: none; font-size: 14px; padding: 0; } QPushButton:hover { color: #2980B9; }");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &OtherPage::showFeatureList);
    layout->addWidget(backBtn);

    QLabel *title = new QLabel("📤 数据导出");
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #2C3E50; background: transparent;");
    layout->addWidget(title);

    QLabel *desc = new QLabel(
        "将所有记账记录导出为一个 TXT 文本文档，保存在桌面。\n"
        "您可以用记事本打开查看、打印，或分享给他人。\n\n"
        "文件命名格式: 记账明细-至2026-06-19.txt（日期自动取当前）\n\n"
        "导出内容包含:\n"
        "  • 导出时间戳\n"
        "  • 总收入 / 总支出 / 结余 汇总\n"
        "  • 按日期分组的全部流水明细\n"
        "  • 每日小计（收入、支出、净额）");
    desc->setStyleSheet("font-size: 13px; color: #555; background: transparent; line-height: 1.6;");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    QPushButton *exportBtn = new QPushButton("📤 导出全部数据到桌面");
    exportBtn->setStyleSheet(
        "QPushButton { background: #27AE60; color: white; border: none; border-radius: 8px; "
        "padding: 14px 32px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background: #219A52; }");
    exportBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setMinimumHeight(48);
    connect(exportBtn, &QPushButton::clicked, this, &OtherPage::onExportData);
    layout->addWidget(exportBtn);

    layout->addStretch();
    return page;
}

// ========== 二级页：清除数据 ==========
QWidget* OtherPage::createClearDetailPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(16);

    // 返回按钮
    QPushButton *backBtn = new QPushButton("← 返回");
    backBtn->setStyleSheet("QPushButton { background: transparent; color: #3498DB; border: none; font-size: 14px; padding: 0; } QPushButton:hover { color: #2980B9; }");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &OtherPage::showFeatureList);
    layout->addWidget(backBtn);

    QLabel *title = new QLabel("🗑️ 清除数据");
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #E74C3C; background: transparent;");
    layout->addWidget(title);

    QLabel *desc = new QLabel(
        "永久删除所有记账记录和自定义分类。\n\n"
        "⚠️ 此操作不可撤销！\n"
        "⚠️ 建议先使用「数据导出」功能备份数据。\n"
        "⚠️ 需要经过三次确认才会执行删除。");
    desc->setStyleSheet("font-size: 13px; color: #555; background: transparent; line-height: 1.6;");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    QPushButton *clearBtn = new QPushButton("🗑️ 清除全部数据");
    clearBtn->setStyleSheet(
        "QPushButton { background: #E74C3C; color: white; border: none; border-radius: 8px; "
        "padding: 14px 32px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background: #CB4335; }");
    clearBtn->setCursor(Qt::PointingHandCursor);
    clearBtn->setMinimumHeight(48);
    connect(clearBtn, &QPushButton::clicked, this, &OtherPage::onClearData);
    layout->addWidget(clearBtn);

    layout->addStretch();
    return page;
}

// ========== 导航槽函数 ==========
void OtherPage::showFeatureList()   { m_stack->setCurrentIndex(0); }
void OtherPage::showExportDetail()  { m_stack->setCurrentIndex(1); }
void OtherPage::showClearDetail()   { m_stack->setCurrentIndex(2); }

// ========== 导出逻辑（不变）==========
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

// ========== 清除数据逻辑（三次确认，不变）==========
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
