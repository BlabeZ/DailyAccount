/********************************************************************************
** Form generated from reading UI file 'flowdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FLOWDIALOG_H
#define UI_FLOWDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_FlowDialog
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QFormLayout *formLayout;
    QLabel *dateLabel;
    QHBoxLayout *dateRow;
    QDateEdit *dateEdit;
    QPushButton *calendarBtn;
    QLabel *typeLabel;
    QHBoxLayout *typeRow;
    QRadioButton *radioExpense;
    QRadioButton *radioIncome;
    QSpacerItem *typeSpacer;
    QLabel *amountLabel;
    QDoubleSpinBox *amountSpin;
    QLabel *categoryLabel;
    QComboBox *categoryCombo;
    QLabel *subCategoryLabel;
    QComboBox *subCategoryCombo;
    QLabel *noteLabel;
    QLineEdit *noteEdit;
    QHBoxLayout *btnRow;
    QPushButton *btnDelete;
    QSpacerItem *btnSpacer;
    QPushButton *btnCancel;
    QPushButton *btnOk;

    void setupUi(QDialog *FlowDialog)
    {
        if (FlowDialog->objectName().isEmpty())
            FlowDialog->setObjectName("FlowDialog");
        FlowDialog->resize(420, 460);
        FlowDialog->setMinimumWidth(420);
        mainLayout = new QVBoxLayout(FlowDialog);
        mainLayout->setSpacing(16);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(24, 24, 24, 24);
        titleLabel = new QLabel(FlowDialog);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: #2C3E50; background: transparent;"));

        mainLayout->addWidget(titleLabel);

        formLayout = new QFormLayout();
        formLayout->setSpacing(12);
        formLayout->setObjectName("formLayout");
        formLayout->setLabelAlignment(Qt::AlignRight|Qt::AlignVCenter);
        dateLabel = new QLabel(FlowDialog);
        dateLabel->setObjectName("dateLabel");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, dateLabel);

        dateRow = new QHBoxLayout();
        dateRow->setSpacing(4);
        dateRow->setObjectName("dateRow");
        dateEdit = new QDateEdit(FlowDialog);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setCalendarPopup(false);
        dateEdit->setMinimumWidth(200);

        dateRow->addWidget(dateEdit);

        calendarBtn = new QPushButton(FlowDialog);
        calendarBtn->setObjectName("calendarBtn");
        calendarBtn->setFixedSize(QSize(28, 28));
        calendarBtn->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        calendarBtn->setStyleSheet(QString::fromUtf8("QPushButton { background: #EBF5FB; border: 1px solid #D5DCE6; border-radius: 4px; font-size: 14px; } QPushButton:hover { background: #3498DB; }"));

        dateRow->addWidget(calendarBtn);


        formLayout->setLayout(0, QFormLayout::ItemRole::FieldRole, dateRow);

        typeLabel = new QLabel(FlowDialog);
        typeLabel->setObjectName("typeLabel");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, typeLabel);

        typeRow = new QHBoxLayout();
        typeRow->setObjectName("typeRow");
        radioExpense = new QRadioButton(FlowDialog);
        radioExpense->setObjectName("radioExpense");
        radioExpense->setChecked(true);

        typeRow->addWidget(radioExpense);

        radioIncome = new QRadioButton(FlowDialog);
        radioIncome->setObjectName("radioIncome");

        typeRow->addWidget(radioIncome);

        typeSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        typeRow->addItem(typeSpacer);


        formLayout->setLayout(1, QFormLayout::ItemRole::FieldRole, typeRow);

        amountLabel = new QLabel(FlowDialog);
        amountLabel->setObjectName("amountLabel");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, amountLabel);

        amountSpin = new QDoubleSpinBox(FlowDialog);
        amountSpin->setObjectName("amountSpin");
        amountSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
        amountSpin->setDecimals(2);
        amountSpin->setMaximum(99999999.989999994635582);
        amountSpin->setMinimumWidth(200);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, amountSpin);

        categoryLabel = new QLabel(FlowDialog);
        categoryLabel->setObjectName("categoryLabel");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, categoryLabel);

        categoryCombo = new QComboBox(FlowDialog);
        categoryCombo->setObjectName("categoryCombo");
        categoryCombo->setMinimumWidth(200);

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, categoryCombo);

        subCategoryLabel = new QLabel(FlowDialog);
        subCategoryLabel->setObjectName("subCategoryLabel");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, subCategoryLabel);

        subCategoryCombo = new QComboBox(FlowDialog);
        subCategoryCombo->setObjectName("subCategoryCombo");
        subCategoryCombo->setMinimumWidth(200);

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, subCategoryCombo);

        noteLabel = new QLabel(FlowDialog);
        noteLabel->setObjectName("noteLabel");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, noteLabel);

        noteEdit = new QLineEdit(FlowDialog);
        noteEdit->setObjectName("noteEdit");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, noteEdit);


        mainLayout->addLayout(formLayout);

        btnRow = new QHBoxLayout();
        btnRow->setObjectName("btnRow");
        btnDelete = new QPushButton(FlowDialog);
        btnDelete->setObjectName("btnDelete");
        btnDelete->setStyleSheet(QString::fromUtf8("QPushButton { background: #E74C3C; color: white; border-radius: 6px; padding: 10px 20px; } QPushButton:hover { background: #CB4335; }"));

        btnRow->addWidget(btnDelete);

        btnSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        btnRow->addItem(btnSpacer);

        btnCancel = new QPushButton(FlowDialog);
        btnCancel->setObjectName("btnCancel");
        btnCancel->setStyleSheet(QString::fromUtf8("QPushButton { background: #ECF0F1; color: #2C3E50; border-radius: 6px; padding: 10px 24px; } QPushButton:hover { background: #D5DCE6; }"));

        btnRow->addWidget(btnCancel);

        btnOk = new QPushButton(FlowDialog);
        btnOk->setObjectName("btnOk");
        btnOk->setStyleSheet(QString::fromUtf8("QPushButton { background: #3498DB; color: white; border-radius: 6px; padding: 10px 32px; } QPushButton:hover { background: #2980B9; }"));

        btnRow->addWidget(btnOk);


        mainLayout->addLayout(btnRow);


        retranslateUi(FlowDialog);

        QMetaObject::connectSlotsByName(FlowDialog);
    } // setupUi

    void retranslateUi(QDialog *FlowDialog)
    {
        dateLabel->setText(QCoreApplication::translate("FlowDialog", "\346\227\245\346\234\237:", nullptr));
        dateEdit->setDisplayFormat(QCoreApplication::translate("FlowDialog", "yyyy-MM-dd", nullptr));
        calendarBtn->setText(QCoreApplication::translate("FlowDialog", "\360\237\223\205", nullptr));
#if QT_CONFIG(tooltip)
        calendarBtn->setToolTip(QCoreApplication::translate("FlowDialog", "\346\211\223\345\274\200\346\227\245\345\216\206\351\200\211\346\213\251", nullptr));
#endif // QT_CONFIG(tooltip)
        typeLabel->setText(QCoreApplication::translate("FlowDialog", "\347\261\273\345\236\213:", nullptr));
        radioExpense->setText(QCoreApplication::translate("FlowDialog", "\346\224\257\345\207\272", nullptr));
        radioIncome->setText(QCoreApplication::translate("FlowDialog", "\346\224\266\345\205\245", nullptr));
        amountLabel->setText(QCoreApplication::translate("FlowDialog", "\351\207\221\351\242\235(\357\277\245):", nullptr));
        amountSpin->setSpecialValueText(QString());
        categoryLabel->setText(QCoreApplication::translate("FlowDialog", "\345\210\206\347\261\273:", nullptr));
        subCategoryLabel->setText(QCoreApplication::translate("FlowDialog", "\351\245\256\351\243\237\347\261\273\345\236\213:", nullptr));
        noteLabel->setText(QCoreApplication::translate("FlowDialog", "\345\244\207\346\263\250:", nullptr));
        noteEdit->setPlaceholderText(QCoreApplication::translate("FlowDialog", "\345\217\257\351\200\211\357\274\232\346\267\273\345\212\240\345\244\207\346\263\250\344\277\241\346\201\257...", nullptr));
        btnDelete->setText(QCoreApplication::translate("FlowDialog", "\345\210\240\351\231\244\350\256\260\345\275\225", nullptr));
        btnCancel->setText(QCoreApplication::translate("FlowDialog", "\345\217\226\346\266\210", nullptr));
        btnOk->setText(QCoreApplication::translate("FlowDialog", "\347\241\256\345\256\232", nullptr));
        (void)FlowDialog;
    } // retranslateUi

};

namespace Ui {
    class FlowDialog: public Ui_FlowDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FLOWDIALOG_H
