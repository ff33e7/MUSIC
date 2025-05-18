#include "likebutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QEnterEvent>
#include <QMouseEvent>

LikeButton::LikeButton(QWidget *parent) : QPushButton(parent)
{
    setupAppearance();
    updateIcon();

    pressAnimation = new QPropertyAnimation(this, "yOffset");
    pressAnimation->setDuration(100);
    pressAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    setFixedSize(64, 64);
    setCursor(Qt::PointingHandCursor);
}

void LikeButton::setupAppearance()
{
    setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #222222, stop:1 #000000);"
        "   border: 1px solid #b0b0b0;"
        "   border-radius: 12px;"
        "}"
        );
}

QPixmap LikeButton::createHeartIcon(const QColor &color)
{
    const int size = 128;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    QPainterPath heart;
    heart.moveTo(size / 2, size * 0.7);
    heart.cubicTo(size * 0.15, size * 0.5, size * 0.25, size * 0.2, size / 2, size * 0.35);
    heart.cubicTo(size * 0.75, size * 0.2, size * 0.85, size * 0.5, size / 2, size * 0.7);
    painter.drawPath(heart);

    return pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void LikeButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.translate(0, m_yOffset);

    QPixmap heart = createHeartIcon(isLiked ? likedColor : (underMouse() ? hoverColor : normalColor));
    int x = (width() - heart.width()) / 2;
    int y = (height() - heart.height()) / 2;

    painter.drawPixmap(x, y, heart);
}

void LikeButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    animatePress(true);
}

void LikeButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);

    if (rect().contains(event->pos())) {
        isLiked = !isLiked;
        updateIcon();
    }
    animatePress(false);
}

void LikeButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    updateIcon();
}

void LikeButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    updateIcon();
}

void LikeButton::updateIcon()
{
    update(); // Просто перерисовываем, всё логика в paintEvent
}

void LikeButton::animatePress(bool pressed)
{
    pressAnimation->stop();
    pressAnimation->setStartValue(m_yOffset);
    pressAnimation->setEndValue(pressed ? 3 : 0);
    pressAnimation->start();
}

int LikeButton::yOffset() const
{
    return m_yOffset;
}

void LikeButton::setYOffset(int offset)
{
    m_yOffset = offset;
    update();
}
