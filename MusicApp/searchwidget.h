#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QLineEdit>

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget();

    // Метод для установки имени трека
    void paintEvent(QPaintEvent *event) override;
    void setTrackName(const QString &trackPath);

    // Метод для получения виджета QLineEdit (если нужно в других местах использовать)
    QLineEdit* getSearchLineEdit();

protected:
    // Переопределение метода для обработки событий
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLineEdit *searchLineEdit; // Поле для ввода текста
};

#endif // SEARCHWIDGET_H
