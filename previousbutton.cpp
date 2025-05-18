#include "previousbutton.h"
#include <QPainter>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>

PreviousButton::PreviousButton(QWidget *parent) : QPushButton(parent)
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

void PreviousButton::setupAppearance()
{
    setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #222222, stop:1 #000000);"
        "   border: 1px solid #b0b0b0;"
        "   border-radius: 12px;"
        "}"
        );
}

QPixmap PreviousButton::createPreviousIcon(const QColor &color)
{
    const int size = 128;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    QPointF points1[3] = {
        QPointF(size * 0.65, size * 0.3),
        QPointF(size * 0.45, size * 0.5),
        QPointF(size * 0.65, size * 0.7)
    };
    QPointF points2[3] = {
        QPointF(size * 0.45, size * 0.3),
        QPointF(size * 0.25, size * 0.5),
        QPointF(size * 0.45, size * 0.7)
    };
    painter.drawPolygon(points1, 3);
    painter.drawPolygon(points2, 3);

    return pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void PreviousButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.translate(0, m_yOffset);

    QPixmap icon = createPreviousIcon(m_hovered ? hoverColor : normalColor);
    int x = (width() - icon.width()) / 2;
    int y = (height() - icon.height()) / 2;

    painter.drawPixmap(x, y, icon);
}

void PreviousButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    animatePress(true);
}

void PreviousButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    animatePress(false);
}


void PreviousButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    m_hovered = true;
    update();
}

void PreviousButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    m_hovered = false;
    update();
}

void PreviousButton::animatePress(bool pressed)
{
    pressAnimation->stop();
    pressAnimation->setStartValue(m_yOffset);
    pressAnimation->setEndValue(pressed ? 3 : 0);
    pressAnimation->start();
}

int PreviousButton::yOffset() const
{
    return m_yOffset;
}

void PreviousButton::setYOffset(int offset)
{
    m_yOffset = offset;
    update();
}

void PreviousButton::setButtonVisible(bool visible)
{
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(effect);
    }

    effect->setOpacity(visible ? 1.0 : 0.0);  // Управление прозрачностью
}

// --- Вот это добавляем для подключения извне
void PreviousButton::setButtonVisibleExternally(bool visible)
{
    setButtonVisible(visible);
}
