// tonearm.cpp
#include "tonearm.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QtGui/qpainterpath.h>

Tonearm::Tonearm(QWidget *parent)
    : QWidget(parent), m_angle(0.0),
    m_offsetX(-100), m_offsetY(0) {
    setFixedSize(600, 600);  // Увеличенный виджет
    setAttribute(Qt::WA_OpaquePaintEvent);
}

qreal Tonearm::angle() const {
    return m_angle;
}

void Tonearm::setAngle(qreal angle) {
    if (qFuzzyCompare(m_angle, angle))
        return;
    m_angle = angle;
    emit angleChanged(m_angle);
    update();
}

void Tonearm::setOffset(qreal x, qreal y) {
    m_offsetX = x;
    m_offsetY = y;
    update();
}

void Tonearm::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Центр вращения + смещение
    QPointF pivot(200 + m_offsetX, 400 + m_offsetY);

    painter.translate(pivot);
    painter.rotate(-60 + m_angle);
    painter.translate(-pivot);




    // === 3. Основная трубка ===
    QPointF p1 = pivot;
    QPointF bend1 = p1 + QPointF(120, -12);
    QPointF bend2 = bend1 + QPointF(110, 32);
    QPointF p2 = bend2 + QPointF(55, 0);

    QPainterPath armPath;
    armPath.moveTo(p1);
    armPath.lineTo(bend1);
    armPath.lineTo(bend2);
    armPath.lineTo(p2);

    QLinearGradient tubeGrad(p1, p2);
    tubeGrad.setColorAt(0, QColor(130, 130, 130));
    tubeGrad.setColorAt(0.5, QColor(210, 210, 210));
    tubeGrad.setColorAt(1, QColor(100, 100, 100));

    painter.setPen(QPen(QBrush(tubeGrad), 14, Qt::SolidLine, Qt::RoundCap));
    painter.drawPath(armPath);

    // === 4. Хэдшелл ===
    QRectF headShell(p2.x(), p2.y() - 16, 60, 32);
    QLinearGradient shellGrad(headShell.topLeft(), headShell.bottomRight());
    shellGrad.setColorAt(0, QColor(240, 240, 240));
    shellGrad.setColorAt(1, QColor(90, 90, 90));

    painter.setBrush(shellGrad);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRoundedRect(headShell, 2, 2);

    // Перфорация хэдшелла
    painter.setBrush(Qt::black);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 6; ++col) {
            QPointF holePos = headShell.topLeft() + QPointF(8 + col * 8, 7 + row * 7);
            painter.drawEllipse(holePos, 1.8, 1.8);
        }
    }
}
