#ifndef PREVIOUSBUTTON_H
#define PREVIOUSBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class PreviousButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

public:
    explicit PreviousButton(QWidget *parent = nullptr);

    int yOffset() const;
    void setYOffset(int offset);
    void setButtonVisible(bool visible);



    int diskOffset() const;
    void setDiskOffset(int offset);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
signals:
    void visibilityChanged(bool visible);  // Сигнал для уведомления

public slots:
    void setButtonVisibleExternally(bool visible);  // Слот для подключения извне

private:
    void setupAppearance();
    QPixmap createPreviousIcon(const QColor &color);
    void animatePress(bool pressed);

    QPropertyAnimation *pressAnimation;
    int m_yOffset = 0;
    bool m_hovered = false;

    const QColor normalColor = QColor("#e0e0e0");
    const QColor hoverColor = QColor("#00bfa5");
};

#endif // PREVIOUSBUTTON_H
