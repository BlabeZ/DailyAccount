/*
 * ===========================================================================
 * 文件名称: otherpage.h
 * 所属模块: gui
 * 功能描述: OtherPage —— "其他功能"页面。界面由 otherpage.ui 定义。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#ifndef OTHERPAGE_H
#define OTHERPAGE_H

#include <QWidget>
#include "ui_otherpage.h"

class Ledger;

class OtherPage : public QWidget {
    Q_OBJECT

public:
    explicit OtherPage(Ledger& ledger, QWidget *parent = nullptr);
    void refresh();

private slots:
    void showFeatureList();
    void showExportDetail();
    void showClearDetail();
    void onExportData();
    void onClearData();

private:
    QString getDesktopPath() const;
    QString formatRecords() const;

    Ledger& m_ledger;
    Ui::OtherPage *ui;
};

#endif // OTHERPAGE_H
