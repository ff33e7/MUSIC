#ifndef VINYLWIDGET_H
#define VINYLWIDGET_H

#include <QWidget>
#include <QImage>
#include <QTimer>

class VinylWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VinylWidget(QWidget *parent = nullptr);

    void setCoverImage(const QImage &cover);
    void startSpinning();
    void stopSpinning();
    void resetRotation();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_cover;
    QTimer m_timer;
    qreal m_rotationAngle = 0.0;
};

#endif // VINYLWIDGET_H
