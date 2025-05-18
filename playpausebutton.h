#ifndef PLAYPAUSEBUTTON_H
#define PLAYPAUSEBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>

class PlayPauseButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

public:
    explicit PlayPauseButton(QWidget *parent = nullptr);
    void setPlaying(bool playing);  // Управляет состоянием (играет/паузит)

    int yOffset() const;
    void setYOffset(int offset);
    void setButtonVisible(bool visible);  // Устанавливает видимость кнопки

signals:
    void clicked(bool playing);  // Специфичный сигнал с состоянием
    void visibilityChanged(bool visible);  // Для изменения видимости

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupAppearance();  // Настройка внешнего вида кнопки
    QPixmap createPlayPauseIcon(const QColor &color);  // Создание иконки
    void updateIcon();  // Обновление иконки
    void animatePress(bool pressed);  // Анимация нажатия

    bool m_playing = false;  // Флаг, играет ли музыка
    bool m_hovered = false;  // Флаг, нажат ли на кнопку
    int m_yOffset = 0;  // Смещение по оси Y для анимации

    QPropertyAnimation *pressAnimation = nullptr;  // Анимация нажатия

    const QColor normalColor = QColor("#e0e0e0");
    const QColor hoverColor = QColor("#00bfa5");
};

#endif // PLAYPAUSEBUTTON_H
