/*
 * ============================================================================
 * 文件名: homepage.h
 * 模块:   入场首页（GUI头文件）
 * 功能:   声明 HomePage 类 —— 程序启动时显示的入场首页。
 *         页面正中显示"小工具"三个大字，下方副标题"欢迎使用"。
 *         所有文字在页面显示时带有淡入动画效果。
 *         界面由 homepage.ui 定义，可通过 Qt Designer 可视化编辑。
 * 编码:   UTF-8
 * ============================================================================
 */

#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "ui_homepage.h"

class QGraphicsOpacityEffect;

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void fadeInLabel(QLabel *label, QGraphicsOpacityEffect *effect,
                     int delayMs, int durationMs);

    Ui::HomePage *ui;

    QGraphicsOpacityEffect *m_charEffects[3];  // 三个字的透明度效果
    QGraphicsOpacityEffect *m_subtitleEffect;  // 副标题透明度效果
};

#endif // HOMEPAGE_H
