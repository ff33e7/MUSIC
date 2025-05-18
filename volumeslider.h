#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include <QColor>

class VolumeSlider : public QSlider
{
    Q_OBJECT

public:
    explicit VolumeSlider(QWidget *parent = nullptr);

    void setColors(const QColor& track, const QColor& fill, const QColor& handle);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;


private:
    void drawGroove(QPainter *painter);
    void drawHandle(QPainter *painter);

    QRectF grooveRect() const;
    QRectF handleRect() const;
    qreal handlePosition() const;

    // Размеры
    qreal m_trackWidth = 24;
    qreal m_handleWidth = 60;
    qreal m_handleHeight = 20;
    qreal m_cornerRadius = 8.0;

    // Состояния
    bool m_hovered = false;

    // Цвета
    QColor m_trackColor = QColor(80, 80, 80);
    QColor m_fillColor = QColor(120, 200, 255);
    QColor m_handleColor = QColor(200, 200, 200);
};

#endif // VOLUMESLIDER_H
