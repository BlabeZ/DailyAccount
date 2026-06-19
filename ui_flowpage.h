/********************************************************************************
** Form generated from reading UI file 'flowpage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FLOWPAGE_H
#define UI_FLOWPAGE_H

#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FlowPage
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QFrame *toolbar;
    QHBoxLayout *toolLayout;
    QLabel *filterLabel;
    QLabel *fromLabel;
    QDateEdit *m_filterStart;
    QLabel *toLabel;
    QDateEdit *m_filterEnd;
    QComboBox *m_filterType;
    QPushButton *btnFilter;
    QSpacerItem *toolSpacer;
    QPushButton *m_btnAdd;
    QTreeWidget *m_tree;

    void setupUi(QWidget *FlowPage)
    {
        if (FlowPage->objectName().isEmpty())
            FlowPage->setObjectName("FlowPage");
        FlowPage->resize(800, 600);
        mainLayout = new QVBoxLayout(FlowPage);
        mainLayout->setSpacing(16);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(24, 24, 24, 24);
        titleLabel = new QLabel(FlowPage);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: #2C3E50; background: transparent;"));

        mainLayout->addWidget(titleLabel);

        toolbar = new QFrame(FlowPage);
        toolbar->setObjectName("toolbar");
        toolbar->setStyleSheet(QString::fromUtf8("QFrame { background: white; border-radius: 10px; border: 1px solid #E8ECF1; padding: 12px; }"));
        toolLayout = new QHBoxLayout(toolbar);
        toolLayout->setSpacing(12);
        toolLayout->setObjectName("toolLayout");
        filterLabel = new QLabel(toolbar);
        filterLabel->setObjectName("filterLabel");
        filterLabel->setStyleSheet(QString::fromUtf8("font-weight: bold; background: transparent;"));

        toolLayout->addWidget(filterLabel);

        fromLabel = new QLabel(toolbar);
        fromLabel->setObjectName("fromLabel");

        toolLayout->addWidget(fromLabel);

        m_filterStart = new QDateEdit(toolbar);
        m_filterStart->setObjectName("m_filterStart");
        m_filterStart->setCalendarPopup(true);
        m_filterStart->setMinimumWidth(160);
        m_filterStart->setDate(QDate(2026, 6, 1));

        toolLayout->addWidget(m_filterStart);

        toLabel = new QLabel(toolbar);
        toLabel->setObjectName("toLabel");

        toolLayout->addWidget(toLabel);

        m_filterEnd = new QDateEdit(toolbar);
        m_filterEnd->setObjectName("m_filterEnd");
        m_filterEnd->setCalendarPopup(true);
        m_filterEnd->setMinimumWidth(160);
        m_filterEnd->setDate(QDate(2026, 6, 19));

        toolLayout->addWidget(m_filterEnd);

        m_filterType = new QComboBox(toolbar);
        m_filterType->addItem(QString());
        m_filterType->addItem(QString());
        m_filterType->addItem(QString());
        m_filterType->setObjectName("m_filterType");

        toolLayout->addWidget(m_filterType);

        btnFilter = new QPushButton(toolbar);
        btnFilter->setObjectName("btnFilter");
        btnFilter->setStyleSheet(QString::fromUtf8("QPushButton { background: #3498DB; color: white; padding: 8px 16px; border-radius: 6px; } QPushButton:hover { background: #2980B9; }"));

        toolLayout->addWidget(btnFilter);

        toolSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        toolLayout->addItem(toolSpacer);

        m_btnAdd = new QPushButton(toolbar);
        m_btnAdd->setObjectName("m_btnAdd");
        m_btnAdd->setStyleSheet(QString::fromUtf8("QPushButton { background: #27AE60; color: white; padding: 10px 20px; font-size: 14px; border-radius: 6px; } QPushButton:hover { background: #219A52; }"));

        toolLayout->addWidget(m_btnAdd);


        mainLayout->addWidget(toolbar);

        m_tree = new QTreeWidget(FlowPage);
        m_tree->setObjectName("m_tree");
        m_tree->setRootIsDecorated(true);
        m_tree->setIndentation(20);
        m_tree->setAnimated(true);
        m_tree->setAlternatingRowColors(false);
        m_tree->setStyleSheet(QString::fromUtf8("QTreeWidget::item { padding: 6px 4px; border-bottom: 1px solid #F0F3F7; } QTreeWidget::item:hover { background-color: #F5F7FA; }"));

        mainLayout->addWidget(m_tree);


        retranslateUi(FlowPage);

        QMetaObject::connectSlotsByName(FlowPage);
    } // setupUi

    void retranslateUi(QWidget *FlowPage)
    {
        titleLabel->setText(QCoreApplication::translate("FlowPage", "\360\237\222\260 \350\264\246\347\233\256\346\230\216\347\273\206", nullptr));
        filterLabel->setText(QCoreApplication::translate("FlowPage", "\347\255\233\351\200\211:", nullptr));
        fromLabel->setText(QCoreApplication::translate("FlowPage", "\344\273\216:", nullptr));
        m_filterStart->setDisplayFormat(QCoreApplication::translate("FlowPage", "yyyy-MM-dd", nullptr));
        toLabel->setText(QCoreApplication::translate("FlowPage", "\350\207\263:", nullptr));
        m_filterEnd->setDisplayFormat(QCoreApplication::translate("FlowPage", "yyyy-MM-dd", nullptr));
        m_filterType->setItemText(0, QCoreApplication::translate("FlowPage", "\345\205\250\351\203\250", nullptr));
        m_filterType->setItemText(1, QCoreApplication::translate("FlowPage", "\346\224\257\345\207\272", nullptr));
        m_filterType->setItemText(2, QCoreApplication::translate("FlowPage", "\346\224\266\345\205\245", nullptr));

        btnFilter->setText(QCoreApplication::translate("FlowPage", "\347\255\233\351\200\211", nullptr));
        m_btnAdd->setText(QCoreApplication::translate("FlowPage", "\342\236\225 \346\267\273\345\212\240\350\256\260\345\275\225", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = m_tree->headerItem();
        ___qtreewidgetitem->setText(5, QCoreApplication::translate("FlowPage", "\345\244\207\346\263\250", nullptr));
        ___qtreewidgetitem->setText(4, QCoreApplication::translate("FlowPage", "\345\210\206\347\261\273", nullptr));
        ___qtreewidgetitem->setText(3, QCoreApplication::translate("FlowPage", "\351\207\221\351\242\235", nullptr));
        ___qtreewidgetitem->setText(2, QCoreApplication::translate("FlowPage", "\347\261\273\345\236\213", nullptr));
        ___qtreewidgetitem->setText(1, QCoreApplication::translate("FlowPage", "ID", nullptr));
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("FlowPage", "\346\227\245\346\234\237", nullptr));
        (void)FlowPage;
    } // retranslateUi

};

namespace Ui {
    class FlowPage: public Ui_FlowPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FLOWPAGE_H
