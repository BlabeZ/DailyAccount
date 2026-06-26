/********************************************************************************
** Form generated from reading UI file 'statisticspage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STATISTICSPAGE_H
#define UI_STATISTICSPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StatisticsPage
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *titleRow;
    QLabel *pageTitle;
    QSpacerItem *titleSpacer;
    QLabel *rangeLabel;
    QComboBox *rangeCombo;
    QHBoxLayout *summaryRow;
    QLabel *summaryIncome;
    QLabel *summaryExpense;
    QLabel *summaryBalance;
    QLabel *summaryCount;
    QSpacerItem *summarySpacer;
    QHBoxLayout *chartsRow;
    QFrame *expensePieFrame;
    QVBoxLayout *expensePieLayout;
    QLabel *expensePieTitle;
    QWidget *expensePiePlaceholder;
    QFrame *incomePieFrame;
    QVBoxLayout *incomePieLayout;
    QLabel *incomePieTitle;
    QWidget *incomePiePlaceholder;
    QFrame *barFrame;
    QVBoxLayout *barLayout;
    QLabel *barTitle;
    QWidget *barPlaceholder;

    void setupUi(QWidget *StatisticsPage)
    {
        if (StatisticsPage->objectName().isEmpty())
            StatisticsPage->setObjectName("StatisticsPage");
        StatisticsPage->resize(800, 600);
        mainLayout = new QVBoxLayout(StatisticsPage);
        mainLayout->setSpacing(16);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(24, 24, 24, 24);
        titleRow = new QHBoxLayout();
        titleRow->setObjectName("titleRow");
        pageTitle = new QLabel(StatisticsPage);
        pageTitle->setObjectName("pageTitle");
        pageTitle->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: #2C3E50; background: transparent;"));

        titleRow->addWidget(pageTitle);

        titleSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        titleRow->addItem(titleSpacer);

        rangeLabel = new QLabel(StatisticsPage);
        rangeLabel->setObjectName("rangeLabel");

        titleRow->addWidget(rangeLabel);

        rangeCombo = new QComboBox(StatisticsPage);
        rangeCombo->addItem(QString());
        rangeCombo->addItem(QString());
        rangeCombo->addItem(QString());
        rangeCombo->addItem(QString());
        rangeCombo->setObjectName("rangeCombo");
        rangeCombo->setFixedWidth(130);

        titleRow->addWidget(rangeCombo);


        mainLayout->addLayout(titleRow);

        summaryRow = new QHBoxLayout();
        summaryRow->setSpacing(16);
        summaryRow->setObjectName("summaryRow");
        summaryIncome = new QLabel(StatisticsPage);
        summaryIncome->setObjectName("summaryIncome");
        summaryIncome->setTextFormat(Qt::RichText);
        summaryIncome->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: #27AE60; background: transparent; padding: 8px 0;"));

        summaryRow->addWidget(summaryIncome);

        summaryExpense = new QLabel(StatisticsPage);
        summaryExpense->setObjectName("summaryExpense");
        summaryExpense->setTextFormat(Qt::RichText);
        summaryExpense->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: #E74C3C; background: transparent; padding: 8px 0;"));

        summaryRow->addWidget(summaryExpense);

        summaryBalance = new QLabel(StatisticsPage);
        summaryBalance->setObjectName("summaryBalance");
        summaryBalance->setTextFormat(Qt::RichText);
        summaryBalance->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: #3498DB; background: transparent; padding: 8px 0;"));

        summaryRow->addWidget(summaryBalance);

        summaryCount = new QLabel(StatisticsPage);
        summaryCount->setObjectName("summaryCount");
        summaryCount->setTextFormat(Qt::RichText);
        summaryCount->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: #7F8C8D; background: transparent; padding: 8px 0;"));

        summaryRow->addWidget(summaryCount);

        summarySpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        summaryRow->addItem(summarySpacer);


        mainLayout->addLayout(summaryRow);

        chartsRow = new QHBoxLayout();
        chartsRow->setSpacing(16);
        chartsRow->setObjectName("chartsRow");
        expensePieFrame = new QFrame(StatisticsPage);
        expensePieFrame->setObjectName("expensePieFrame");
        expensePieLayout = new QVBoxLayout(expensePieFrame);
        expensePieLayout->setObjectName("expensePieLayout");
        expensePieTitle = new QLabel(expensePieFrame);
        expensePieTitle->setObjectName("expensePieTitle");
        expensePieTitle->setAlignment(Qt::AlignCenter);
        expensePieTitle->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; padding: 0 0 8px 0; background: transparent;"));

        expensePieLayout->addWidget(expensePieTitle);

        expensePiePlaceholder = new QWidget(expensePieFrame);
        expensePiePlaceholder->setObjectName("expensePiePlaceholder");
        expensePiePlaceholder->setMinimumSize(QSize(280, 250));

        expensePieLayout->addWidget(expensePiePlaceholder);


        chartsRow->addWidget(expensePieFrame);

        incomePieFrame = new QFrame(StatisticsPage);
        incomePieFrame->setObjectName("incomePieFrame");
        incomePieLayout = new QVBoxLayout(incomePieFrame);
        incomePieLayout->setObjectName("incomePieLayout");
        incomePieTitle = new QLabel(incomePieFrame);
        incomePieTitle->setObjectName("incomePieTitle");
        incomePieTitle->setAlignment(Qt::AlignCenter);
        incomePieTitle->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; padding: 0 0 8px 0; background: transparent;"));

        incomePieLayout->addWidget(incomePieTitle);

        incomePiePlaceholder = new QWidget(incomePieFrame);
        incomePiePlaceholder->setObjectName("incomePiePlaceholder");
        incomePiePlaceholder->setMinimumSize(QSize(280, 250));

        incomePieLayout->addWidget(incomePiePlaceholder);


        chartsRow->addWidget(incomePieFrame);


        mainLayout->addLayout(chartsRow);

        barFrame = new QFrame(StatisticsPage);
        barFrame->setObjectName("barFrame");
        barLayout = new QVBoxLayout(barFrame);
        barLayout->setObjectName("barLayout");
        barTitle = new QLabel(barFrame);
        barTitle->setObjectName("barTitle");
        barTitle->setAlignment(Qt::AlignCenter);
        barTitle->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; padding: 0 0 8px 0; background: transparent;"));

        barLayout->addWidget(barTitle);

        barPlaceholder = new QWidget(barFrame);
        barPlaceholder->setObjectName("barPlaceholder");
        barPlaceholder->setMinimumSize(QSize(240, 168));

        barLayout->addWidget(barPlaceholder);


        mainLayout->addWidget(barFrame);


        retranslateUi(StatisticsPage);

        QMetaObject::connectSlotsByName(StatisticsPage);
    } // setupUi

    void retranslateUi(QWidget *StatisticsPage)
    {
        pageTitle->setText(QCoreApplication::translate("StatisticsPage", "\360\237\223\210 \347\273\237\350\256\241\345\210\206\346\236\220", nullptr));
        rangeLabel->setText(QCoreApplication::translate("StatisticsPage", "\346\227\266\351\227\264\350\214\203\345\233\264:", nullptr));
        rangeCombo->setItemText(0, QCoreApplication::translate("StatisticsPage", "\346\234\254\346\234\210", nullptr));
        rangeCombo->setItemText(1, QCoreApplication::translate("StatisticsPage", "\350\277\2213\344\270\252\346\234\210", nullptr));
        rangeCombo->setItemText(2, QCoreApplication::translate("StatisticsPage", "\350\277\2211\345\271\264", nullptr));
        rangeCombo->setItemText(3, QCoreApplication::translate("StatisticsPage", "\345\205\250\351\203\250", nullptr));

        expensePieFrame->setProperty("class", QVariant(QCoreApplication::translate("StatisticsPage", "card", nullptr)));
        expensePieTitle->setText(QCoreApplication::translate("StatisticsPage", "\346\224\257\345\207\272\345\210\206\347\261\273\345\210\206\345\270\203", nullptr));
        incomePieFrame->setProperty("class", QVariant(QCoreApplication::translate("StatisticsPage", "card", nullptr)));
        incomePieTitle->setText(QCoreApplication::translate("StatisticsPage", "\346\224\266\345\205\245\345\210\206\347\261\273\345\210\206\345\270\203", nullptr));
        barFrame->setProperty("class", QVariant(QCoreApplication::translate("StatisticsPage", "card", nullptr)));
        barTitle->setText(QCoreApplication::translate("StatisticsPage", "\346\234\210\345\272\246\346\224\266\346\224\257\350\266\213\345\212\277", nullptr));
        (void)StatisticsPage;
    } // retranslateUi

};

namespace Ui {
    class StatisticsPage: public Ui_StatisticsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STATISTICSPAGE_H
