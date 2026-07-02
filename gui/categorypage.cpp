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
#include "classifier.h"
#include "ledger.h"
#include <QMessageBox>
#include <QApplication>
#include <QInputDialog>
#include <QScrollArea>

CategoryPage::CategoryPage(Ledger& ledger, CategoryManager& catMan,
                           SmartClassifier& classifier, QWidget *parent)
    : QWidget(parent)
    , m_ledger(ledger)
    , m_catMan(catMan)
    , m_classifier(classifier)
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

    // 在分类列表下方添加关键词映射管理区域
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        // 关键词映射卡片
        QFrame *keywordFrame = new QFrame;
        keywordFrame->setProperty("class", "card");
        keywordFrame->setStyleSheet("QFrame[class=\"card\"] { background: white; "
                                     "border: 1px solid #E8ECF1; border-radius: 10px; "
                                     "padding: 16px; margin-top: 8px; }");
        QVBoxLayout *kwLayout = new QVBoxLayout(keywordFrame);

        QLabel *kwTitle = new QLabel("🔗 关键词 → 分类映射");
        kwTitle->setStyleSheet("font-size: 15px; font-weight: bold; background: transparent; "
                                "padding-bottom: 4px;");
        kwLayout->addWidget(kwTitle);

        QLabel *kwHint = new QLabel("输入备注时自动匹配关键词并建议分类");
        kwHint->setStyleSheet("color: #95A5A6; font-size: 12px; background: transparent; "
                               "padding-bottom: 8px;");
        kwLayout->addWidget(kwHint);

        // 映射列表
        m_keywordList = new QListWidget;
        m_keywordList->setMaximumHeight(150);
        m_keywordList->setStyleSheet(
            "QListWidget::item { padding: 6px 10px; font-size: 12px; } "
            "QListWidget::item:selected { background: #EBF5FB; color: #2C3E50; }");
        kwLayout->addWidget(m_keywordList);

        // 按钮行
        QHBoxLayout *kwBtnRow = new QHBoxLayout;
        QPushButton *btnLearn = new QPushButton("🔄 从历史记录学习");
        btnLearn->setStyleSheet("QPushButton { background: #3498DB; color: white; "
                                 "border-radius: 6px; padding: 6px 14px; font-size: 12px; } "
                                 "QPushButton:hover { background: #2980B9; }");
        connect(btnLearn, &QPushButton::clicked, this, &CategoryPage::onLearnKeywords);

        QPushButton *btnAddKw = new QPushButton("➕ 手动添加");
        btnAddKw->setStyleSheet("QPushButton { background: #27AE60; color: white; "
                                 "border-radius: 6px; padding: 6px 14px; font-size: 12px; } "
                                 "QPushButton:hover { background: #219A52; }");
        connect(btnAddKw, &QPushButton::clicked, this, &CategoryPage::onAddKeywordMapping);

        QPushButton *btnDelKw = new QPushButton("🗑 删除选中");
        btnDelKw->setStyleSheet("QPushButton { background: #ECF0F1; color: #E74C3C; "
                                 "border: 1px solid #E74C3C; border-radius: 6px; "
                                 "padding: 6px 14px; font-size: 12px; } "
                                 "QPushButton:hover { background: #FDEDEC; }");
        connect(btnDelKw, &QPushButton::clicked, this, &CategoryPage::onDeleteKeywordMapping);

        kwBtnRow->addWidget(btnLearn);
        kwBtnRow->addWidget(btnAddKw);
        kwBtnRow->addWidget(btnDelKw);
        kwBtnRow->addStretch();
        kwLayout->addLayout(kwBtnRow);

        mainLayout->addWidget(keywordFrame);
    }
}

void CategoryPage::refresh()
{
    loadCategories();
    loadKeywordMappings();
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

// ==================== 关键词映射管理 ====================

void CategoryPage::loadKeywordMappings()
{
    if (!m_keywordList) return;
    m_keywordList->clear();

    const auto& mappings = m_classifier.getAllMappings();
    for (const auto& [keyword, category] : mappings) {
        QString display = QString("%1  →  %2")
                              .arg(QString::fromStdString(keyword))
                              .arg(QString::fromStdString(category));
        QListWidgetItem *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, QString::fromStdString(keyword));
        m_keywordList->addItem(item);
    }

    if (mappings.empty()) {
        QListWidgetItem *item = new QListWidgetItem(
            QString::fromUtf8("暂无映射。点击「从历史记录学习」或「手动添加」来创建。"));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setForeground(QColor("#95A5A6"));
        m_keywordList->addItem(item);
    }
}

void CategoryPage::onLearnKeywords()
{
    const auto& records = m_ledger.getAllRecords();
    if (records.empty()) {
        QMessageBox::information(this, "提示", "暂无记账记录，无法学习关键词映射。\n请先添加一些记录。");
        return;
    }

    size_t before = m_classifier.getAllMappings().size();
    m_classifier.learn(records);
    size_t after = m_classifier.getAllMappings().size();

    int added = (int)(after - before);
    loadKeywordMappings();

    if (added > 0) {
        QMessageBox::information(this, "学习完成",
            QString("从 %1 条记录中学习了 %2 个新的关键词映射。\n"
                    "现在输入备注时会自动建议分类。")
                .arg(records.size()).arg(added));
    } else {
        QMessageBox::information(this, "学习完成",
            "未发现新的关键词映射。\n可能所有关键词已有映射，或记录中备注信息不足。");
    }
}

void CategoryPage::onAddKeywordMapping()
{
    // 获取分类列表供选择
    QStringList cats;
    auto expCats = m_catMan.getCategories(RecordType::EXPENSE);
    for (const auto& c : expCats) {
        cats.append(QString::fromStdString(c));
    }
    auto incCats = m_catMan.getCategories(RecordType::INCOME);
    for (const auto& c : incCats) {
        cats.append(QString::fromStdString(c));
    }

    if (cats.isEmpty()) {
        QMessageBox::warning(this, "错误", "没有可用的分类。");
        return;
    }

    bool ok;
    QString keyword = QInputDialog::getText(this, "添加关键词映射",
        "请输入关键词（2-4个汉字）:", QLineEdit::Normal, "", &ok);
    if (!ok || keyword.trimmed().isEmpty()) return;

    QString category = QInputDialog::getItem(this, "选择分类",
        "请选择该关键词对应的分类:", cats, 0, false, &ok);
    if (!ok) return;

    m_classifier.addMapping(keyword.trimmed().toStdString(), category.toStdString());
    loadKeywordMappings();
}

void CategoryPage::onDeleteKeywordMapping()
{
    if (!m_keywordList) return;
    auto *item = m_keywordList->currentItem();
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) {
        QMessageBox::information(this, "提示", "请先选中要删除的关键词映射。");
        return;
    }

    QString keyword = item->data(Qt::UserRole).toString();
    m_classifier.removeMapping(keyword.toStdString());
    loadKeywordMappings();
}
