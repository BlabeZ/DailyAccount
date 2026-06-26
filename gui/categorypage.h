/*
 * ============================================================================
 * 文件名: categorypage.h
 * 模块:   分类管理页面（GUI头文件）
 * 功能:   声明分类管理页面类 CategoryPage。
 *         界面由 categorypage.ui 定义，可通过 Qt Designer 可视化编辑。
 * 编码:   UTF-8
 * ============================================================================
 */

#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "ui_categorypage.h"

class Ledger;
class CategoryManager;

class CategoryPage : public QWidget {
    Q_OBJECT

public:
    explicit CategoryPage(Ledger& ledger, CategoryManager& catMan, QWidget *parent = nullptr);
    void refresh();

private slots:
    void onAddExpenseCategory();
    void onAddIncomeCategory();
    void onDeleteExpenseCategory();
    void onDeleteIncomeCategory();

private:
    void loadCategories();

    Ledger& m_ledger;
    CategoryManager& m_catMan;

    Ui::CategoryPage *ui;
};

#endif // CATEGORYPAGE_H
