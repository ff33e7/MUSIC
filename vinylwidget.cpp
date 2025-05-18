#include "vinylwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QConicalGradient>
#include <QRadialGradient>
#include <QtMath>

VinylWidget::VinylWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);

    connect(&m_timer, &QTimer::timeout, this, [this]() {
        m_rotationAngle += 1.0;
        if (m_rotationAngle >= 360.0)
            m_rotationAngle -= 360.0;
        update();
    });

    m_timer.setInterval(16); // ~60 FPS
}

void VinylWidget::setCoverImage(const QImage &cover) {
    m_cover = cover;
    update();
}

void VinylWidget::startSpinning() {
    if (!m_timer.isActive())
        m_timer.start();
}

void VinylWidget::stopSpinning() {
    m_timer.stop();
}

void VinylWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int margin = 50;
    int side = qMin(width(), height()) - margin;
    QRectF rect((width() - side) / 2.0, (height() - side) / 2.0, side, side);

    QPointF center = rect.center();

    // Вращение винила и обложки
    painter.translate(center);
    painter.rotate(m_rotationAngle);
    painter.translate(-center);

    // Верхняя половина винила
    QPainterPath path;
    path.moveTo(center);
    path.arcTo(rect, 0, 360);
    path.closeSubpath();

    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);

    // Рельефные бороздки
    int grooveCount = 30;
    for (int i = 0; i < grooveCount; ++i) {
        qreal ratio = 1.0 - (i + 1) * 0.015;
        QRectF grooveRect(
            center.x() - rect.width() * ratio / 2,
            center.y() - rect.height() * ratio / 2,
            rect.width() * ratio,
            rect.height() * ratio
            );
        QColor grooveColor(80, 80, 80, 50);
        painter.setPen(QPen(grooveColor, 0.8));
        painter.drawArc(grooveRect, 0 * 16, 360 * 16);
    }

    // Световой блик
    QRadialGradient gradient(center, side / 2);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 30));
    gradient.setColorAt(1.0, Qt::transparent);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);

    // Серебряные 3D-обводки
    qreal innerRingR = rect.width() / 2.0 + 1;
    qreal outerRingR = rect.width() / 2.0 + 11;

    auto draw3DRing = [&](qreal r1, qreal r2) {
        QConicalGradient grad(center, 90);
        grad.setColorAt(0.0, QColor(220, 220, 220));
        grad.setColorAt(0.25, QColor(150, 150, 150));
        grad.setColorAt(0.5, QColor(220, 220, 220));
        grad.setColorAt(0.75, QColor(180, 180, 180));
        grad.setColorAt(1.0, QColor(220, 220, 220));

        QPainterPath ringPath;
        ringPath.addEllipse(center, r2, r2);
        ringPath.addEllipse(center, r1, r1);
        painter.setBrush(grad);
        painter.setPen(Qt::NoPen);
        painter.drawPath(ringPath.simplified());
    };

    draw3DRing(rect.width() / 2.0 - 3, innerRingR);
    draw3DRing(outerRingR, outerRingR + 10);

    // Точки по краю
    int dotCount = 320;
    qreal outerRadius = rect.width() / 2.0 + 8;
    qreal innerRadius = rect.width() / 2.0 + 3;
    qreal dotRadius = 1.5;

    qreal dotRingInner = innerRadius - dotRadius;
    qreal dotRingOuter = outerRadius + dotRadius;

    // Фон под точки
    QPainterPath dotBackground;
    dotBackground.addEllipse(center, dotRingOuter, dotRingOuter);
    dotBackground.addEllipse(center, dotRingInner, dotRingInner);
    painter.setBrush(QColor(30, 30, 50));
    painter.setPen(Qt::NoPen);
    painter.drawPath(dotBackground.simplified());

    // Точки по окружности
    painter.setBrush(Qt::white);
    for (int i = 0; i < dotCount; ++i) {
        // полный круг: 2 * M_PI
        qreal angle = 2 * M_PI * i / dotCount;

        qreal x1 = center.x() + outerRadius * qCos(angle);
        qreal y1 = center.y() - outerRadius * qSin(angle);
        painter.drawEllipse(QPointF(x1, y1), dotRadius, dotRadius);
        qreal angleShifted = angle + M_PI / dotCount;
        qreal x2 = center.x() + innerRadius * qCos(angleShifted);
        qreal y2 = center.y() - innerRadius * qSin(angleShifted);
        painter.drawEllipse(QPointF(x2, y2), dotRadius, dotRadius);
    }

    // Обложка трека
    if (!m_cover.isNull()) {
        qreal coverRadius = rect.width() * 0.27;
        QRectF coverRect(center.x() - coverRadius, center.y() - coverRadius,
                         coverRadius * 2, coverRadius * 2);

        QImage scaledCover = m_cover.scaled(coverRect.size().toSize(),
                                            Qt::KeepAspectRatioByExpanding,
                                            Qt::SmoothTransformation);

        QPainterPath maskPath;
        maskPath.addEllipse(coverRect);
        painter.setClipPath(maskPath);
        painter.drawImage(coverRect, scaledCover);
        painter.setClipping(false);
    }

    // Центральная игла
    int spindleRadius = side / 90;
    QRectF spindleRect(center.x() - spindleRadius, center.y() - spindleRadius,
                       spindleRadius * 2, spindleRadius * 2);
    QRadialGradient spindleGradient(center, spindleRadius * 2);
    spindleGradient.setColorAt(0, QColor("#f0f0f0"));
    spindleGradient.setColorAt(1, QColor("#999999"));
    painter.setBrush(spindleGradient);
    painter.setPen(QPen(Qt::black, 0.5));
    painter.drawEllipse(spindleRect);
}

void VinylWidget::resetRotation()
{
    m_rotationAngle = 0;  // или как ты хранишь текущий угол
    update();             // перерисовать
}
