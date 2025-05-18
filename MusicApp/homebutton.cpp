#include "homebutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>

HomeButton::HomeButton(QWidget *parent) : QPushButton(parent)
{
    setupAppearance();
    updateIcon();

    pressAnimation = new QPropertyAnimation(this, "yOffset");
    pressAnimation->setDuration(100);
    pressAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    setFixedSize(64, 64);
    setCursor(Qt::PointingHandCursor);
}

void HomeButton::setupAppearance()
{
    setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #222222, stop:1 #000000);"
        "   border: 1px solid #b0b0b0;"
        "   border-radius: 12px;"
        "}"
        );
}

QPixmap HomeButton::createNoteIcon(const QColor &color)
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Настройка кисти и пера
    QPen pen(color);
    pen.setWidth(3);  // Толщина для ножек
    painter.setPen(pen);
    painter.setBrush(color);

    // Размеры
    int noteWidth = 10;
    int noteHeight = 10;
    int stemHeight = 20;
    int beamHeight = 5;

    // Координаты левой ноты
    int leftX = 14;
    int leftY = 36;

    // Левая головка
    painter.drawEllipse(QRectF(leftX, leftY, noteWidth, noteHeight));
    // Левая ножка справа от головки
    painter.drawLine(QPointF(leftX + noteWidth, leftY + noteHeight / 2),
                     QPointF(leftX + noteWidth, leftY - stemHeight + noteHeight / 2));

    // Координаты правой ноты
    int rightX = 34;
    int rightY = 36;

    // Правая головка
    painter.drawEllipse(QRectF(rightX, rightY, noteWidth, noteHeight));
    // Правая ножка справа от головки
    painter.drawLine(QPointF(rightX + noteWidth, rightY + noteHeight / 2),
                     QPointF(rightX + noteWidth, rightY - stemHeight + noteHeight / 2));

    // Перекладина (beam)
    painter.fillRect(QRectF(leftX + noteWidth, leftY - stemHeight + noteHeight / 2,
                            (rightX + noteWidth) - (leftX + noteWidth), beamHeight), color);

    return pixmap;
}



void HomeButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.translate(0, m_yOffset);

    QPixmap icon = createNoteIcon(underMouse() ? hoverColor : normalColor);
    int x = (width() - icon.width()) / 2;
    int y = (height() - icon.height()) / 2;

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(x, y, icon);
}

void HomeButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    animatePress(true);
}

void HomeButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    animatePress(false);
}

void HomeButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    updateIcon();
}

void HomeButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    updateIcon();
}

void HomeButton::updateIcon()
{
    update();
}

void HomeButton::animatePress(bool pressed)
{
    pressAnimation->stop();
    pressAnimation->setStartValue(m_yOffset);
    pressAnimation->setEndValue(pressed ? 3 : 0);
    pressAnimation->start();
}

int HomeButton::yOffset() const
{
    return m_yOffset;
}

void HomeButton::setYOffset(int offset)
{
    m_yOffset = offset;
    update();
}
