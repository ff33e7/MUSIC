#include "woodbackgroundwidget.h"
#include <QPainter>

WoodBackgroundWidget::WoodBackgroundWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents); // Пропускает клики мыши
    setGeometry(0, 0, 1200, 250); // Размер и позиция виджета
}

void WoodBackgroundWidget::setImagePath(const QString &path)
{
    backgroundTexture = QPixmap(path);
    update(); // Перерисовать виджет
}

void WoodBackgroundWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setOpacity(0.1);

    // Проверяем, есть ли текстура
    if (!backgroundTexture.isNull()) {
        // Сохраняем пропорции и заполняем весь виджет
        QRectF targetRect = rect();
        QRectF sourceRect = backgroundTexture.rect();

        // Применяем метод растяжения с сохранением пропорций
        painter.drawPixmap(targetRect, backgroundTexture, sourceRect);
    }

    // Цветовой фильтр сверху (например, тёмно-серый с прозрачностью)
    QColor overlayColor(44, 44, 44, 100); // (R, G, B, Alpha)
    painter.fillRect(rect(), overlayColor);
}
