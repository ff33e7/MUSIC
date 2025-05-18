// accountbutton.cpp
#include "accountbutton.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>

AccountButton::AccountButton(QWidget *parent) : QPushButton(parent)
{
    setupAppearance();

    baseLayer = createBaseLayer();
    discLayer = createDisc(normalColor);
    normalIcon = combineLayers(0, normalColor);
    hoverIcon = combineLayers(0, hoverColor);

    setIcon(normalIcon);
    setIconSize(QSize(64, 64));
    setFixedSize(80, 80);
    setCursor(Qt::PointingHandCursor);
}

AccountButton::~AccountButton()
{
    stopRotationAnimation();
    if (graphicsEffect()) {
        graphicsEffect()->deleteLater();
    }
}

QPixmap AccountButton::createBaseLayer()
{
    const int size = 128;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Центральная этикетка
    painter.setBrush(QColor(30, 30, 30));
    painter.drawEllipse(size/2 - 20, size/2 - 20, 40, 40);

    // Акцент
    painter.setBrush(Qt::white);
    painter.drawEllipse(size/2 - 8, size/2 - 8, 16, 16);

    // Тонарм (уменьшен)
    QPainterPath tonearmPath;
    tonearmPath.moveTo(25, 25);
    tonearmPath.lineTo(size - 40, size - 40);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawPath(tonearmPath);

    // Головка
    painter.setBrush(QColor(180, 180, 180));
    painter.drawEllipse(size - 44, size - 44, 10, 10);

    // Металл часть
    painter.setBrush(QColor(200, 200, 200));
    painter.drawEllipse(size - 48, size - 48, 6, 6);

    return pixmap;
}

QPixmap AccountButton::createDisc(const QColor &color)
{
    const int size = 128;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRadialGradient vinylGradient(size/2, size/2, size/2);
    vinylGradient.setColorAt(0, QColor(50, 50, 50));
    vinylGradient.setColorAt(1, QColor(20, 20, 20));

    painter.setBrush(vinylGradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(8, 8, size - 16, size - 16);

    // Дорожки
    painter.setPen(QPen(QColor(120, 120, 120, 150), 1.5));
    painter.setBrush(Qt::NoBrush);
    for (int r = 20; r < size/2 - 10; r += 8) {
        painter.drawEllipse(QPointF(size/2, size/2), r, r);
    }

    return pixmap;
}

QIcon AccountButton::combineLayers(int angle, const QColor &color)
{
    const int size = 128;
    QPixmap finalPixmap(size, size);
    finalPixmap.fill(Qt::transparent);

    QPainter painter(&finalPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Вращаем только диск
    QPixmap rotatedDisc(size, size);
    rotatedDisc.fill(Qt::transparent);
    {
        QPainter rotator(&rotatedDisc);
        rotator.setRenderHint(QPainter::Antialiasing, true);
        rotator.translate(size / 2, size / 2);
        rotator.rotate(angle);
        rotator.translate(-size / 2, -size / 2);
        rotator.drawPixmap(0, 0, discLayer);
    }

    painter.drawPixmap(0, 0, rotatedDisc);
    painter.drawPixmap(0, 0, baseLayer);

    return QIcon(finalPixmap);
}

void AccountButton::setupAppearance()
{
    setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   background: transparent;"
        "   border-radius: 40px;"
        "}"
        "QPushButton:hover {"
        "   background: rgba(0, 150, 136, 30); /* Бирюзовый с прозрачностью */"
        "}"
        );
}

void AccountButton::startRotationAnimation()
{
    if (rotateAnim) return;

    rotateAnim = new QPropertyAnimation(this, "rotation", this);
    rotateAnim->setDuration(2000);
    rotateAnim->setStartValue(0);
    rotateAnim->setEndValue(360);
    rotateAnim->setLoopCount(-1); // Бесконечная анимация
    rotateAnim->start();
}

void AccountButton::stopRotationAnimation()
{
    if (rotateAnim) {
        rotateAnim->stop();
        rotateAnim->deleteLater();
        rotateAnim = nullptr;
    }
    setRotation(0); // Сброс вращения
}

void AccountButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    setIcon(hoverIcon);

    // Бирюзовая тень при наведении
    auto *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(15);
    shadow->setColor(hoverColor); // #009688
    shadow->setOffset(0, 0);
    setGraphicsEffect(shadow);

    // Анимация увеличения
    auto *anim = new QPropertyAnimation(this, "iconSize");
    anim->setDuration(200);
    anim->setStartValue(QSize(64, 64));
    anim->setEndValue(QSize(72, 72));
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // Запуск анимации вращения
    startRotationAnimation();
}

void AccountButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    setIcon(normalIcon);
    setGraphicsEffect(nullptr);

    auto *anim = new QPropertyAnimation(this, "iconSize");
    anim->setDuration(200);
    anim->setStartValue(QSize(72, 72));
    anim->setEndValue(QSize(64, 64));
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // Остановка анимации вращения
    stopRotationAnimation();
}

void AccountButton::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
}

void AccountButton::setRotation(int angle)
{
    rotation = angle;
    updateIcon();
}

void AccountButton::updateIcon()
{
    setIcon(combineLayers(rotation, underMouse() ? hoverColor : normalColor));
}
