// tonearm.h
#ifndef TONEARM_H
#define TONEARM_H

#include <QWidget>

class Tonearm : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle NOTIFY angleChanged)

public:
    explicit Tonearm(QWidget *parent = nullptr);
    qreal angle() const;
    void setAngle(qreal angle);

    void setOffset(qreal x, qreal y);  // добавляем метод смещения

signals:
    void angleChanged(qreal angle);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_angle;
    qreal m_offsetX;  // смещение X
    qreal m_offsetY;  // смещение Y
};

#endif // TONEARM_H
