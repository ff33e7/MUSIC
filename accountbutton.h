// accountbutton.h
#ifndef ACCOUNTBUTTON_H
#define ACCOUNTBUTTON_H

#include <QPushButton>
#include <QIcon>
#include <QPixmap>
#include <QtCore/qpropertyanimation.h>

class AccountButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int rotation READ getRotation WRITE setRotation)

public:
    explicit AccountButton(QWidget *parent = nullptr);
    ~AccountButton();

    int getRotation() const { return rotation; }
    void setRotation(int angle);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    QColor normalColor = QColor(100, 100, 100); // Серый в обычном состоянии
    QColor hoverColor = QColor(0, 150, 136);    // Бирюзовый (#009688) при наведении

    QPixmap baseLayer;
    QPixmap discLayer;
    QIcon normalIcon;
    QIcon hoverIcon;

    int rotation = 0;
    QPropertyAnimation* rotateAnim = nullptr;

    void setupAppearance();
    QPixmap createBaseLayer();
    QPixmap createDisc(const QColor &color);
    QIcon combineLayers(int angle, const QColor &color);
    void updateIcon();
    void startRotationAnimation();
    void stopRotationAnimation();
};
#endif // ACCOUNTBUTTON_H
