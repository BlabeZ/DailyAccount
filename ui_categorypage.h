/********************************************************************************
** Form generated from reading UI file 'categorypage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CATEGORYPAGE_H
#define UI_CATEGORYPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CategoryPage
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QHBoxLayout *columnsLayout;
    QFrame *expenseFrame;
    QVBoxLayout *expenseLayout;
    QLabel *expenseTitle;
    QListWidget *expenseList;
    QHBoxLayout *expenseAddRow;
    QLineEdit *expenseInput;
    QPushButton *btnAddExpense;
    QPushButton *btnDelExpense;
    QFrame *incomeFrame;
    QVBoxLayout *incomeLayout;
    QLabel *incomeTitle;
    QListWidget *incomeList;
    QHBoxLayout *incomeAddRow;
    QLineEdit *incomeInput;
    QPushButton *btnAddIncome;
    QPushButton *btnDelIncome;

    void setupUi(QWidget *CategoryPage)
    {
        if (CategoryPage->objectName().isEmpty())
            CategoryPage->setObjectName("CategoryPage");
        CategoryPage->resize(700, 500);
        mainLayout = new QVBoxLayout(CategoryPage);
        mainLayout->setSpacing(16);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(24, 24, 24, 24);
        titleLabel = new QLabel(CategoryPage);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: #2C3E50; background: transparent;"));

        mainLayout->addWidget(titleLabel);

        subtitleLabel = new QLabel(CategoryPage);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setStyleSheet(QString::fromUtf8("color: #95A5A6; font-size: 12px; background: transparent;"));

        mainLayout->addWidget(subtitleLabel);

        columnsLayout = new QHBoxLayout();
        columnsLayout->setSpacing(16);
        columnsLayout->setObjectName("columnsLayout");
        expenseFrame = new QFrame(CategoryPage);
        expenseFrame->setObjectName("expenseFrame");
        expenseLayout = new QVBoxLayout(expenseFrame);
        expenseLayout->setObjectName("expenseLayout");
        expenseTitle = new QLabel(expenseFrame);
        expenseTitle->setObjectName("expenseTitle");
        expenseTitle->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: #E74C3C; background: transparent; padding-bottom: 8px;"));

        expenseLayout->addWidget(expenseTitle);

        expenseList = new QListWidget(expenseFrame);
        expenseList->setObjectName("expenseList");
        expenseList->setStyleSheet(QString::fromUtf8("QListWidget::item { padding: 10px 14px; } QListWidget::item:selected { background: #FDEDEC; color: #2C3E50; }"));

        expenseLayout->addWidget(expenseList);

        expenseAddRow = new QHBoxLayout();
        expenseAddRow->setObjectName("expenseAddRow");
        expenseInput = new QLineEdit(expenseFrame);
        expenseInput->setObjectName("expenseInput");

        expenseAddRow->addWidget(expenseInput);

        btnAddExpense = new QPushButton(expenseFrame);
        btnAddExpense->setObjectName("btnAddExpense");
        btnAddExpense->setStyleSheet(QString::fromUtf8("QPushButton { background: #E74C3C; color: white; padding: 8px 16px; } QPushButton:hover { background: #CB4335; }"));

        expenseAddRow->addWidget(btnAddExpense);


        expenseLayout->addLayout(expenseAddRow);

        btnDelExpense = new QPushButton(expenseFrame);
        btnDelExpense->setObjectName("btnDelExpense");
        btnDelExpense->setStyleSheet(QString::fromUtf8("QPushButton { background: #ECF0F1; color: #E74C3C; padding: 8px; border: 1px solid #E74C3C; border-radius: 6px; } QPushButton:hover { background: #FDEDEC; }"));

        expenseLayout->addWidget(btnDelExpense);


        columnsLayout->addWidget(expenseFrame);

        incomeFrame = new QFrame(CategoryPage);
        incomeFrame->setObjectName("incomeFrame");
        incomeLayout = new QVBoxLayout(incomeFrame);
        incomeLayout->setObjectName("incomeLayout");
        incomeTitle = new QLabel(incomeFrame);
        incomeTitle->setObjectName("incomeTitle");
        incomeTitle->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: #27AE60; background: transparent; padding-bottom: 8px;"));

        incomeLayout->addWidget(incomeTitle);

        incomeList = new QListWidget(incomeFrame);
        incomeList->setObjectName("incomeList");
        incomeList->setStyleSheet(QString::fromUtf8("QListWidget::item { padding: 10px 14px; } QListWidget::item:selected { background: #EAFAF1; color: #2C3E50; }"));

        incomeLayout->addWidget(incomeList);

        incomeAddRow = new QHBoxLayout();
        incomeAddRow->setObjectName("incomeAddRow");
        incomeInput = new QLineEdit(incomeFrame);
        incomeInput->setObjectName("incomeInput");

        incomeAddRow->addWidget(incomeInput);

        btnAddIncome = new QPushButton(incomeFrame);
        btnAddIncome->setObjectName("btnAddIncome");
        btnAddIncome->setStyleSheet(QString::fromUtf8("QPushButton { background: #27AE60; color: white; padding: 8px 16px; } QPushButton:hover { background: #219A52; }"));

        incomeAddRow->addWidget(btnAddIncome);


        incomeLayout->addLayout(incomeAddRow);

        btnDelIncome = new QPushButton(incomeFrame);
        btnDelIncome->setObjectName("btnDelIncome");
        btnDelIncome->setStyleSheet(QString::fromUtf8("QPushButton { background: #ECF0F1; color: #E74C3C; padding: 8px; border: 1px solid #E74C3C; border-radius: 6px; } QPushButton:hover { background: #FDEDEC; }"));

        incomeLayout->addWidget(btnDelIncome);


        columnsLayout->addWidget(incomeFrame);


        mainLayout->addLayout(columnsLayout);


        retranslateUi(CategoryPage);

        QMetaObject::connectSlotsByName(CategoryPage);
    } // setupUi

    void retranslateUi(QWidget *CategoryPage)
    {
        titleLabel->setText(QCoreApplication::translate("CategoryPage", "\360\237\217\267 \345\210\206\347\261\273\347\256\241\347\220\206", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("CategoryPage", "\351\242\204\350\256\276\345\210\206\347\261\273\344\270\215\345\217\257\345\210\240\351\231\244\343\200\202\346\255\243\345\234\250\344\275\277\347\224\250\347\232\204\350\207\252\345\256\232\344\271\211\345\210\206\347\261\273\345\217\227\344\277\235\346\212\244\343\200\202", nullptr));
        expenseFrame->setProperty("class", QVariant(QCoreApplication::translate("CategoryPage", "card", nullptr)));
        expenseTitle->setText(QCoreApplication::translate("CategoryPage", "\360\237\223\211 \346\224\257\345\207\272\345\210\206\347\261\273", nullptr));
        expenseInput->setPlaceholderText(QCoreApplication::translate("CategoryPage", "\346\226\260\346\224\257\345\207\272\345\210\206\347\261\273\345\220\215\347\247\260...", nullptr));
        btnAddExpense->setText(QCoreApplication::translate("CategoryPage", "\342\236\225 \346\267\273\345\212\240", nullptr));
        btnDelExpense->setText(QCoreApplication::translate("CategoryPage", "\360\237\227\221 \345\210\240\351\231\244\351\200\211\344\270\255", nullptr));
        incomeFrame->setProperty("class", QVariant(QCoreApplication::translate("CategoryPage", "card", nullptr)));
        incomeTitle->setText(QCoreApplication::translate("CategoryPage", "\360\237\223\210 \346\224\266\345\205\245\345\210\206\347\261\273", nullptr));
        incomeInput->setPlaceholderText(QCoreApplication::translate("CategoryPage", "\346\226\260\346\224\266\345\205\245\345\210\206\347\261\273\345\220\215\347\247\260...", nullptr));
        btnAddIncome->setText(QCoreApplication::translate("CategoryPage", "\342\236\225 \346\267\273\345\212\240", nullptr));
        btnDelIncome->setText(QCoreApplication::translate("CategoryPage", "\360\237\227\221 \345\210\240\351\231\244\351\200\211\344\270\255", nullptr));
        (void)CategoryPage;
    } // retranslateUi

};

namespace Ui {
    class CategoryPage: public Ui_CategoryPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CATEGORYPAGE_H
