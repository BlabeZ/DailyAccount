#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H

#include <QLineEdit>
#include <QListWidget>
#include <QWidget>

class CategoryManager;
class Ledger;

class CategoryPage : public QWidget {
    Q_OBJECT

public:
    explicit CategoryPage(Ledger& ledger, const CategoryManager& categoryManager,
                          QWidget* parent = nullptr);
    void refresh();

signals:
    void dataChanged();

private slots:
    void onAddExpenseCategory();
    void onAddIncomeCategory();
    void onDeleteExpenseCategory();
    void onDeleteIncomeCategory();

private:
    void setupUI();
    void loadCategories();
    void showLedgerError(const QString& action);

    Ledger& m_ledger;
    const CategoryManager& m_categoryManager;
    QListWidget* m_expenseList;
    QListWidget* m_incomeList;
    QLineEdit* m_expenseInput;
    QLineEdit* m_incomeInput;
};

#endif // CATEGORYPAGE_H
