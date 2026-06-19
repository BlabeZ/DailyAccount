/*
 * ===========================================================================
 * 文件名称: otherpage.cpp
 * 所属模块: gui（图形用户界面层）
 * 功能描述: OtherPage 类的具体实现 —— 数据一键导出到桌面 TXT 文件。
 *
 * 导出文件格式:
 *   - 纯文本文件，UTF-8 编码（带 BOM，确保记事本正确识别中文）
 *   - 包含: 导出时间、总览统计、按日期分组的流水明细
 *   - 每条记录: 时间 | 类型 | 金额 | 分类 | 备注
 *
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "otherpage.h"
#include "ledger.h"
#include "record.h"

// ---- Qt 框架头文件 ----
#include <QVBoxLayout>       // 垂直布局
#include <QHBoxLayout>       // 水平布局
#include <QFrame>            // 框架容器（卡片样式）
#include <QLabel>            // 文本标签
#include <QPushButton>       // 按钮
#include <QMessageBox>       // 消息对话框（成功/失败提示）
#include <QFile>             // 文件读写
#include <QTextStream>       // 文本流（写入文件）
#include <QDate>             // 日期处理（获取当前日期）
#include <QStandardPaths>    // 标准路径（获取桌面文件夹）
#include <QDir>              // 目录操作
#include <QFileInfo>         // 文件信息
#include <QDateTime>         // 日期时间（导出时间戳）
#include <QApplication>      // 应用程序（用于剪贴板等）
#include <QStyle>            // 样式

// ============================================================================
// 构造函数: OtherPage
// ============================================================================
OtherPage::OtherPage(Ledger& ledger, QWidget *parent)
    : QWidget(parent), m_ledger(ledger)
{
    setupUI();
}

// ============================================================================
// 方法: setupUI —— 构建页面界面
// ----------------------------------------------------------------------------
// 页面布局（从上到下）:
//   ┌──────────────────────────────────────────┐
//   │  🔧 其他功能                              │
//   │                                          │
//   │  ┌────────────────────────────────────┐  │
//   │  │  📤 数据导出                         │  │
//   │  │                                     │  │
//   │  │  将所有记账记录导出为一个TXT文本文    │  │
//   │  │  件，保存在桌面。文件名格式：         │  │
//   │  │  记账明细-至2026-06-19.txt           │  │
//   │  │                                     │  │
//   │  │  [📤 导出全部数据到桌面]             │  │
//   │  │                                     │  │
//   │  │  ✅ 提示信息显示区域                  │  │
//   │  └────────────────────────────────────┘  │
//   │                                          │
//   └──────────────────────────────────────────┘
// ============================================================================
void OtherPage::setupUI()
{
    // ---- 主布局 ----
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    // ---- 页面标题 ----
    QLabel *pageTitle = new QLabel("🔧 其他功能");
    pageTitle->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #2C3E50; "
        "background: transparent; padding: 0;");
    mainLayout->addWidget(pageTitle);

    // ---- 数据导出卡片 ----
    QFrame *exportCard = new QFrame;
    exportCard->setProperty("class", "card");
    exportCard->setStyleSheet(
        "QFrame { background: white; border-radius: 10px; "
        "border: 1px solid #E8ECF1; padding: 24px; }");

    QVBoxLayout *cardLayout = new QVBoxLayout(exportCard);
    cardLayout->setSpacing(16);

    // 卡片内标题
    QLabel *cardTitle = new QLabel("📤 数据导出");
    cardTitle->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #2C3E50; "
        "background: transparent;");
    cardLayout->addWidget(cardTitle);

    // 说明文字
    QLabel *description = new QLabel(
        "将所有记账记录导出为一个 TXT 文本文档，保存在桌面。\n"
        "您可以用记事本打开查看、打印，或分享给他人。\n\n"
        "文件命名格式: <b>记账明细-至2026-06-19.txt</b>\n"
        "（日期部分自动取当前日期）\n\n"
        "导出内容包含:\n"
        "  • 导出时间戳\n"
        "  • 总收入 / 总支出 / 结余 汇总\n"
        "  • 按日期分组的全部流水明细\n"
        "  • 每日小计（收入、支出、净额）");
    description->setStyleSheet(
        "font-size: 13px; color: #555; background: transparent; "
        "line-height: 1.6;");
    description->setWordWrap(true);
    cardLayout->addWidget(description);

    // 导出按钮
    m_exportBtn = new QPushButton("📤 导出全部数据到桌面");
    m_exportBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #27AE60;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 14px 32px;"
        "  font-size: 15px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #219A52;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1E8449;"
        "}");
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setMinimumHeight(48);
    cardLayout->addWidget(m_exportBtn);

    // ---- 连接信号 ----
    // 点击导出按钮 → 执行 onExportData() 槽函数
    connect(m_exportBtn, &QPushButton::clicked,
            this, &OtherPage::onExportData);

    // ---- 状态提示标签（初始隐藏）----
    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet(
        "font-size: 13px; padding: 10px 16px; border-radius: 6px; "
        "background: transparent;");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);
    cardLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(exportCard);

    // ---- 清除数据卡片 ----
    QFrame *clearCard = new QFrame;
    clearCard->setProperty("class", "card");
    clearCard->setStyleSheet(
        "QFrame { background: white; border-radius: 10px; "
        "border: 1px solid #E8ECF1; padding: 24px; }");

    QVBoxLayout *clearLayout = new QVBoxLayout(clearCard);
    clearLayout->setSpacing(16);

    QLabel *clearTitle = new QLabel("🗑️ 清除数据");
    clearTitle->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #E74C3C; "
        "background: transparent;");
    clearLayout->addWidget(clearTitle);

    QLabel *clearDesc = new QLabel(
        "永久删除所有记账记录和自定义分类。\n"
        "此操作不可撤销，建议先导出数据备份。");
    clearDesc->setStyleSheet(
        "font-size: 13px; color: #555; background: transparent; line-height: 1.6;");
    clearDesc->setWordWrap(true);
    clearLayout->addWidget(clearDesc);

    QPushButton *clearBtn = new QPushButton("🗑️ 清除全部数据");
    clearBtn->setStyleSheet(
        "QPushButton { background: #E74C3C; color: white; border: none; "
        "border-radius: 8px; padding: 14px 32px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background: #CB4335; }");
    clearBtn->setCursor(Qt::PointingHandCursor);
    clearBtn->setMinimumHeight(48);
    clearLayout->addWidget(clearBtn);
    connect(clearBtn, &QPushButton::clicked, this, &OtherPage::onClearData);

    mainLayout->addWidget(clearCard);

    // ---- 底部留白 ----
    mainLayout->addStretch();
}

// ============================================================================
// 方法: onClearData —— 三次确认后清除全部数据
// ============================================================================
void OtherPage::onClearData()
{
    // 第一次确认
    QMessageBox msg1(this);
    msg1.setWindowTitle("清除数据 - 第1次确认");
    msg1.setText("⚠️ 确定要清除所有记账数据吗？\n此操作不可撤销！");
    msg1.setInformativeText("建议先使用数据导出功能备份数据。");
    msg1.setIcon(QMessageBox::Warning);
    msg1.setStandardButtons(QMessageBox::NoButton);
    QPushButton *btn1Yes = msg1.addButton("继续", QMessageBox::YesRole);
    QPushButton *btn1No  = msg1.addButton("取消", QMessageBox::NoRole);
    btn1Yes->setStyleSheet("QPushButton { background: #E74C3C; color: white; "
                           "border-radius: 6px; padding: 8px 20px; min-width: 90px; }");
    msg1.setDefaultButton(btn1No);
    msg1.exec();
    if (msg1.clickedButton() != btn1Yes) return;

    // 第二次确认
    QMessageBox msg2(this);
    msg2.setWindowTitle("清除数据 - 第2次确认");
    msg2.setText("⚠️ 所有记账记录和自定义分类将被永久删除。\n是否继续？");
    msg2.setIcon(QMessageBox::Warning);
    msg2.setStandardButtons(QMessageBox::NoButton);
    QPushButton *btn2Yes = msg2.addButton("继续", QMessageBox::YesRole);
    QPushButton *btn2No  = msg2.addButton("取消", QMessageBox::NoRole);
    btn2Yes->setStyleSheet("QPushButton { background: #E74C3C; color: white; "
                           "border-radius: 6px; padding: 8px 20px; min-width: 90px; }");
    msg2.setDefaultButton(btn2No);
    msg2.exec();
    if (msg2.clickedButton() != btn2Yes) return;

    // 第三次确认
    QMessageBox msg3(this);
    msg3.setWindowTitle("清除数据 - 最后确认");
    msg3.setText("🚫 这是最后一次确认。\n清除后数据无法恢复！");
    msg3.setIcon(QMessageBox::Critical);
    msg3.setStandardButtons(QMessageBox::NoButton);
    QPushButton *btn3Yes = msg3.addButton("确认清除", QMessageBox::YesRole);
    QPushButton *btn3No  = msg3.addButton("取消", QMessageBox::NoRole);
    btn3Yes->setStyleSheet("QPushButton { background: #C0392B; color: white; "
                           "border-radius: 6px; padding: 8px 20px; min-width: 90px; "
                           "font-weight: bold; }");
    msg3.setDefaultButton(btn3No);
    msg3.exec();
    if (msg3.clickedButton() != btn3Yes) return;

    // 执行清除：删除所有记录
    const auto& all = m_ledger.getAllRecords();
    // 从后往前删除避免索引问题
    for (int i = (int)all.size() - 1; i >= 0; i--) {
        m_ledger.deleteRecord(all[i].id);
    }

    QMessageBox::information(this, "完成", "✅ 所有数据已清除。");
}

// ============================================================================
// 方法: refresh —— 刷新页面（预留接口，当前页面为静态功能页）
// ============================================================================
void OtherPage::refresh()
{
    // 本页面当前无动态数据需要刷新，预留接口供未来扩展
}

// ============================================================================
// 方法: onExportData —— 执行数据导出的核心逻辑
// ----------------------------------------------------------------------------
// 执行步骤（共 7 步）:
//   1. 获取全部流水记录，检查是否为空
//   2. 格式化流水数据为可读文本
//   3. 生成文件名（含当前日期）
//   4. 获取桌面路径
//   5. 写入文件（UTF-8 编码，带 BOM）
//   6. 显示结果提示
//   7. 更新状态标签
// ============================================================================
void OtherPage::onExportData()
{
    // ---- 步骤1: 获取数据并检查 ----
    const auto& transactions = m_ledger.getAllRecords();
    if (transactions.empty()) {
        QMessageBox::information(this, "提示",
            "当前没有任何记账记录，无法导出。\n\n请先添加一些收支记录后再试。");
        return;
    }

    // ---- 步骤2: 格式化数据 ----
    QString content = formatRecords();

    // ---- 步骤3: 生成文件名 ----
    // 格式: "记账明细-至YYYY-MM-DD.txt"
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString fileName = QString("记账明细-至%1.txt").arg(today);

    // ---- 步骤4: 获取桌面路径 ----
    QString desktopPath = getDesktopPath();
    if (desktopPath.isEmpty()) {
        QMessageBox::warning(this, "导出失败",
            "无法获取桌面文件夹路径，导出失败。\n\n"
            "请检查系统环境变量是否正常。");
        return;
    }

    QString fullPath = desktopPath + "/" + fileName;

    // ---- 步骤5: 写入文件 ----
    // 使用 QFile + QTextStream，设置 UTF-8 编码
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败",
            QString("无法创建文件: %1\n\n"
                    "请检查磁盘空间是否充足，或目标位置是否有写入权限。")
                .arg(fullPath));
        return;
    }

    // 设置 UTF-8 编码并写入 BOM（Byte Order Mark）
    // BOM = \xEF\xBB\xBF，帮助 Windows 记事本正确识别 UTF-8 编码
    QTextStream stream(&file);
    // Qt 6 默认使用 UTF-8 编码，无需 setCodec()
    stream.setGenerateByteOrderMark(true);  // 写入 UTF-8 BOM（记事本识别用）
    stream << content;
    file.close();

    // ---- 步骤6: 显示成功提示 ----
    QMessageBox::information(this, "导出成功",
        QString("✅ 数据已成功导出！\n\n"
                "文件位置: %1\n"
                "文件名称: %2\n"
                "记录总数: %3 条\n\n"
                "您可以用记事本或其他文本编辑工具打开查看。")
            .arg(desktopPath)
            .arg(fileName)
            .arg(transactions.size()));

    // ---- 步骤7: 更新页面状态标签 ----
    m_statusLabel->setText(
        QString("✅ 上次导出: %1 → %2/%3 (共 %4 条记录)")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(desktopPath)
            .arg(fileName)
            .arg(transactions.size()));
    m_statusLabel->setStyleSheet(
        "font-size: 13px; padding: 10px 16px; border-radius: 6px; "
        "background: #EAFAF1; color: #27AE60; font-weight: bold;");
    m_statusLabel->setVisible(true);
}

// ============================================================================
// 方法: getDesktopPath —— 获取桌面文件夹绝对路径
// ----------------------------------------------------------------------------
// 获取策略（按优先级尝试）:
//   1. Qt 标准路径: QStandardPaths::DesktopLocation
//   2. 环境变量: USERPROFILE + "\\Desktop"（Windows）
//   3. 环境变量: HOME + "/Desktop"（Linux / macOS）
//
// 返回值: 桌面文件夹的绝对路径字符串；获取失败返回空字符串
// ============================================================================
QString OtherPage::getDesktopPath() const
{
    // ---- 方法1: 使用 Qt 标准路径 API（跨平台）----
    QString desktopPath = QStandardPaths::writableLocation(
        QStandardPaths::DesktopLocation);

    // 如果 Qt 返回了有效路径且目录存在，直接使用
    if (!desktopPath.isEmpty()) {
        QDir dir(desktopPath);
        if (dir.exists()) {
            return QDir::toNativeSeparators(desktopPath);
        }
    }

    // ---- 方法2: 使用环境变量（Windows）----
    // 当 Qt 无法获取时（某些 Windows 配置），回退到环境变量
    QString homePath = qgetenv("USERPROFILE");
    if (!homePath.isEmpty()) {
        QString winDesktop = homePath + "/Desktop";
        QDir dir(winDesktop);
        if (dir.exists()) {
            return QDir::toNativeSeparators(winDesktop);
        }
    }

    // ---- 方法3: Linux / macOS 回退 ----
    homePath = qgetenv("HOME");
    if (!homePath.isEmpty()) {
        QString linuxDesktop = homePath + "/Desktop";
        QDir dir(linuxDesktop);
        if (dir.exists()) {
            return QDir::toNativeSeparators(linuxDesktop);
        }
    }

    // ---- 所有方法都失败 ----
    return QString();
}

// ============================================================================
// 方法: formatRecords —— 格式化全部流水记录为可读文本
// ----------------------------------------------------------------------------
// 输出文本结构:
//   ┌─ 文件头（标题 + 导出时间）
//   ├─ 总体汇总（总收入 / 总支出 / 结余 / 总笔数）
//   ├─ 分隔线
//   ├─ 按日期分组:
//   │   ├─ 日期标题（日期 + 当日小计）
//   │   ├─ 表头（时间 | 类型 | 金额 | 分类 | 备注）
//   │   ├─ 每条流水记录一行
//   │   └─ 空行
//   └─ 文件尾
//
// 返回值: 完整的格式化文本（QString，UTF-8）
// ============================================================================
QString OtherPage::formatRecords() const
{
    const auto& transactions = m_ledger.getAllRecords();
    QString out;

    // ---- 计算列宽 ----
    // 确保中英文混排时表格对齐（等宽字体下有效）
    const int TYPE_W     = 6;   // 类型列宽度（"收入"/"支出"）
    const int AMOUNT_W   = 12;  // 金额列宽度
    const int CATEGORY_W = 12;  // 分类列宽度

    // =========================================================================
    // 文件头
    // =========================================================================
    out += "╔══════════════════════════════════════════════════════╗\n";
    out += "║              📒 记账明细 - 数据导出                    ║\n";
    out += "╠══════════════════════════════════════════════════════╣\n";
    out += QString("║  导出时间: %1                         ║\n")
               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    out += "╚══════════════════════════════════════════════════════╝\n";
    out += "\n";

    // =========================================================================
    // 总体汇总
    // =========================================================================
    double totalIncome  = m_ledger.getTotalIncome();
    double totalExpense = m_ledger.getTotalExpense();
    double balance      = m_ledger.getBalance();

    out += "━━━━━━━━━━━━━━━ 总体汇总 ━━━━━━━━━━━━━━━\n";
    out += QString("  总收入:  + ¥ %1\n").arg(totalIncome, 0, 'f', 2);
    out += QString("  总支出:  - ¥ %1\n").arg(totalExpense, 0, 'f', 2);
    out += QString("  结  余:  %1 ¥ %2\n")
               .arg(balance >= 0 ? "+" : "")
               .arg(balance, 0, 'f', 2);
    out += QString("  总笔数:  %1 条\n").arg(transactions.size());
    out += "\n";

    // =========================================================================
    // 按日期分组输出流水明细
    // =========================================================================
    // 第一步: 收集所有日期并降序排列
    std::set<std::string> dateSet;
    for (const auto& t : transactions) {
        dateSet.insert(t.date);
    }
    std::vector<std::string> dates(dateSet.rbegin(), dateSet.rend());

    // 第二步: 按日期逐日输出
    for (const auto& date : dates) {
        // ---- 收集当天的流水并按时序排列 ----
        std::vector<Record> dayTxns;
        for (const auto& t : transactions) {
            if (t.date == date) {
                dayTxns.push_back(t);
            }
        }

        // ---- 计算当日小计 ----
        double dayIncome = 0, dayExpense = 0;
        for (const auto& t : dayTxns) {
            if (t.type == RecordType::INCOME)
                dayIncome += t.amount;
            else
                dayExpense += t.amount;
        }

        // ---- 日期标题行 ----
        out += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        out += QString("  📅 %1    收入: +¥%2    支出: -¥%3    净额: %4¥%5    共 %6 笔\n")
                   .arg(QString::fromStdString(date))
                   .arg(dayIncome, 0, 'f', 2)
                   .arg(dayExpense, 0, 'f', 2)
                   .arg(dayIncome - dayExpense >= 0 ? "+" : "")
                   .arg(dayIncome - dayExpense, 0, 'f', 2)
                   .arg(dayTxns.size());
        out += "──────────────────────────────────────────────────────\n";

        // ---- 表头 ----
        out += QString("  %1  %2  %3  %4\n")
                   .arg(QString("类型").leftJustified(TYPE_W, ' '))
                   .arg(QString("金额").rightJustified(AMOUNT_W, ' '))
                   .arg(QString("分类").leftJustified(CATEGORY_W, ' '))
                   .arg("备注");
        out += "  --------------------------------------------------\n";

        // ---- 逐笔输出 ----
        for (const auto& t : dayTxns) {
            // 金额格式: 收入显示 +xx.xx，支出显示 -xx.xx
            QString amtStr;
            if (t.type == RecordType::INCOME) {
                amtStr = QString("+%1").arg(t.amount, 0, 'f', 2);
            } else {
                amtStr = QString("-%1").arg(t.amount, 0, 'f', 2);
            }

            out += QString("  %1  %2  %3  %4\n")
                       .arg(QString::fromStdString(typeToChinese(t.type))
                                .leftJustified(TYPE_W, ' '))
                       .arg(amtStr.rightJustified(AMOUNT_W, ' '))
                       .arg(QString::fromStdString(t.category)
                                .leftJustified(CATEGORY_W, ' '))
                       .arg(QString::fromStdString(t.note));
        }

        out += "\n";  // 日期之间空一行
    }

    // =========================================================================
    // 文件尾
    // =========================================================================
    out += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    out += "  数据导出完毕。感谢使用记账工具！\n";
    out += QString("  导出时间: %1\n")
               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    return out;
}
