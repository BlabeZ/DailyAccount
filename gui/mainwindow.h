/*
 * ===========================================================================
 * 文件名称: mainwindow.h
 * 所属模块: gui（图形用户界面层）
 * 功能描述: 声明 MainWindow 类 —— GUI 记账工具的主窗口。
 *
 * 主窗口是用户界面的"外壳"，负责:
 *   1. 提供左侧导航栏（滑动切换：一级菜单 ↔ 二级菜单）
 *   2. 管理右侧内容区域（使用 QStackedWidget 切换6个功能页面）
 *   3. 显示底部状态栏（实时显示总收入/总支出/结余数据）
 *
 * 编码格式: UTF-8
 * ===========================================================================
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>
#include <vector>

class Ledger;
class CategoryManager;
class HomePage;
class DashboardPage;
class FlowPage;
class StatisticsPage;
class CategoryPage;
class OtherPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Ledger& ledger, CategoryManager& catMan, QWidget *parent = nullptr);
    ~MainWindow();

    void refreshAll();

private slots:
    void switchToPage(int index);
    void updateStatusBar();
    void slideToSecondary();
    void slideToPrimary();

private:
    void setupUI();
    void setupSidebar(QVBoxLayout *sideLayout);
    void setupStatusBar();
    void setNavButtonActive(int index);

    Ledger& m_ledger;
    CategoryManager& m_catMan;

    QStackedWidget *m_stackedWidget;
    HomePage        *m_homePage;         // 索引 0: 入场首页
    DashboardPage   *m_dashboardPage;    // 索引 1: 概览
    FlowPage        *m_flowPage;         // 索引 2: 账目
    StatisticsPage  *m_statisticsPage;   // 索引 3: 统计
    CategoryPage    *m_categoryPage;     // 索引 4: 分类
    OtherPage       *m_otherPage;        // 索引 5: 其他功能

    std::vector<QPushButton*> m_navButtons;

    // 侧边栏滑动菜单
    QStackedWidget *m_slideArea;  // 滑动容器（截图叠加动画实现滑动）
    QWidget *m_primaryPage;     // 一级菜单页
    QWidget *m_secondaryPage;   // 二级菜单页
    bool m_showingPrimary = true;

    QLabel *m_statusIncome;
    QLabel *m_statusExpense;
    QLabel *m_statusBalance;
};

#endif // MAINWINDOW_H
