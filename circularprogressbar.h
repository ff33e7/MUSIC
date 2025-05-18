// circularprogressbar.h
#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

#include <QWidget>
#include <QColor>
#include <QPoint>

class CircularProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit CircularProgressBar(QWidget *parent = nullptr);

    void setProgress(qreal value); // от 0.0 до 1.0
    void setThickness(int px);
    void setGlowColor(const QColor &color);

signals:
    void progressChanged(qreal progress);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void handleMouse(const QPoint &pos);

    qreal m_progress;
    int m_thickness;
    QColor m_glowColor;
};

#endif // CIRCULARPROGRESSBAR_H
