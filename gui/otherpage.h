/*
 * ===========================================================================
 * 文件名称: otherpage.h
 * 所属模块: gui
 * 功能描述: OtherPage —— "其他功能"页面。首页仅显示功能入口按钮，
 *           点击后进入二级详情页执行实际操作。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#ifndef OTHERPAGE_H
#define OTHERPAGE_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>

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
    void setupUI();
    QWidget* createFeatureListPage();
    QWidget* createExportDetailPage();
    QWidget* createClearDetailPage();
    QString getDesktopPath() const;
    QString formatRecords() const;

    Ledger& m_ledger;
    QStackedWidget *m_stack;
};

#endif // OTHERPAGE_H
