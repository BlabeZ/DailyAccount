/********************************************************************************
** Form generated from reading UI file 'otherpage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OTHERPAGE_H
#define UI_OTHERPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OtherPage
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QSpacerItem *titleSpacer;
    QStackedWidget *featureStack;
    QWidget *featureListPage;
    QVBoxLayout *featureListLayout;
    QFrame *exportCard;
    QHBoxLayout *exportCardLayout;
    QLabel *exportIcon;
    QVBoxLayout *exportTextLayout;
    QLabel *exportTitle;
    QLabel *exportDesc;
    QPushButton *btnExportEnter;
    QFrame *clearCard;
    QHBoxLayout *clearCardLayout;
    QLabel *clearIcon;
    QVBoxLayout *clearTextLayout;
    QLabel *clearTitle;
    QLabel *clearDesc;
    QPushButton *btnClearEnter;
    QSpacerItem *featureListSpacer;
    QWidget *exportDetailPage;
    QVBoxLayout *exportDetailLayout;
    QPushButton *btnExportBack;
    QLabel *exportDetailTitle;
    QLabel *exportDetailDesc;
    QPushButton *btnExportData;
    QSpacerItem *exportDetailSpacer;
    QWidget *clearDetailPage;
    QVBoxLayout *clearDetailLayout;
    QPushButton *btnClearBack;
    QLabel *clearDetailTitle;
    QLabel *clearDetailDesc;
    QPushButton *btnClearData;
    QSpacerItem *clearDetailSpacer;

    void setupUi(QWidget *OtherPage)
    {
        if (OtherPage->objectName().isEmpty())
            OtherPage->setObjectName("OtherPage");
        OtherPage->resize(500, 400);
        mainLayout = new QVBoxLayout(OtherPage);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(24, 24, 24, 24);
        titleLabel = new QLabel(OtherPage);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: #2C3E50; background: transparent;"));

        mainLayout->addWidget(titleLabel);

        titleSpacer = new QSpacerItem(20, 16, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);

        mainLayout->addItem(titleSpacer);

        featureStack = new QStackedWidget(OtherPage);
        featureStack->setObjectName("featureStack");
        featureListPage = new QWidget();
        featureListPage->setObjectName("featureListPage");
        featureListLayout = new QVBoxLayout(featureListPage);
        featureListLayout->setSpacing(12);
        featureListLayout->setObjectName("featureListLayout");
        exportCard = new QFrame(featureListPage);
        exportCard->setObjectName("exportCard");
        exportCard->setStyleSheet(QString::fromUtf8("QFrame { background: white; border-radius: 10px; border: 1px solid #E8ECF1; padding: 20px; }"));
        exportCardLayout = new QHBoxLayout(exportCard);
        exportCardLayout->setObjectName("exportCardLayout");
        exportIcon = new QLabel(exportCard);
        exportIcon->setObjectName("exportIcon");
        exportIcon->setStyleSheet(QString::fromUtf8("font-size: 28px; background: transparent;"));

        exportCardLayout->addWidget(exportIcon);

        exportTextLayout = new QVBoxLayout();
        exportTextLayout->setObjectName("exportTextLayout");
        exportTitle = new QLabel(exportCard);
        exportTitle->setObjectName("exportTitle");
        exportTitle->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; background: transparent;"));

        exportTextLayout->addWidget(exportTitle);

        exportDesc = new QLabel(exportCard);
        exportDesc->setObjectName("exportDesc");
        exportDesc->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7F8C8D; background: transparent;"));

        exportTextLayout->addWidget(exportDesc);


        exportCardLayout->addLayout(exportTextLayout);

        btnExportEnter = new QPushButton(exportCard);
        btnExportEnter->setObjectName("btnExportEnter");
        btnExportEnter->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnExportEnter->setStyleSheet(QString::fromUtf8("QPushButton { background: #3498DB; color: white; border-radius: 6px; padding: 10px 20px; } QPushButton:hover { background: #2980B9; }"));

        exportCardLayout->addWidget(btnExportEnter);


        featureListLayout->addWidget(exportCard);

        clearCard = new QFrame(featureListPage);
        clearCard->setObjectName("clearCard");
        clearCard->setStyleSheet(QString::fromUtf8("QFrame { background: white; border-radius: 10px; border: 1px solid #E8ECF1; padding: 20px; }"));
        clearCardLayout = new QHBoxLayout(clearCard);
        clearCardLayout->setObjectName("clearCardLayout");
        clearIcon = new QLabel(clearCard);
        clearIcon->setObjectName("clearIcon");
        clearIcon->setStyleSheet(QString::fromUtf8("font-size: 28px; background: transparent;"));

        clearCardLayout->addWidget(clearIcon);

        clearTextLayout = new QVBoxLayout();
        clearTextLayout->setObjectName("clearTextLayout");
        clearTitle = new QLabel(clearCard);
        clearTitle->setObjectName("clearTitle");
        clearTitle->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: #E74C3C; background: transparent;"));

        clearTextLayout->addWidget(clearTitle);

        clearDesc = new QLabel(clearCard);
        clearDesc->setObjectName("clearDesc");
        clearDesc->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7F8C8D; background: transparent;"));

        clearTextLayout->addWidget(clearDesc);


        clearCardLayout->addLayout(clearTextLayout);

        btnClearEnter = new QPushButton(clearCard);
        btnClearEnter->setObjectName("btnClearEnter");
        btnClearEnter->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnClearEnter->setStyleSheet(QString::fromUtf8("QPushButton { background: #E74C3C; color: white; border-radius: 6px; padding: 10px 20px; } QPushButton:hover { background: #CB4335; }"));

        clearCardLayout->addWidget(btnClearEnter);


        featureListLayout->addWidget(clearCard);

        featureListSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        featureListLayout->addItem(featureListSpacer);

        featureStack->addWidget(featureListPage);
        exportDetailPage = new QWidget();
        exportDetailPage->setObjectName("exportDetailPage");
        exportDetailLayout = new QVBoxLayout(exportDetailPage);
        exportDetailLayout->setSpacing(16);
        exportDetailLayout->setObjectName("exportDetailLayout");
        btnExportBack = new QPushButton(exportDetailPage);
        btnExportBack->setObjectName("btnExportBack");
        btnExportBack->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnExportBack->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #3498DB; border: none; font-size: 14px; padding: 0; } QPushButton:hover { color: #2980B9; }"));

        exportDetailLayout->addWidget(btnExportBack);

        exportDetailTitle = new QLabel(exportDetailPage);
        exportDetailTitle->setObjectName("exportDetailTitle");
        exportDetailTitle->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: #2C3E50; background: transparent;"));

        exportDetailLayout->addWidget(exportDetailTitle);

        exportDetailDesc = new QLabel(exportDetailPage);
        exportDetailDesc->setObjectName("exportDetailDesc");
        exportDetailDesc->setWordWrap(true);
        exportDetailDesc->setStyleSheet(QString::fromUtf8("font-size: 13px; color: #555; background: transparent; line-height: 1.6;"));

        exportDetailLayout->addWidget(exportDetailDesc);

        btnExportData = new QPushButton(exportDetailPage);
        btnExportData->setObjectName("btnExportData");
        btnExportData->setMinimumHeight(48);
        btnExportData->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnExportData->setStyleSheet(QString::fromUtf8("QPushButton { background: #27AE60; color: white; border: none; border-radius: 8px; padding: 14px 32px; font-size: 15px; font-weight: bold; } QPushButton:hover { background: #219A52; }"));

        exportDetailLayout->addWidget(btnExportData);

        exportDetailSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        exportDetailLayout->addItem(exportDetailSpacer);

        featureStack->addWidget(exportDetailPage);
        clearDetailPage = new QWidget();
        clearDetailPage->setObjectName("clearDetailPage");
        clearDetailLayout = new QVBoxLayout(clearDetailPage);
        clearDetailLayout->setSpacing(16);
        clearDetailLayout->setObjectName("clearDetailLayout");
        btnClearBack = new QPushButton(clearDetailPage);
        btnClearBack->setObjectName("btnClearBack");
        btnClearBack->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnClearBack->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #3498DB; border: none; font-size: 14px; padding: 0; } QPushButton:hover { color: #2980B9; }"));

        clearDetailLayout->addWidget(btnClearBack);

        clearDetailTitle = new QLabel(clearDetailPage);
        clearDetailTitle->setObjectName("clearDetailTitle");
        clearDetailTitle->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: #E74C3C; background: transparent;"));

        clearDetailLayout->addWidget(clearDetailTitle);

        clearDetailDesc = new QLabel(clearDetailPage);
        clearDetailDesc->setObjectName("clearDetailDesc");
        clearDetailDesc->setWordWrap(true);
        clearDetailDesc->setStyleSheet(QString::fromUtf8("font-size: 13px; color: #555; background: transparent; line-height: 1.6;"));

        clearDetailLayout->addWidget(clearDetailDesc);

        btnClearData = new QPushButton(clearDetailPage);
        btnClearData->setObjectName("btnClearData");
        btnClearData->setMinimumHeight(48);
        btnClearData->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnClearData->setStyleSheet(QString::fromUtf8("QPushButton { background: #E74C3C; color: white; border: none; border-radius: 8px; padding: 14px 32px; font-size: 15px; font-weight: bold; } QPushButton:hover { background: #CB4335; }"));

        clearDetailLayout->addWidget(btnClearData);

        clearDetailSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        clearDetailLayout->addItem(clearDetailSpacer);

        featureStack->addWidget(clearDetailPage);

        mainLayout->addWidget(featureStack);


        retranslateUi(OtherPage);

        QMetaObject::connectSlotsByName(OtherPage);
    } // setupUi

    void retranslateUi(QWidget *OtherPage)
    {
        titleLabel->setText(QCoreApplication::translate("OtherPage", "\360\237\224\247 \345\205\266\344\273\226\345\212\237\350\203\275", nullptr));
        exportIcon->setText(QCoreApplication::translate("OtherPage", "\360\237\223\244", nullptr));
        exportTitle->setText(QCoreApplication::translate("OtherPage", "\346\225\260\346\215\256\345\257\274\345\207\272", nullptr));
        exportDesc->setText(QCoreApplication::translate("OtherPage", "\345\260\206\346\211\200\346\234\211\350\256\260\345\275\225\345\257\274\345\207\272\344\270\272TXT\346\226\207\344\273\266\344\277\235\345\255\230\345\210\260\346\241\214\351\235\242", nullptr));
        btnExportEnter->setText(QCoreApplication::translate("OtherPage", "\350\277\233\345\205\245 \342\206\222", nullptr));
        clearIcon->setText(QCoreApplication::translate("OtherPage", "\360\237\227\221\357\270\217", nullptr));
        clearTitle->setText(QCoreApplication::translate("OtherPage", "\346\270\205\351\231\244\346\225\260\346\215\256", nullptr));
        clearDesc->setText(QCoreApplication::translate("OtherPage", "\346\260\270\344\271\205\345\210\240\351\231\244\346\211\200\346\234\211\350\256\260\345\275\225\345\222\214\350\207\252\345\256\232\344\271\211\345\210\206\347\261\273", nullptr));
        btnClearEnter->setText(QCoreApplication::translate("OtherPage", "\350\277\233\345\205\245 \342\206\222", nullptr));
        btnExportBack->setText(QCoreApplication::translate("OtherPage", "\342\206\220 \350\277\224\345\233\236", nullptr));
        exportDetailTitle->setText(QCoreApplication::translate("OtherPage", "\360\237\223\244 \346\225\260\346\215\256\345\257\274\345\207\272", nullptr));
        exportDetailDesc->setText(QCoreApplication::translate("OtherPage", "\345\260\206\346\211\200\346\234\211\350\256\260\350\264\246\350\256\260\345\275\225\345\257\274\345\207\272\344\270\272\344\270\200\344\270\252 TXT \346\226\207\346\234\254\346\226\207\346\241\243\357\274\214\344\277\235\345\255\230\345\234\250\346\241\214\351\235\242\343\200\202", nullptr));
        btnExportData->setText(QCoreApplication::translate("OtherPage", "\360\237\223\244 \345\257\274\345\207\272\345\205\250\351\203\250\346\225\260\346\215\256\345\210\260\346\241\214\351\235\242", nullptr));
        btnClearBack->setText(QCoreApplication::translate("OtherPage", "\342\206\220 \350\277\224\345\233\236", nullptr));
        clearDetailTitle->setText(QCoreApplication::translate("OtherPage", "\360\237\227\221\357\270\217 \346\270\205\351\231\244\346\225\260\346\215\256", nullptr));
        clearDetailDesc->setText(QCoreApplication::translate("OtherPage", "\346\260\270\344\271\205\345\210\240\351\231\244\346\211\200\346\234\211\350\256\260\350\264\246\350\256\260\345\275\225\345\222\214\350\207\252\345\256\232\344\271\211\345\210\206\347\261\273\343\200\202", nullptr));
        btnClearData->setText(QCoreApplication::translate("OtherPage", "\360\237\227\221\357\270\217 \346\270\205\351\231\244\345\205\250\351\203\250\346\225\260\346\215\256", nullptr));
        (void)OtherPage;
    } // retranslateUi

};

namespace Ui {
    class OtherPage: public Ui_OtherPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OTHERPAGE_H
