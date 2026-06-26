/*
 * ============================================================================
 * 文件名: homepage.cpp
 * 模块:   入场首页（GUI实现文件）
 * 功能:   实现 HomePage —— 界面由 homepage.ui 定义。
 *         文字在页面显示时带有逐字淡入动画效果。
 * 编码:   UTF-8
 * ============================================================================
 */

#include "homepage.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QTimer>
#include <QHBoxLayout>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);

    // 标题三个字符的样式（缩小至原来的60%）
    const QString charStyle =
        "font-family: 'Microsoft YaHei';"
        "font-size: 48px;"
        "font-weight: bold;"
        "color: #2C3E50;"
        "background: transparent;";

    QLabel *charLabels[3] = {
        ui->charLabel_0, ui->charLabel_1, ui->charLabel_2
    };

    for (int i = 0; i < 3; i++) {
        charLabels[i]->setStyleSheet(charStyle);
        m_charEffects[i] = new QGraphicsOpacityEffect(charLabels[i]);
        m_charEffects[i]->setOpacity(0.0);
        charLabels[i]->setGraphicsEffect(m_charEffects[i]);
    }

    // 副标题样式和透明度
    ui->subtitleLabel->setStyleSheet(
        "font-family: 'Microsoft YaHei';"
        "font-size: 16px;"
        "color: #95A5A6;"
        "background: transparent;"
        "margin-top: 16px;");
    m_subtitleEffect = new QGraphicsOpacityEffect(ui->subtitleLabel);
    m_subtitleEffect->setOpacity(0.0);
    ui->subtitleLabel->setGraphicsEffect(m_subtitleEffect);

    setStyleSheet("background: transparent;");
}

void HomePage::fadeInLabel(QLabel *, QGraphicsOpacityEffect *effect,
                            int delayMs, int durationMs) {
    QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(durationMs);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    if (delayMs > 0) {
        QTimer::singleShot(delayMs, this, [anim]() {
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    } else {
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void HomePage::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    QLabel *charLabels[3] = {
        ui->charLabel_0, ui->charLabel_1, ui->charLabel_2
    };

    fadeInLabel(charLabels[0], m_charEffects[0],   0, 700);   // "小"
    fadeInLabel(charLabels[1], m_charEffects[1], 120, 700);   // "工"
    fadeInLabel(charLabels[2], m_charEffects[2], 240, 700);   // "具"

    fadeInLabel(ui->subtitleLabel, m_subtitleEffect, 500, 600);
}
