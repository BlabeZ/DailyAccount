/*
 * ===========================================================================
 * 文件名称：dashboardpage.h
 * 所属模块：GUI 图形用户界面 - 概览页面
 * 功能描述：声明 DashboardPage 类。界面由 dashboardpage.ui 定义。
 * 编码格式：UTF-8
 * ===========================================================================
 */

#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include "ui_dashboardpage.h"

class Ledger;
class CategoryManager;
class BudgetManager;
class GoalManager;

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(Ledger& ledger, CategoryManager& catMan,
                           BudgetManager& budgetMan, GoalManager& goalMan,
                           QWidget *parent = nullptr);
    void refresh();

private:
    void refreshSummary();
    void refreshRecentRecords();
    void refreshCategoryBreakdown();
    void refreshBudgetOverview();
    void refreshGoalCards();

    Ledger& m_ledger;
    CategoryManager& m_catMan;
    BudgetManager& m_budgetMan;
    GoalManager& m_goalMan;

    Ui::DashboardPage *ui;
};

#endif // DASHBOARDPAGE_H
