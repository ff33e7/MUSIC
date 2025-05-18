#ifndef LIKEBUTTON_H
#define LIKEBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class LikeButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

public:
    explicit LikeButton(QWidget *parent = nullptr);

    int yOffset() const;
    void setYOffset(int offset);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupAppearance();
    void updateIcon();
    void animatePress(bool pressed);

    bool isLiked = false;
    int m_yOffset = 0;

    QColor normalColor = QColor(60, 60, 60);
    QColor hoverColor = QColor(0, 150, 136);
    QColor likedColor = QColor(220, 20, 60);

    QPixmap createHeartIcon(const QColor &color);

    QPropertyAnimation *pressAnimation = nullptr;
};

#endif // LIKEBUTTON_H
