#include "favoritebutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QEasingCurve>

FavoriteButton::FavoriteButton(QWidget *parent) : QPushButton(parent)
{
    setFixedSize(50, 50);
    setupAppearance();
    m_glowAnimation = new QPropertyAnimation(this, "glowIntensity", this);
    m_glowAnimation->setDuration(300);
}

void FavoriteButton::setupAppearance()
{
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   background: transparent;"
        "}"
        );
}

void FavoriteButton::setLiked(bool liked)
{
    m_liked = liked;
    update();

    // Анимация при изменении состояния
    QPropertyAnimation *anim = new QPropertyAnimation(this, "glowIntensity", this);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->setStartValue(m_glowIntensity);
    anim->setEndValue(liked ? 1.0 : 0.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FavoriteButton::setGlowIntensity(qreal intensity)
{
    m_glowIntensity = intensity;
    update();
}

void FavoriteButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const qreal size = qMin(width(), height()) * 0.7;
    const QRectF heartRect(width()/2 - size/2, height()/2 - size/2, size, size);

    // Эффект свечения
    if (m_glowIntensity > 0) {
        QRadialGradient glow(heartRect.center(), size/2 * 1.5);
        glow.setColorAt(0, QColor(255, 100, 100, 150 * m_glowIntensity));
        glow.setColorAt(1, Qt::transparent);

        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(heartRect.center(), size/2 * 1.2, size/2 * 1.2);
    }

    // Рисуем сердечко
    QPainterPath heartPath;
    heartPath.moveTo(heartRect.center().x(), heartRect.center().y() + size/4);

    heartPath.cubicTo(
        heartRect.center().x() + size/2, heartRect.center().y() - size/2,
        heartRect.center().x(), heartRect.center().y() - size,
        heartRect.center().x(), heartRect.center().y() - size/3
        );

    heartPath.cubicTo(
        heartRect.center().x(), heartRect.center().y() - size,
        heartRect.center().x() - size/2, heartRect.center().y() - size/2,
        heartRect.center().x(), heartRect.center().y() + size/4
        );

    // Градиент для сердечка
    QLinearGradient grad(heartRect.topLeft(), heartRect.bottomRight());
    QColor heartColor = m_liked ? m_baseColor : QColor(150, 150, 150);
    grad.setColorAt(0, heartColor.lighter(120));
    grad.setColorAt(1, heartColor.darker(m_liked ? 120 : 100));

    painter.setPen(QPen(QColor(80, 80, 80), 0.5));
    painter.setBrush(grad);
    painter.drawPath(heartPath);
}

void FavoriteButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    m_glowAnimation->stop();
    m_glowAnimation->setStartValue(m_glowIntensity);
    m_glowAnimation->setEndValue(0.3);
    m_glowAnimation->start();
}

void FavoriteButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    if (!m_liked) {
        m_glowAnimation->stop();
        m_glowAnimation->setStartValue(m_glowIntensity);
        m_glowAnimation->setEndValue(0.0);
        m_glowAnimation->start();
    }
}

void FavoriteButton::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
    setLiked(!m_liked);
    animateClick();
}

void FavoriteButton::animateClick()
{
    QPropertyAnimation *scaleAnim = new QPropertyAnimation(this, "geometry", this);
    scaleAnim->setDuration(200);
    scaleAnim->setEasingCurve(QEasingCurve::OutBack);

    QRect startRect = geometry();
    QRect scaledRect = QRect(
        startRect.x() + startRect.width() * 0.1,
        startRect.y() + startRect.height() * 0.1,
        startRect.width() * 0.8,
        startRect.height() * 0.8
        );

    scaleAnim->setStartValue(scaledRect);
    scaleAnim->setEndValue(startRect);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
