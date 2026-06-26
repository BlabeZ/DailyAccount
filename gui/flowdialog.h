/*
 * ============================================================================
 * 文件名:   flowdialog.h
 * 所属模块: GUI - 流水添加/编辑对话框（头文件）
 * 功    能: 声明 FlowDialog 类。界面由 flowdialog.ui 定义。
 *          支持添加模式和编辑模式（通过构造函数重载区分）。
 * 编    码: UTF-8
 * ============================================================================
 */

#ifndef FLOWDIALOG_H
#define FLOWDIALOG_H

#include <QDialog>
#include "ui_flowdialog.h"
#include "record.h"

class CategoryManager;

class FlowDialog : public QDialog {
    Q_OBJECT

public:
    explicit FlowDialog(CategoryManager& catMan, QWidget *parent = nullptr);
    FlowDialog(CategoryManager& catMan, const Record& existing, QWidget *parent = nullptr);

    Record getRecord() const;
    bool deleteRequested() const { return m_deleteRequested; }

private slots:
    void onTypeChanged();
    void onAccept();

private:
    void setupUI();
    void populateCategories(RecordType type);
    void setRecord(const Record& t);
    void updateSubCategory();

    CategoryManager& m_catMan;
    int m_editId = -1;
    bool m_deleteRequested = false;

    Ui::FlowDialog *ui;
};

#endif // FLOWDIALOG_H
