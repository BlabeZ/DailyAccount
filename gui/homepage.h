/*
 * ============================================================================
 * 文件名: homepage.h
 * 模块:   启动首页（GUI头文件）
 * 功能:   声明 HomePage 类 —— 程序启动时显示的欢迎首页。
 *         页面正中显示"记账本"三个大字，无数据依赖。
 *         用户点击任意侧边栏导航后离开此页，且不会再返回。
 * 编码:   UTF-8
 * ============================================================================
 */

#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

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

    QLabel *m_charLabels[3];     // "记" "账" "本"
    QGraphicsOpacityEffect *m_charEffects[3];
    QLabel *m_subtitleLabel;
    QGraphicsOpacityEffect *m_subtitleEffect;
};

#endif // HOMEPAGE_H
