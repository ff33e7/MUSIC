#ifndef HOMEBUTTON_H
#define HOMEBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class HomeButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

public:
    explicit HomeButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupAppearance();
    QPixmap createNoteIcon(const QColor &color);
    void updateIcon();
    void animatePress(bool pressed);

    int yOffset() const;
    void setYOffset(int offset);

private:
    int m_yOffset = 0;
    QPropertyAnimation *pressAnimation;

    QColor normalColor = QColor(60, 60, 60);
    QColor hoverColor = QColor(64, 224, 208); // Бирюзовый
};

#endif // HOMEBUTTON_H
