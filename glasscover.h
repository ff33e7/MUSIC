#ifndef GLASSCOVER_H
#define GLASSCOVER_H

#include <QWidget>
#include <QPropertyAnimation>

class GlassCover : public QWidget
{
    Q_OBJECT

public:
    QPropertyAnimation* animation() const { return m_animation; }
    bool isOpen() const { return m_isOpen; }
    explicit GlassCover(QWidget *parent = nullptr);

    void open();
    void close();
    void moveToClosedPosition();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPropertyAnimation *m_animation;
    bool m_isOpen = false;
};

#endif // GLASSCOVER_H
