/*
 * ============================================================================
 * 文件名: homepage.cpp
 * 模块:   入场首页（GUI实现文件）
 * 功能:   实现 HomePage —— 居中显示"小工具"三个大字，字间距30px+。
 *         文字在页面显示时带有淡入动画效果。
 *         本页是入场首页，无数据加载逻辑，不依赖 Ledger 或 CategoryManager。
 * 编码:   UTF-8
 * ============================================================================
 */

#include "homepage.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QTimer>
#include <QHBoxLayout>

HomePage::HomePage(QWidget *parent) : QWidget(parent) {
    // 主垂直布局 —— 内容水平和垂直均居中
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // ======== 标题行：三个独立字符 "小" "工" "具"，水平居中 ========
    QHBoxLayout *titleRow = new QHBoxLayout;
    titleRow->setAlignment(Qt::AlignCenter);
    titleRow->setSpacing(36);  // 字间距 36px

    const QString chars[] = {
        QStringLiteral("小"),
        QStringLiteral("工"),
        QStringLiteral("具")
    };

    const QString charStyle =
        "font-family: 'Microsoft YaHei';"
        "font-size: 80px;"
        "font-weight: bold;"
        "color: #2C3E50;"
        "background: transparent;";

    for (int i = 0; i < 3; i++) {
        m_charLabels[i] = new QLabel(chars[i]);
        m_charLabels[i]->setStyleSheet(charStyle);
        m_charLabels[i]->setAlignment(Qt::AlignCenter);
        m_charLabels[i]->setFixedWidth(100);  // 固定宽度，保证字间距稳定

        // 透明度效果 —— 初始不可见
        m_charEffects[i] = new QGraphicsOpacityEffect(m_charLabels[i]);
        m_charEffects[i]->setOpacity(0.0);
        m_charLabels[i]->setGraphicsEffect(m_charEffects[i]);

        titleRow->addWidget(m_charLabels[i]);
    }

    mainLayout->addLayout(titleRow);

    // ======== 副标题：欢迎语 ========
    m_subtitleLabel = new QLabel(QStringLiteral("欢迎使用"));
    m_subtitleLabel->setStyleSheet(
        "font-family: 'Microsoft YaHei';"
        "font-size: 16px;"
        "color: #95A5A6;"
        "background: transparent;"
        "margin-top: 16px;");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    // 副标题透明度效果 —— 初始不可见
    m_subtitleEffect = new QGraphicsOpacityEffect(m_subtitleLabel);
    m_subtitleEffect->setOpacity(0.0);
    m_subtitleLabel->setGraphicsEffect(m_subtitleEffect);

    mainLayout->addWidget(m_subtitleLabel);

    setStyleSheet("background: transparent;");
}

/*
 * fadeInLabel — 对单个标签启动淡入动画
 *
 * 参数:
 *   label     — 目标标签（用于设置 parent 生命周期）
 *   effect    — 该标签的透明度效果
 *   delayMs   — 延迟毫秒数
 *   durationMs— 动画持续时间毫秒数
 */
void HomePage::fadeInLabel(QLabel * /*label*/, QGraphicsOpacityEffect *effect,
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

/*
 * showEvent — 页面首次显示时触发逐字淡入动画
 *
 * 动画节奏:
 *   "小"  —— 立即开始, 700ms
 *   "工"  —— 延迟 120ms, 700ms
 *   "具"  —— 延迟 240ms, 700ms
 *   副标题 —— 延迟 500ms, 600ms
 *
 *   三个字依次出现形成"书写"般的渐入效果，
 *   副标题最后浮现，完成整个欢迎序列。
 *
 *   使用 QEasingCurve::OutCubic —— 开始快、结尾慢，视觉上更自然。
 */
void HomePage::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    // 三个字符依次淡入
    fadeInLabel(m_charLabels[0], m_charEffects[0],   0, 700);   // "小"
    fadeInLabel(m_charLabels[1], m_charEffects[1], 120, 700);   // "工"
    fadeInLabel(m_charLabels[2], m_charEffects[2], 240, 700);   // "具"

    // 副标题延迟后淡入
    fadeInLabel(m_subtitleLabel, m_subtitleEffect, 500, 600);
}
