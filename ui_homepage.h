/********************************************************************************
** Form generated from reading UI file 'homepage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HOMEPAGE_H
#define UI_HOMEPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HomePage
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *titleRow;
    QLabel *charLabel_0;
    QLabel *charLabel_1;
    QLabel *charLabel_2;
    QLabel *subtitleLabel;

    void setupUi(QWidget *HomePage)
    {
        if (HomePage->objectName().isEmpty())
            HomePage->setObjectName("HomePage");
        HomePage->resize(400, 300);
        mainLayout = new QVBoxLayout(HomePage);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setAlignment(Qt::AlignCenter);
        titleRow = new QHBoxLayout();
        titleRow->setSpacing(36);
        titleRow->setObjectName("titleRow");
        titleRow->setAlignment(Qt::AlignCenter);
        charLabel_0 = new QLabel(HomePage);
        charLabel_0->setObjectName("charLabel_0");
        charLabel_0->setAlignment(Qt::AlignCenter);
        charLabel_0->setFixedWidth(100);

        titleRow->addWidget(charLabel_0);

        charLabel_1 = new QLabel(HomePage);
        charLabel_1->setObjectName("charLabel_1");
        charLabel_1->setAlignment(Qt::AlignCenter);
        charLabel_1->setFixedWidth(100);

        titleRow->addWidget(charLabel_1);

        charLabel_2 = new QLabel(HomePage);
        charLabel_2->setObjectName("charLabel_2");
        charLabel_2->setAlignment(Qt::AlignCenter);
        charLabel_2->setFixedWidth(100);

        titleRow->addWidget(charLabel_2);


        mainLayout->addLayout(titleRow);

        subtitleLabel = new QLabel(HomePage);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(subtitleLabel);


        retranslateUi(HomePage);

        QMetaObject::connectSlotsByName(HomePage);
    } // setupUi

    void retranslateUi(QWidget *HomePage)
    {
        charLabel_0->setText(QCoreApplication::translate("HomePage", "\345\260\217", nullptr));
        charLabel_1->setText(QCoreApplication::translate("HomePage", "\345\267\245", nullptr));
        charLabel_2->setText(QCoreApplication::translate("HomePage", "\345\205\267", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("HomePage", "\346\254\242\350\277\216\344\275\277\347\224\250", nullptr));
        (void)HomePage;
    } // retranslateUi

};

namespace Ui {
    class HomePage: public Ui_HomePage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOMEPAGE_H
