#ifndef FLOWPAGE_H
#define FLOWPAGE_H

#include <QWidget>
#include <QString>

namespace Ui { class FlowPage; }
class CategoryManager;
class Ledger;

class FlowPage : public QWidget {
    Q_OBJECT

public:
    explicit FlowPage(Ledger& ledger, const CategoryManager& categoryManager,
                      QWidget* parent = nullptr);
    ~FlowPage() override;

    void refresh();

signals:
    void dataChanged();

private slots:
    void onAdd();
    void onEdit(int id);
    void onDelete(int id);
    void onFilterChanged();

private:
    void loadPage();
    void includeDateInFilter(const QString& date);
    void showLedgerError(const QString& action);

    Ledger& m_ledger;
    const CategoryManager& m_categoryManager;
    Ui::FlowPage* ui;
};

#endif // FLOWPAGE_H
