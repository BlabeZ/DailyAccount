/*
 * ============================================================================
 * 文件名:   flowdialog.cpp
 * 所属模块: GUI - 流水添加/编辑对话框（实现文件）
 * 功    能: FlowDialog 类的完整实现。包含：
 *           1. 双模式构造函数（添加模式 / 编辑模式）
 *           2. 界面搭建（setupUI）—— QFormLayout 表单布局
 *           3. 分类联动（onTypeChanged / populateCategories）
 *           4. 表单数据读取与填充（getRecord / setRecord）
 *           5. 提交前验证（onAccept）
 * 编    码: UTF-8
 * 说    明: 本对话框是用户与系统交互最频繁的界面之一。每次添加或修改
 *           流水记录都会使用它。因此代码中特别注重了数据验证和用户体验。
 * ============================================================================
 */

// ---------- 本模块头文件 ----------
#include "flowdialog.h"

// ---------- 项目内部模块 ----------
#include "category.h"       // CategoryManager 类、RecordType 枚举等

// ---------- Qt 框架头文件 ----------
#include <QFormLayout>      // 【表单布局】专门为"标签-输入框"成对排列设计的布局。
#include <QVBoxLayout>      // 【垂直布局】从上到下排列控件
#include <QHBoxLayout>      // 【水平布局】从左到右排列控件
#include <QPushButton>      // 【按钮控件】取消、确定按钮
#include <QButtonGroup>     // 【按钮组】将多个单选按钮归为一组，确保互斥选择
#include <QMessageBox>      // 【消息框】弹出警告和提示信息
#include <QDate>            // 【日期类】获取当前日期
#include <QLabel>           // 【标签控件】显示标题文字


// ============================================================================
// 构  造  函  数  ——  添  加  模  式
// ============================================================================
FlowDialog::FlowDialog(CategoryManager& catMan, QWidget *parent)
    : QDialog(parent)
    , m_catMan(catMan)
    , m_editId(-1)
{
    setWindowTitle(" 添加记录");
    setupUI();
    setRecord(Record{});
}


// ============================================================================
// 构  造  函  数  ——  编  辑  模  式
// ============================================================================
FlowDialog::FlowDialog(CategoryManager& catMan, const Record& existing,
                                     QWidget *parent)
    : QDialog(parent)
    , m_catMan(catMan)
    , m_editId(existing.id)
{
    setWindowTitle(" 修改记录");
    setupUI();
    setRecord(existing);
}


// ============================================================================
// 界  面  搭  建  函  数
// ============================================================================
void FlowDialog::setupUI()
{
    setMinimumWidth(420);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // 标题
    QLabel *title = new QLabel(windowTitle());
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2C3E50; "
                         "background: transparent;");
    mainLayout->addWidget(title);

    // 表单区域
    QFormLayout *form = new QFormLayout;
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 日期字段
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    form->addRow("日期:", m_dateEdit);

    // 类型字段（单选按钮组）
    m_radioExpense = new QRadioButton("支出");
    m_radioIncome  = new QRadioButton("收入");
    m_radioExpense->setChecked(true);

    QButtonGroup *typeGroup = new QButtonGroup(this);
    typeGroup->addButton(m_radioExpense);
    typeGroup->addButton(m_radioIncome);

    QHBoxLayout *typeLayout = new QHBoxLayout;
    typeLayout->addWidget(m_radioExpense);
    typeLayout->addWidget(m_radioIncome);
    typeLayout->addStretch();
    form->addRow("类型:", typeLayout);

    connect(m_radioExpense, &QRadioButton::toggled, this, &FlowDialog::onTypeChanged);
    connect(m_radioIncome,  &QRadioButton::toggled, this, &FlowDialog::onTypeChanged);

    // 金额字段
    m_amountSpin = new QDoubleSpinBox;
    m_amountSpin->setRange(0.01, 99999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setPrefix(" ");
    m_amountSpin->setValue(0.01);
    m_amountSpin->setMinimumWidth(200);
    form->addRow("金额:", m_amountSpin);

    // 分类字段
    m_categoryCombo = new QComboBox;
    m_categoryCombo->setMinimumWidth(200);
    form->addRow("分类:", m_categoryCombo);

    // 备注字段
    m_noteEdit = new QLineEdit;
    m_noteEdit->setPlaceholderText("可选：添加备注信息...");
    form->addRow("备注:", m_noteEdit);

    mainLayout->addLayout(form);
    mainLayout->addSpacing(8);

    // 底部按钮栏
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    QPushButton *cancelBtn = new QPushButton("取消");
    cancelBtn->setStyleSheet(
        "QPushButton { background: #ECF0F1; color: #2C3E50; border-radius: 6px; "
        "padding: 10px 24px; } QPushButton:hover { background: #D5DCE6; }");
    btnLayout->addWidget(cancelBtn);

    QPushButton *okBtn = new QPushButton("确定");
    okBtn->setStyleSheet(
        "QPushButton { background: #3498DB; color: white; border-radius: 6px; "
        "padding: 10px 32px; } QPushButton:hover { background: #2980B9; }");
    btnLayout->addWidget(okBtn);

    mainLayout->addLayout(btnLayout);

    // 信号连接
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, this, &FlowDialog::onAccept);

    // 初始化分类列表
    populateCategories(RecordType::EXPENSE);
}


// ============================================================================
// 分  类  联  动
// ============================================================================
void FlowDialog::onTypeChanged()
{
    RecordType type = m_radioIncome->isChecked()
        ? RecordType::INCOME
        : RecordType::EXPENSE;
    populateCategories(type);
}


// ============================================================================
// 填  充  分  类  列  表
// ============================================================================
void FlowDialog::populateCategories(RecordType type)
{
    m_categoryCombo->clear();
    auto cats = m_catMan.getCategories(type);
    for (const auto& c : cats) {
        m_categoryCombo->addItem(QString::fromStdString(c));
    }
}


// ============================================================================
// 表  单  数  据  填  充
// ============================================================================
void FlowDialog::setRecord(const Record& t)
{
    if (!t.date.empty()) {
        m_dateEdit->setDate(QDate::fromString(
            QString::fromStdString(t.date), "yyyy-MM-dd"));
    }

    if (t.type == RecordType::INCOME) {
        m_radioIncome->setChecked(true);
    } else {
        m_radioExpense->setChecked(true);
    }

    populateCategories(t.type);

    m_amountSpin->setValue(t.amount > 0 ? t.amount : 0.01);

    int idx = m_categoryCombo->findText(QString::fromStdString(t.category));
    if (idx >= 0) {
        m_categoryCombo->setCurrentIndex(idx);
    }

    m_noteEdit->setText(QString::fromStdString(t.note));
}


// ============================================================================
// 读  取  表  单  数  据
// ============================================================================
Record FlowDialog::getRecord() const
{
    Record t;
    t.id = m_editId;
    t.date = m_dateEdit->date().toString("yyyy-MM-dd").toStdString();
    t.type = m_radioIncome->isChecked() ? RecordType::INCOME : RecordType::EXPENSE;
    t.amount = m_amountSpin->value();
    t.category = m_categoryCombo->currentText().toStdString();
    t.note = m_noteEdit->text().toStdString();
    return t;
}


// ============================================================================
// 提  交  验  证
// ============================================================================
void FlowDialog::onAccept()
{
    if (m_amountSpin->value() <= 0) {
        QMessageBox::warning(this, "输入错误",
                             "请输入有效的金额（大于0）。");
        m_amountSpin->setFocus();
        return;
    }

    if (m_categoryCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, "输入错误",
                             "请选择一个分类。");
        return;
    }

    accept();
}
