/*
 * ============================================================================
 * 文件名:   flowdialog.cpp
 * 所属模块: GUI - 流水添加/编辑对话框（实现文件）
 * 功    能: FlowDialog 类的完整实现。界面由 flowdialog.ui 定义。
 * 编    码: UTF-8
 * ============================================================================
 */

#include "flowdialog.h"
#include "category.h"
#include <QButtonGroup>
#include <QMessageBox>
#include <QDate>
#include <QCalendarWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

// ==================== 添加模式构造函数 ====================
FlowDialog::FlowDialog(CategoryManager& catMan, QWidget *parent)
    : QDialog(parent)
    , m_catMan(catMan)
    , m_editId(-1)
    , ui(new Ui::FlowDialog)
{
    setWindowTitle(" 添加记录");
    setupUI();
    setRecord(Record{});
    ui->dateEdit->setDate(QDate::currentDate());  // 默认日期设为今天
    ui->categoryCombo->setCurrentIndex(-1);
    ui->subCategoryCombo->setVisible(false);
    ui->subCategoryLabel->setVisible(false);
    ui->amountSpin->setValue(0.00);
}

// ==================== 编辑模式构造函数 ====================
FlowDialog::FlowDialog(CategoryManager& catMan, const Record& existing, QWidget *parent)
    : QDialog(parent)
    , m_catMan(catMan)
    , m_editId(existing.id)
    , ui(new Ui::FlowDialog)
{
    setWindowTitle(" 修改记录");
    setupUI();
    setRecord(existing);
}

// ==================== 界面搭建 ====================
void FlowDialog::setupUI()
{
    ui->setupUi(this);

    // 标题文字
    ui->titleLabel->setText(windowTitle());

    // 单选按钮组（互斥）
    QButtonGroup *typeGroup = new QButtonGroup(this);
    typeGroup->addButton(ui->radioExpense);
    typeGroup->addButton(ui->radioIncome);

    connect(ui->radioExpense, &QRadioButton::toggled, this, &FlowDialog::onTypeChanged);
    connect(ui->radioIncome,  &QRadioButton::toggled, this, &FlowDialog::onTypeChanged);

    // 日历按钮
    connect(ui->calendarBtn, &QPushButton::clicked, this, [this]() {
        QDate d = ui->dateEdit->date();
        QDialog calDlg(this);
        calDlg.setWindowTitle("选择日期");
        QVBoxLayout *calLayout = new QVBoxLayout(&calDlg);
        QCalendarWidget *cal = new QCalendarWidget;
        cal->setSelectedDate(d);
        calLayout->addWidget(cal);
        QHBoxLayout *calBtns = new QHBoxLayout;
        calBtns->addStretch();
        QPushButton *ok = new QPushButton("确定");
        ok->setStyleSheet("QPushButton { background: #3498DB; color: white; "
                          "border-radius: 6px; padding: 8px 24px; }");
        calBtns->addWidget(ok);
        calLayout->addLayout(calBtns);
        connect(ok, &QPushButton::clicked, &calDlg, &QDialog::accept);
        connect(cal, &QCalendarWidget::clicked, &calDlg, &QDialog::accept);
        if (calDlg.exec() == QDialog::Accepted)
            ui->dateEdit->setDate(cal->selectedDate());
    });

    // 分类选择联动子分类
    connect(ui->categoryCombo, QOverload<int>::of(&QComboBox::activated),
            this, [this](int) { updateSubCategory(); });

    // 金额字段设置
    ui->amountSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->amountSpin->setRange(0.00, 99999999.99);
    ui->amountSpin->setDecimals(2);
    ui->amountSpin->setSpecialValueText(" ");

    // 删除按钮（添加模式下隐藏）
    if (m_editId < 0) ui->btnDelete->setVisible(false);
    connect(ui->btnDelete, &QPushButton::clicked, this, [this]() {
        m_deleteRequested = true;
        reject();
    });

    // 取消/确定按钮
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnOk, &QPushButton::clicked, this, &FlowDialog::onAccept);

    // 初始化分类列表
    populateCategories(RecordType::EXPENSE);
    ui->categoryCombo->setCurrentIndex(-1);
}

// ==================== 分类联动 ====================
void FlowDialog::onTypeChanged()
{
    RecordType type = ui->radioIncome->isChecked()
        ? RecordType::INCOME : RecordType::EXPENSE;
    populateCategories(type);
}

void FlowDialog::updateSubCategory()
{
    QString cat = ui->categoryCombo->currentText();
    ui->subCategoryCombo->clear();
    QStringList items;
    if (cat == "饮食")
        items = {"早饭", "午饭", "晚饭", "夜宵", "小吃", "聚餐", "其他"};
    else if (cat == "交通")
        items = {"公交", "地铁", "打车", "共享单车", "火车", "飞机", "其他"};
    bool hasSub = !items.isEmpty();
    ui->subCategoryCombo->setVisible(hasSub);
    ui->subCategoryLabel->setVisible(hasSub);
    if (hasSub) {
        if (cat == "交通") ui->subCategoryLabel->setText("交通类型:");
        else ui->subCategoryLabel->setText("饮食类型:");
        ui->subCategoryCombo->addItems(items);
        ui->subCategoryCombo->setCurrentIndex(-1);
    }
}

void FlowDialog::populateCategories(RecordType type)
{
    ui->categoryCombo->clear();
    auto cats = m_catMan.getCategories(type);
    for (const auto& c : cats) {
        ui->categoryCombo->addItem(QString::fromStdString(c));
    }
}

// ==================== 表单数据填充/读取 ====================
void FlowDialog::setRecord(const Record& t)
{
    if (!t.date.empty()) {
        ui->dateEdit->setDate(QDate::fromString(
            QString::fromStdString(t.date), "yyyy-MM-dd"));
    }
    if (t.type == RecordType::INCOME)
        ui->radioIncome->setChecked(true);
    else
        ui->radioExpense->setChecked(true);

    populateCategories(t.type);
    ui->amountSpin->setValue(t.amount > 0 ? t.amount : 0.01);

    QString catStr = QString::fromStdString(t.category);
    int parenPos = catStr.indexOf('(');
    if (parenPos > 0) {
        QString mainCat = catStr.left(parenPos);
        QString subCat = catStr.mid(parenPos + 1).chopped(1);
        int idx = ui->categoryCombo->findText(mainCat);
        if (idx >= 0) ui->categoryCombo->setCurrentIndex(idx);
        updateSubCategory();
        int subIdx = ui->subCategoryCombo->findText(subCat);
        if (subIdx >= 0) ui->subCategoryCombo->setCurrentIndex(subIdx);
    } else if (!catStr.isEmpty()) {
        int idx2 = ui->categoryCombo->findText(catStr);
        if (idx2 >= 0) ui->categoryCombo->setCurrentIndex(idx2);
        updateSubCategory();
    }

    ui->noteEdit->setText(QString::fromStdString(t.note));
}

Record FlowDialog::getRecord() const
{
    Record t;
    t.id = m_editId;
    t.date = ui->dateEdit->date().toString("yyyy-MM-dd").toStdString();
    t.type = ui->radioIncome->isChecked() ? RecordType::INCOME : RecordType::EXPENSE;
    t.amount = ui->amountSpin->value();
    QString cat = ui->categoryCombo->currentText();
    if ((cat == "饮食" || cat == "交通") && ui->subCategoryCombo->currentIndex() >= 0) {
        cat += "(" + ui->subCategoryCombo->currentText() + ")";
    }
    t.category = cat.toStdString();
    t.note = ui->noteEdit->text().toStdString();
    return t;
}

// ==================== 提交验证 ====================
void FlowDialog::onAccept()
{
    if (ui->amountSpin->value() <= 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的金额（大于0）。");
        ui->amountSpin->setFocus();
        return;
    }
    if (ui->categoryCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择一个分类。");
        return;
    }
    if (ui->dateEdit->date() > QDate::currentDate()) {
        QString pickedDate = ui->dateEdit->date().toString("yyyy-MM-dd");
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("日期确认");
        msgBox.setText(QString("录入日期为 %1，晚于今天。\n是否继续记录？").arg(pickedDate));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setIcon(QMessageBox::Question);
        if (msgBox.exec() != QMessageBox::Ok) return;
    }
    accept();
}
