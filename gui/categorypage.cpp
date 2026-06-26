/*
 * ============================================================================
 * 文件名: categorypage.cpp
 * 模块:   分类管理页面（GUI实现文件）
 * 功能:   实现分类管理页面的全部逻辑。界面由 categorypage.ui 定义。
 * 编码:   UTF-8
 * ============================================================================
 */

#include "categorypage.h"
#include "category.h"
#include "ledger.h"
#include <QMessageBox>
#include <QApplication>

CategoryPage::CategoryPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent)
    : QWidget(parent)
    , m_ledger(ledger)
    , m_catMan(catMan)
    , ui(new Ui::CategoryPage)
{
    ui->setupUi(this);

    // 信号连接
    connect(ui->btnAddExpense, &QPushButton::clicked,
            this, &CategoryPage::onAddExpenseCategory);
    connect(ui->btnAddIncome, &QPushButton::clicked,
            this, &CategoryPage::onAddIncomeCategory);
    connect(ui->btnDelExpense, &QPushButton::clicked,
            this, &CategoryPage::onDeleteExpenseCategory);
    connect(ui->btnDelIncome, &QPushButton::clicked,
            this, &CategoryPage::onDeleteIncomeCategory);
}

void CategoryPage::refresh()
{
    loadCategories();
}

void CategoryPage::loadCategories()
{
    ui->expenseList->clear();
    ui->incomeList->clear();

    auto inUseExpense = m_catMan.getInUseCategories(RecordType::EXPENSE);
    auto inUseIncome  = m_catMan.getInUseCategories(RecordType::INCOME);

    // 填充支出分类列表
    auto expenseCats = m_catMan.getCategories(RecordType::EXPENSE);
    for (const auto& cat : expenseCats) {
        bool isPreset = m_catMan.isPreset(RecordType::EXPENSE, cat);
        bool isInUse  = (inUseExpense.find(cat) != inUseExpense.end());

        QString display = QString::fromStdString(cat);
        display += isPreset ? QStringLiteral("  [预设]") : QStringLiteral("  [自定义]");
        if (isInUse) display += QStringLiteral("  (使用中)");

        QListWidgetItem *item = new QListWidgetItem(display);
        if (isPreset) item->setForeground(QColor("#95A5A6"));
        item->setData(Qt::UserRole, QString::fromStdString(cat));
        item->setData(Qt::UserRole + 1, isPreset);
        item->setData(Qt::UserRole + 2, isInUse);
        ui->expenseList->addItem(item);
    }

    // 填充收入分类列表
    auto incomeCats = m_catMan.getCategories(RecordType::INCOME);
    for (const auto& cat : incomeCats) {
        bool isPreset = m_catMan.isPreset(RecordType::INCOME, cat);
        bool isInUse  = (inUseIncome.find(cat) != inUseIncome.end());

        QString display = QString::fromStdString(cat);
        display += isPreset ? QStringLiteral("  [预设]") : QStringLiteral("  [自定义]");
        if (isInUse) display += QStringLiteral("  (使用中)");

        QListWidgetItem *item = new QListWidgetItem(display);
        if (isPreset) item->setForeground(QColor("#95A5A6"));
        item->setData(Qt::UserRole, QString::fromStdString(cat));
        item->setData(Qt::UserRole + 1, isPreset);
        item->setData(Qt::UserRole + 2, isInUse);
        ui->incomeList->addItem(item);
    }
}

void CategoryPage::onAddExpenseCategory()
{
    QString name = ui->expenseInput->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("输入错误"), QStringLiteral("请输入分类名称。"));
        return;
    }
    if (m_catMan.addCustomCategory(RecordType::EXPENSE, name.toStdString())) {
        m_ledger.save();
        loadCategories();
        ui->expenseInput->clear();
    } else {
        QMessageBox::warning(this, QStringLiteral("添加失败"), QStringLiteral("分类名称已存在或无效。"));
    }
}

void CategoryPage::onAddIncomeCategory()
{
    QString name = ui->incomeInput->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("输入错误"), QStringLiteral("请输入分类名称。"));
        return;
    }
    if (m_catMan.addCustomCategory(RecordType::INCOME, name.toStdString())) {
        m_ledger.save();
        loadCategories();
        ui->incomeInput->clear();
    } else {
        QMessageBox::warning(this, QStringLiteral("添加失败"), QStringLiteral("分类名称已存在或无效。"));
    }
}

void CategoryPage::onDeleteExpenseCategory()
{
    auto *item = ui->expenseList->currentItem();
    if (!item) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选中要删除的分类。"));
        return;
    }
    bool isPreset = item->data(Qt::UserRole + 1).toBool();
    bool isInUse  = item->data(Qt::UserRole + 2).toBool();
    if (isPreset) {
        QMessageBox::warning(this, QStringLiteral("无法删除"), QStringLiteral("预设分类不可删除。"));
        return;
    }
    if (isInUse) {
        QMessageBox::warning(this, QStringLiteral("无法删除"),
            QStringLiteral("该分类正在被使用中，请先删除或修改相关流水记录。"));
        return;
    }
    QString name = item->data(Qt::UserRole).toString();
    if (m_catMan.removeCustomCategory(RecordType::EXPENSE, name.toStdString())) {
        m_ledger.save();
        loadCategories();
    }
}

void CategoryPage::onDeleteIncomeCategory()
{
    auto *item = ui->incomeList->currentItem();
    if (!item) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选中要删除的分类。"));
        return;
    }
    bool isPreset = item->data(Qt::UserRole + 1).toBool();
    bool isInUse  = item->data(Qt::UserRole + 2).toBool();
    if (isPreset) {
        QMessageBox::warning(this, QStringLiteral("无法删除"), QStringLiteral("预设分类不可删除。"));
        return;
    }
    if (isInUse) {
        QMessageBox::warning(this, QStringLiteral("无法删除"),
            QStringLiteral("该分类正在被使用中，请先删除或修改相关流水记录。"));
        return;
    }
    QString name = item->data(Qt::UserRole).toString();
    if (m_catMan.removeCustomCategory(RecordType::INCOME, name.toStdString())) {
        m_ledger.save();
        loadCategories();
    }
}
