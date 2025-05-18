#ifndef FAVORITEBUTTON_H
#define FAVORITEBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class FavoriteButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal glowIntensity READ glowIntensity WRITE setGlowIntensity)

public:
    explicit FavoriteButton(QWidget *parent = nullptr);
    void setLiked(bool liked);
    bool isLiked() const { return m_liked; }

    qreal glowIntensity() const { return m_glowIntensity; }
    void setGlowIntensity(qreal intensity);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    void setupAppearance();
    void animateClick();

    bool m_liked = false;
    qreal m_glowIntensity = 0.0;
    QColor m_baseColor = QColor(255, 60, 90); // Красный/розовый цвет сердца
    QPropertyAnimation *m_glowAnimation;
};

#endif // FAVORITEBUTTON_H
