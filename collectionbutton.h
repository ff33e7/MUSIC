#ifndef COLLECTIONBUTTON_H
#define COLLECTIONBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class CollectionButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

public:
    explicit CollectionButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void collectionRequested();

private:
    void setupAppearance();
    QPixmap createBookmarkIcon(const QColor &color);
    void updateIcon();
    void animatePress(bool pressed);

    int yOffset() const;
    void setYOffset(int offset);

private:
    bool isCollected = false;
    int m_yOffset = 0;
    QPropertyAnimation *pressAnimation;

    QColor normalColor = QColor(60, 60, 60);
    QColor hoverColor = QColor(0, 200, 200, 200); // Бирюзовый
    QColor collectedColor = QColor(255, 200, 0, 200); // Золотой
};

#endif // COLLECTIONBUTTON_H
