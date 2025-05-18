#include "searchwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QLineEdit>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QFileInfo>

SearchWidget::SearchWidget(QWidget *parent) : QWidget(parent)
{
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setFocusPolicy(Qt::ClickFocus);
    searchLineEdit->setPlaceholderText("Введите название трека...");
    searchLineEdit->setGeometry(10, 10, 800, 50);
    searchLineEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #333;
            color: white;
            border: 2px solid #666;
            border-radius: 20px;
            padding: 0 20px;
            font-size: 14px;
        }
        QLineEdit:hover {
            border-color: #009688; /* Бирюзовый при наведении */
        }
        QLineEdit:focus {
            border-color: #009688; /* Бирюзовый при фокусе */
            background-color: #444;
        }
    )");
    searchLineEdit->setAlignment(Qt::AlignCenter);

    // Устанавливаем фильтр событий для очистки при клике
    searchLineEdit->installEventFilter(this);
}

SearchWidget::~SearchWidget()
{
    delete searchLineEdit;
}

QLineEdit* SearchWidget::getSearchLineEdit()
{
    return searchLineEdit;
}

void SearchWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Рисуем фон поля
    QRectF rect(10, 10, 800, 50);
    qreal radius = 20.0;

    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);

    painter.setBrush(QColor(50, 50, 50));
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    painter.drawPath(path);
}

bool SearchWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == searchLineEdit) {
        if (event->type() == QEvent::FocusIn) {
            // Очистить строку при получении фокуса
            searchLineEdit->clear();
        }
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (searchLineEdit->hasFocus() && !searchLineEdit->text().isEmpty()) {
                searchLineEdit->clear();  // Очистка текста при клике
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void SearchWidget::setTrackName(const QString &trackPath)
{
    // Убираем расширение файла
    QFileInfo trackInfo(trackPath);
    QString trackNameWithoutExtension = trackInfo.baseName(); // Имя без расширения
    searchLineEdit->setText(trackNameWithoutExtension); // Устанавливаем имя без расширения в поле
}
