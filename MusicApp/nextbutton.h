#ifndef NEXTBUTTON_H
#define NEXTBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>
#include <QColor>

class NextButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

public:
    explicit NextButton(QWidget *parent = nullptr);
    int yOffset() const;
    void setYOffset(int offset);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

public slots:
    void setButtonVisibleExternally(bool visible);

private:
    void setupAppearance();
    QPixmap createNextIcon(const QColor &color);
    void animatePress(bool pressed);

    QPropertyAnimation *pressAnimation = nullptr;
    int m_yOffset = 0;
    bool m_hovered = false;

    const QColor normalColor = QColor("#e0e0e0");
    const QColor hoverColor = QColor("#00bfa5");

    void setButtonVisible(bool visible);
};

#endif // NEXTBUTTON_H
