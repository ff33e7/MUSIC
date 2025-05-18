#include "glasscover.h"
#include <QPainter>
#include <QDebug>

GlassCover::GlassCover(QWidget *parent) : QWidget(parent),
    m_animation(new QPropertyAnimation(this, "pos", this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1200, 800);

    m_animation->setDuration(1000);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);

    raise();
}

void GlassCover::moveToClosedPosition()
{
    if (!parentWidget()) return;
    move(parentWidget()->width() - width(), parentWidget()->height() - height());
    qDebug() << "GlassCover moved to closed position:" << pos();
}

void GlassCover::open()
{
    if (m_animation->state() == QAbstractAnimation::Running) return;

    QWidget* parent = parentWidget();
    if (!parent) return;

    QPoint closedPos(parent->width() - width(), parent->height() - height());

    int dx = +(width());   // вправо
    QPoint openPos = closedPos + QPoint(dx, 0);

    m_animation->setStartValue(this->pos());
    m_animation->setEndValue(openPos);
    m_animation->start();

    m_isOpen = true;

    qDebug() << "GlassCover open() called.";
    qDebug() << "ClosedPos:" << closedPos;
    qDebug() << "OpenPos:" << openPos;
}

void GlassCover::close()
{
    if (m_animation->state() == QAbstractAnimation::Running) return;
    if (!parentWidget()) return;

    QPoint closedPos(parentWidget()->width() - width(), parentWidget()->height() - height());

    m_animation->setStartValue(this->pos());
    m_animation->setEndValue(closedPos);
    m_animation->start();

    m_isOpen = false;

    qDebug() << "GlassCover close() called. Moving to:" << closedPos;
}

void GlassCover::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // --- GLASS RECT (более красивый стеклянный эффект) ---
    QRectF glassRect(0, 0, width(), height());

    QLinearGradient glassGrad(glassRect.topLeft(), glassRect.bottomRight());
    glassGrad.setColorAt(0.0, QColor(240, 250, 255, 80));   // светлый голубой верх
    glassGrad.setColorAt(0.4, QColor(200, 220, 240, 120));  // более насыщенный центр
    glassGrad.setColorAt(1.0, QColor(160, 190, 220, 100));  // мягкий низ

    p.setBrush(glassGrad);
    p.setPen(QPen(QColor(180, 200, 220, 100), 2));   // мягкая стеклянная рамка
    p.drawRoundedRect(glassRect, 15, 15);            // скругления

    // --- LIGHT REFLECTION ---
    QLinearGradient reflectionGrad(glassRect.topLeft(), glassRect.bottomLeft());
    reflectionGrad.setColorAt(0.0, QColor(255, 255, 255, 80));
    reflectionGrad.setColorAt(0.3, QColor(255, 255, 255, 20));
    reflectionGrad.setColorAt(1.0, Qt::transparent);

    p.setBrush(reflectionGrad);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(glassRect, 15, 15);

    // --- HINGE (Петля) ---
    QRectF hingeRect(width() - 50, height() - 150, 50, 150);
    QLinearGradient hingeGrad(hingeRect.topLeft(), hingeRect.bottomRight());
    hingeGrad.setColorAt(0.0, QColor(100, 100, 100));
    hingeGrad.setColorAt(0.5, QColor(180, 180, 180));
    hingeGrad.setColorAt(1.0, QColor(230, 230, 230));

    p.setBrush(hingeGrad);
    p.setPen(QPen(QColor(60, 60, 60), 1));
    p.drawRoundedRect(hingeRect, 8, 8);

    // --- Bolts на петле ---
    p.setBrush(QColor(80, 80, 80));
    p.setPen(Qt::NoPen);
    p.drawEllipse(hingeRect.adjusted(10, 20, -10, -hingeRect.height()/2 + 10).topLeft(), 6, 6);
    p.drawEllipse(hingeRect.adjusted(10, hingeRect.height()/2 + 10, -10, -20).topLeft(), 6, 6);
}
