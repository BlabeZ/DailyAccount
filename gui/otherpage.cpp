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
#include "record.h"
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
    : QWidget(parent)
    , m_ledger(ledger)
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
}

void OtherPage::refresh() {}

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
