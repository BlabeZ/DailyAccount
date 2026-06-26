/********************************************************************************
** Form generated from reading UI file 'dashboardpage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DASHBOARDPAGE_H
#define UI_DASHBOARDPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DashboardPage
{
public:
    QVBoxLayout *mainLayout;
    QLabel *pageTitle;
    QHBoxLayout *cardsRow;
    QLabel *cardIncome;
    QLabel *cardExpense;
    QLabel *cardBalance;
    QHBoxLayout *bottomRow;
    QFrame *recentFrame;
    QVBoxLayout *recentLayout;
    QLabel *recentTitle;
    QTableWidget *recentTable;
    QFrame *catFrame;
    QVBoxLayout *catLayout;
    QLabel *catTitle;
    QWidget *categoryBreakdown;
    QVBoxLayout *breakdownLayout;
    QSpacerItem *catSpacer;

    void setupUi(QWidget *DashboardPage)
    {
        if (DashboardPage->objectName().isEmpty())
            DashboardPage->setObjectName("DashboardPage");
        DashboardPage->resize(900, 600);
        mainLayout = new QVBoxLayout(DashboardPage);
        mainLayout->setSpacing(20);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(24, 24, 24, 24);
        pageTitle = new QLabel(DashboardPage);
        pageTitle->setObjectName("pageTitle");
        pageTitle->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: #2C3E50; background: transparent; padding: 0;"));

        mainLayout->addWidget(pageTitle);

        cardsRow = new QHBoxLayout();
        cardsRow->setSpacing(16);
        cardsRow->setObjectName("cardsRow");
        cardIncome = new QLabel(DashboardPage);
        cardIncome->setObjectName("cardIncome");
        cardIncome->setTextFormat(Qt::RichText);

        cardsRow->addWidget(cardIncome);

        cardExpense = new QLabel(DashboardPage);
        cardExpense->setObjectName("cardExpense");
        cardExpense->setTextFormat(Qt::RichText);

        cardsRow->addWidget(cardExpense);

        cardBalance = new QLabel(DashboardPage);
        cardBalance->setObjectName("cardBalance");
        cardBalance->setTextFormat(Qt::RichText);

        cardsRow->addWidget(cardBalance);


        mainLayout->addLayout(cardsRow);

        bottomRow = new QHBoxLayout();
        bottomRow->setSpacing(16);
        bottomRow->setObjectName("bottomRow");
        recentFrame = new QFrame(DashboardPage);
        recentFrame->setObjectName("recentFrame");
        recentLayout = new QVBoxLayout(recentFrame);
        recentLayout->setObjectName("recentLayout");
        recentTitle = new QLabel(recentFrame);
        recentTitle->setObjectName("recentTitle");
        recentTitle->setStyleSheet(QString::fromUtf8("font-size: 15px; font-weight: bold; background: transparent; padding-bottom: 8px;"));

        recentLayout->addWidget(recentTitle);

        recentTable = new QTableWidget(recentFrame);
        if (recentTable->columnCount() < 5)
            recentTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        recentTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        recentTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        recentTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        recentTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        recentTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        recentTable->setObjectName("recentTable");
        recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        recentTable->setMinimumHeight(280);

        recentLayout->addWidget(recentTable);


        bottomRow->addWidget(recentFrame);

        catFrame = new QFrame(DashboardPage);
        catFrame->setObjectName("catFrame");
        catLayout = new QVBoxLayout(catFrame);
        catLayout->setObjectName("catLayout");
        catTitle = new QLabel(catFrame);
        catTitle->setObjectName("catTitle");
        catTitle->setStyleSheet(QString::fromUtf8("font-size: 15px; font-weight: bold; background: transparent; padding-bottom: 8px;"));

        catLayout->addWidget(catTitle);

        categoryBreakdown = new QWidget(catFrame);
        categoryBreakdown->setObjectName("categoryBreakdown");
        categoryBreakdown->setStyleSheet(QString::fromUtf8("background: transparent;"));
        breakdownLayout = new QVBoxLayout(categoryBreakdown);
        breakdownLayout->setSpacing(6);
        breakdownLayout->setObjectName("breakdownLayout");
        breakdownLayout->setContentsMargins(0, 0, 0, 0);

        catLayout->addWidget(categoryBreakdown);

        catSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        catLayout->addItem(catSpacer);


        bottomRow->addWidget(catFrame);


        mainLayout->addLayout(bottomRow);


        retranslateUi(DashboardPage);

        QMetaObject::connectSlotsByName(DashboardPage);
    } // setupUi

    void retranslateUi(QWidget *DashboardPage)
    {
        pageTitle->setText(QCoreApplication::translate("DashboardPage", "\346\246\202\350\247\210", nullptr));
        recentFrame->setProperty("class", QVariant(QCoreApplication::translate("DashboardPage", "card", nullptr)));
        recentTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\223\213 \346\234\200\350\277\221\346\265\201\346\260\264\350\256\260\345\275\225", nullptr));
        QTableWidgetItem *___qtablewidgetitem = recentTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("DashboardPage", "\346\227\245\346\234\237", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = recentTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("DashboardPage", "\347\261\273\345\236\213", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = recentTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("DashboardPage", "\351\207\221\351\242\235", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = recentTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("DashboardPage", "\345\210\206\347\261\273", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = recentTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("DashboardPage", "\345\244\207\346\263\250", nullptr));
        catFrame->setProperty("class", QVariant(QCoreApplication::translate("DashboardPage", "card", nullptr)));
        catTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\223\212 \346\234\254\346\234\210\346\224\257\345\207\272\345\210\206\345\270\203", nullptr));
        (void)DashboardPage;
    } // retranslateUi

};

namespace Ui {
    class DashboardPage: public Ui_DashboardPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DASHBOARDPAGE_H
