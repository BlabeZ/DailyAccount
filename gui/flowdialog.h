#ifndef FLOWDIALOG_H
#define FLOWDIALOG_H

#include "record.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>

class CategoryManager;

class FlowDialog : public QDialog {
    Q_OBJECT

public:
    explicit FlowDialog(const CategoryManager& categoryManager,
                         QWidget* parent = nullptr);
    FlowDialog(const CategoryManager& categoryManager, const Record& existing,
               QWidget* parent = nullptr);

    Record getRecord() const;
    int getRecordId() const { return m_editId; }
    bool deleteRequested() const { return m_deleteRequested; }

private slots:
    void onTypeChanged();
    void updateSubCategory();
    void onAccept();

private:
    void setupUI();
    void populateCategories(RecordType type);
    void setRecord(const Record& record);

    const CategoryManager& m_categoryManager;
    int m_editId = -1;
    bool m_deleteRequested = false;
    QDateEdit* m_dateEdit;
    QRadioButton* m_radioExpense;
    QRadioButton* m_radioIncome;
    QDoubleSpinBox* m_amountSpin;
    QComboBox* m_categoryCombo;
    QComboBox* m_subCategoryCombo;
    QLabel* m_subCategoryLabel;
    QLineEdit* m_noteEdit;
};

#endif // FLOWDIALOG_H
