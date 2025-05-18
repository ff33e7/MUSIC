#ifndef WOODBACKGROUNDWIDGET_H
#define WOODBACKGROUNDWIDGET_H

#include <QWidget>
#include <QPixmap>

class WoodBackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WoodBackgroundWidget(QWidget *parent = nullptr);
    void setImagePath(const QString &path);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap backgroundTexture;
};

#endif // WOODBACKGROUNDWIDGET_H
