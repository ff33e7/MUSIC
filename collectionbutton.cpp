#include "collectionbutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>

CollectionButton::CollectionButton(QWidget *parent) : QPushButton(parent)
{
    setupAppearance();
    updateIcon();

    pressAnimation = new QPropertyAnimation(this, "yOffset");
    pressAnimation->setDuration(100);
    pressAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    setFixedSize(64, 64);
    setCursor(Qt::PointingHandCursor);
}

// "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4d4d4d, stop:1 #2a2a2a);"

void CollectionButton::setupAppearance()
{
    setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #222222, stop:1 #000000);"
        "   border: 1px solid #b0b0b0;"
        "   border-radius: 12px;"
        "}"
        );
}

QPixmap CollectionButton::createBookmarkIcon(const QColor &color)
{
    const int size = 128;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    QPainterPath bookmark;
    bookmark.moveTo(size * 0.3, size * 0.2);
    bookmark.lineTo(size * 0.7, size * 0.2);
    bookmark.lineTo(size * 0.7, size * 0.8);
    bookmark.lineTo(size * 0.5, size * 0.65);
    bookmark.lineTo(size * 0.3, size * 0.8);
    bookmark.closeSubpath();

    painter.drawPath(bookmark);

    return pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CollectionButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.translate(0, m_yOffset);

    QPixmap icon = createBookmarkIcon(underMouse() ? hoverColor : normalColor);
    int x = (width() - icon.width()) / 2;
    int y = (height() - icon.height()) / 2;

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Обычное наложение
    painter.drawPixmap(x, y, icon);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void CollectionButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    animatePress(true);
}

void CollectionButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);

    if (rect().contains(event->pos())) {
        isCollected = !isCollected;
        updateIcon();
        emit collectionRequested(); // Добавляем вызов сигнала
    }
    animatePress(false);
}

void CollectionButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    updateIcon();
}

void CollectionButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    updateIcon();
}

void CollectionButton::updateIcon()
{
    update();
}

void CollectionButton::animatePress(bool pressed)
{
    pressAnimation->stop();
    pressAnimation->setStartValue(m_yOffset);
    pressAnimation->setEndValue(pressed ? 3 : 0);
    pressAnimation->start();
}

int CollectionButton::yOffset() const
{
    return m_yOffset;
}

void CollectionButton::setYOffset(int offset)
{
    m_yOffset = offset;
    update();
}
