#include "nextbutton.h"
#include <QPainter>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QtWidgets/qgraphicseffect.h>

NextButton::NextButton(QWidget *parent) : QPushButton(parent)
{
    setupAppearance();

    pressAnimation = new QPropertyAnimation(this, "yOffset");
    pressAnimation->setDuration(100);
    pressAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    setFixedSize(64, 64);
    setCursor(Qt::PointingHandCursor);
}

void NextButton::setupAppearance()
{
    setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #222222, stop:1 #000000);"
        "   border: 1px solid #b0b0b0;"
        "   border-radius: 12px;"
        "}"
        );
}

QPixmap NextButton::createNextIcon(const QColor &color)
{
    const int size = 128;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    // Две стрелки >> (Next)
    QPointF points1[3] = {
        QPointF(size * 0.35, size * 0.3),
        QPointF(size * 0.55, size * 0.5),
        QPointF(size * 0.35, size * 0.7)
    };
    QPointF points2[3] = {
        QPointF(size * 0.55, size * 0.3),
        QPointF(size * 0.75, size * 0.5),
        QPointF(size * 0.55, size * 0.7)
    };
    painter.drawPolygon(points1, 3);
    painter.drawPolygon(points2, 3);

    return pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void NextButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.translate(0, m_yOffset);

    QPixmap icon = createNextIcon(m_hovered ? hoverColor : normalColor);
    int x = (width() - icon.width()) / 2;
    int y = (height() - icon.height()) / 2;

    painter.drawPixmap(x, y, icon);
}

void NextButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    animatePress(true);
}

void NextButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    animatePress(false);
}


void NextButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    m_hovered = true;
    update();
}

void NextButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    m_hovered = false;
    update();
}

void NextButton::animatePress(bool pressed)
{
    pressAnimation->stop();
    pressAnimation->setStartValue(m_yOffset);
    pressAnimation->setEndValue(pressed ? 3 : 0);
    pressAnimation->start();
}

int NextButton::yOffset() const
{
    return m_yOffset;
}

void NextButton::setYOffset(int offset)
{
    m_yOffset = offset;
    update();
}

void NextButton::setButtonVisible(bool visible)
{
    if (!graphicsEffect()) {
        auto *effect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(effect);
    }
    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(graphicsEffect());
    effect->setOpacity(visible ? 1.0 : 0.0);
}

void NextButton::setButtonVisibleExternally(bool visible)
{
    setButtonVisible(visible);
}
