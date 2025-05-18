// circularprogressbar.cpp
#include "circularprogressbar.h"
#include <QPainter>
#include <QPen>
#include <QtMath>
#include <QMouseEvent>

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent)
    , m_progress(0)
    , m_thickness(6)
    , m_glowColor(Qt::cyan)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
}

void CircularProgressBar::setProgress(qreal value)
{
    m_progress = qBound(0.0, value, 1.0);
    update();
}

void CircularProgressBar::setThickness(int px)
{
    m_thickness = px;
    update();
}

void CircularProgressBar::setGlowColor(const QColor &color)
{
    m_glowColor = color;
    update();
}

void CircularProgressBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int side = qMin(w, h) - m_thickness;
    QRectF rect((w - side) / 2, (h - side) / 2, side, side);

    QPen pen(m_glowColor, m_thickness, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);

    // рисуем только верхнюю полуокружность: от 180° до 0° (по часовой стрелке)
    int startAngle = 180 * 16;
    int spanAngle  = -static_cast<int>(180 * 16 * m_progress);
    painter.drawArc(rect, startAngle, spanAngle);
}

void CircularProgressBar::mousePressEvent(QMouseEvent *event)
{
    handleMouse(event->pos());
}

void CircularProgressBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        handleMouse(event->pos());
}

void CircularProgressBar::handleMouse(const QPoint &pos)
{
    QPointF center(width()/2.0, height()/2.0);
    QPointF delta = pos - center;
    qreal dist  = std::hypot(delta.x(), delta.y());
    qreal radius = (qMin(width(), height()) - m_thickness) / 2.0;

    // Реагируем только если на «кольце» (+- толщину)
    if (qAbs(dist - radius) > m_thickness) return;

    // Вычисляем угол: 0° — вправо, 90° — вверх, 180° — влево, 270° — вниз
    qreal angle = std::atan2(-delta.y(), delta.x()) * 180.0 / M_PI;
    if (angle < 0) angle += 360.0;

    // Интересует именно верхняя полуокружность: углы от 0° до 180°
    if (angle <= 180.0) {
        // 180° (слева) → progress=0, 0° (справа) → progress=1
        qreal normalized = (180.0 - angle) / 180.0;
        setProgress(normalized);
        emit progressChanged(normalized);
    }
}
