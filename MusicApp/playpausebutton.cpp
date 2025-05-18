#include "playpausebutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QGraphicsOpacityEffect>

PlayPauseButton::PlayPauseButton(QWidget *parent) : QPushButton(parent)
{
    setupAppearance();

    pressAnimation = new QPropertyAnimation(this, "yOffset");
    pressAnimation->setDuration(100);
    pressAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    setFixedSize(64, 64);
    setCursor(Qt::PointingHandCursor);

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(opacityEffect);
}

void PlayPauseButton::setupAppearance()
{
    setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #222222, stop:1 #000000);"
        "   border: 1px solid #b0b0b0;"
        "   border-radius: 12px;"
        "}"
        );
}

void PlayPauseButton::setPlaying(bool playing)
{
    if (m_playing != playing) {
        m_playing = playing;
        updateIcon();
    }

    setButtonVisible(!m_playing);
}

void PlayPauseButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.translate(0, m_yOffset);

    QColor iconColor = m_hovered ? hoverColor : normalColor;
    QPixmap icon = createPlayPauseIcon(iconColor);

    int x = (width() - icon.width()) / 2;
    int y = (height() - icon.height()) / 2;

    painter.drawPixmap(x, y, icon);
}

QPixmap PlayPauseButton::createPlayPauseIcon(const QColor &color)
{
    const int size = 128;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    if (m_playing) {
        // Pause icon
        path.addRect(size * 0.3, size * 0.2, size * 0.1, size * 0.6);
        path.addRect(size * 0.6, size * 0.2, size * 0.1, size * 0.6);
    } else {
        // Play icon
        path.moveTo(size * 0.35, size * 0.25);
        path.lineTo(size * 0.7, size * 0.5);
        path.lineTo(size * 0.35, size * 0.75);
        path.closeSubpath();
    }

    painter.drawPath(path);
    return pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void PlayPauseButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    animatePress(true);
}

void PlayPauseButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    if (rect().contains(event->pos())) {
        setPlaying(!m_playing);
        emit clicked(m_playing);
    }
    animatePress(false);
}

void PlayPauseButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    m_hovered = true;
    updateIcon();
}

void PlayPauseButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    m_hovered = false;
    updateIcon();
}

void PlayPauseButton::updateIcon()
{
    update(); // repaint using new state
}

void PlayPauseButton::animatePress(bool pressed)
{
    pressAnimation->stop();
    pressAnimation->setStartValue(m_yOffset);
    pressAnimation->setEndValue(pressed ? 3 : 0);
    pressAnimation->start();
}

int PlayPauseButton::yOffset() const
{
    return m_yOffset;
}

void PlayPauseButton::setYOffset(int offset)
{
    m_yOffset = offset;
    update();
}

void PlayPauseButton::setButtonVisible(bool visible)
{
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(graphicsEffect());
    if (!effect) return;

    effect->setOpacity(visible ? 1.0 : 0.0);
    emit visibilityChanged(visible);  // добавили сигнал
}
