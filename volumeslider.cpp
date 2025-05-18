#include "volumeslider.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

VolumeSlider::VolumeSlider(QWidget *parent) : QSlider(parent)
{
    setOrientation(Qt::Vertical);
    setRange(0, 100);
    setFixedWidth(65);
    setFixedHeight(360);
    setCursor(Qt::PointingHandCursor);
}

void VolumeSlider::setColors(const QColor& track, const QColor& fill, const QColor& handle)
{
    m_trackColor = track;
    m_fillColor = fill;
    m_handleColor = handle;
    update();
}

void VolumeSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    drawGroove(&painter);
    drawHandle(&painter);
}

void VolumeSlider::drawGroove(QPainter *painter)
{
    const QRectF rect = grooveRect();

    QLinearGradient trackGrad(rect.topLeft(), rect.bottomLeft());
    trackGrad.setColorAt(0, m_trackColor.darker(120));
    trackGrad.setColorAt(1, m_trackColor);

    painter->setPen(Qt::NoPen);
    painter->setBrush(trackGrad);
    painter->drawRoundedRect(rect, m_cornerRadius, m_cornerRadius);

    const qreal fillHeight = height() - handlePosition() - m_handleHeight / 2;
    QRectF fillRect = rect;
    fillRect.setTop(fillRect.bottom() - fillHeight);

    QLinearGradient fillGrad(fillRect.topLeft(), fillRect.bottomLeft());
    qreal volumePercentage = static_cast<qreal>(value()) / maximum();

    fillGrad.setColorAt(0.0, m_fillColor);
    fillGrad.setColorAt(0.3, m_fillColor.darker(100 + volumePercentage * 30));
    fillGrad.setColorAt(1.0, m_fillColor.darker(100 + volumePercentage * 60));

    painter->setBrush(fillGrad);
    painter->drawRoundedRect(fillRect, m_cornerRadius, m_cornerRadius);

    painter->setPen(QPen(QColor(255, 255, 255, 30), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), m_cornerRadius, m_cornerRadius);
}

void VolumeSlider::drawHandle(QPainter *painter)
{
    const QRectF handle = handleRect();

    // УДАЛЕН эффект свечения при наведении

    // Handle shadow
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 40));
    painter->drawRoundedRect(handle.translated(0, 1.5), m_cornerRadius, m_cornerRadius);

    // Handle body with gradient
    QLinearGradient handleGrad(handle.topLeft(), handle.bottomLeft());
    handleGrad.setColorAt(0, m_handleColor.lighter(110));
    handleGrad.setColorAt(1, m_handleColor.darker(110));

    painter->setPen(QPen(QColor(80, 80, 80), 1.0));
    painter->setBrush(handleGrad);
    painter->drawRoundedRect(handle, m_cornerRadius, m_cornerRadius);

    // Handle center indicator
    painter->setPen(QPen(QColor(60, 60, 60), 1.2));
    painter->setBrush(QColor(120, 120, 120));
    painter->drawEllipse(handle.center(), 3, 3);
}

QRectF VolumeSlider::grooveRect() const
{
    const qreal margin = m_handleHeight / 2;
    return QRectF(width() / 2 - m_trackWidth / 2, margin,
                  m_trackWidth, height() - m_handleHeight);
}

QRectF VolumeSlider::handleRect() const
{
    const qreal pos = handlePosition();
    return QRectF(width() / 2 - m_handleWidth / 2, pos,
                  m_handleWidth, m_handleHeight);
}

qreal VolumeSlider::handlePosition() const
{
    const qreal percentage = 1.0 - static_cast<qreal>(value() - minimum()) / (maximum() - minimum());
    return percentage * (height() - m_handleHeight);
}

void VolumeSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const int newPos = qBound(0, event->pos().y(), height());
        const qreal percentage = 1.0 - static_cast<qreal>(newPos) / height();
        setValue(minimum() + percentage * (maximum() - minimum()));
    }
    QSlider::mousePressEvent(event);
}

void VolumeSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        const int newPos = qBound(0, event->pos().y(), height());
        const qreal percentage = 1.0 - static_cast<qreal>(newPos) / height();
        setValue(minimum() + percentage * (maximum() - minimum()));
    }
    QSlider::mouseMoveEvent(event);
}

void VolumeSlider::wheelEvent(QWheelEvent *event)
{
    const int delta = event->angleDelta().y() > 0 ? 5 : -5;
    setValue(qBound(minimum(), value() + delta, maximum()));
    event->accept();
}

void VolumeSlider::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QSlider::enterEvent(event);
}

void VolumeSlider::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QSlider::leaveEvent(event);
}

