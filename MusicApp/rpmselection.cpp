#include "rpmselection.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QPushButton>
#include <sqlite3.h>
#include <QDebug>
#include <QCoreApplication>
#include <QPropertyAnimation>
#include <QKeyEvent>
#include <QSettings>
#include <QTimer>
#include <QApplication>
#include <QMouseEvent>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QDir>



// Класс всплывающего окна, позиционируемого относительно виджета-опоры
class PositionedPopup : public QWidget {
public:
    // Конструктор: задаёт безрамочное всплывающее окно
    PositionedPopup(QWidget* anchor, QWidget* parent = nullptr)
        : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool), anchorWidget(anchor) {
        setAttribute(Qt::WA_ShowWithoutActivating);  // Не захватывать фокус
        setFocusPolicy(Qt::NoFocus);
    }

protected:
    // При показе окна — позиционируем его под виджетом-опорой
    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        if (anchorWidget) {
            QPoint globalPos = anchorWidget->mapToGlobal(
                QPoint(anchorWidget->width() / 2 - width() / 2, anchorWidget->height()));
            move(globalPos);
        }
    }

private:
    QWidget* anchorWidget; // Виджет, относительно которого позиционируется окно
};

// Основной класс выбора RPM (жанров/тегов)
RPMSelection::RPMSelection(QWidget *parent)
    : QWidget(parent)
{
    // Создаём кнопку вызова выбора RPM
    mainButton = new QPushButton("RPM Selection");
    // mainButton->setStyleSheet("QPushButton { "
    //                           "background: transparent; "
    //                           "border: none; "
    //                           "font-size: 30px; "  // Увеличиваем размер шрифта
    //                           "color: #1abc9c; "   // Устанавливаем бирюзовый цвет текста
    //                           "}");

    mainButton->setStyleSheet(R"(
    QPushButton {
        background-color: rgba(255, 255, 255, 0.15);
        border: 1px solid rgba(255, 255, 255, 0.25);
        border-radius: 16px;
        color: white;
        font-size: 20px;
        padding: 10px 20px;
        backdrop-filter: blur(10px);
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2);
    }

    QPushButton:hover {
        background-color: rgba(255, 255, 255, 0.25);
        border: 1px solid rgba(255, 255, 255, 0.35);
        box-shadow: 0 6px 20px rgba(0, 255, 255, 0.3);
        color: #1abc9c;
    }

    QPushButton:pressed {
        background-color: rgba(255, 255, 255, 0.1);
        box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.3);
    }
)");


    mainButton->setCursor(Qt::PointingHandCursor);
    connect(mainButton, &QPushButton::clicked, this, &RPMSelection::showPopup);


    // Главное окно (Кнопка RPMSelection)
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mainButton, 0, Qt::AlignHCenter);
    mainButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred); // Размер по горизонтали
    mainButton->setFixedHeight(50); // Размер по вертикали
    setLayout(mainLayout);

    // Создаём всплывающее окно и настраиваем его
    popupWidget = new PositionedPopup(mainButton);

    popupWidget->setFixedSize(300, 250);
    QVBoxLayout *popupLayout = new QVBoxLayout(popupWidget);
    popupLayout->setContentsMargins(10, 10, 10, 10);

    // Поле ввода тегов
    input = new QLineEdit;
    input->setPlaceholderText("Введите тег...");
    connect(input, &QLineEdit::selectionChanged, this, &RPMSelection::showPopup);
    input->installEventFilter(this);
    popupLayout->addWidget(input);

    // Список предложений тегов
    suggestionsList = new QListWidget;
    suggestionsList->setVisible(false);
    connect(suggestionsList, &QListWidget::itemClicked, this, &RPMSelection::addTag);
    popupLayout->addWidget(suggestionsList);

    // КНОПКА СБРОСА ВЫБРАННЫХ ЖАНРОВ
    clearLine = new QFrame;
    clearLine->setFixedHeight(6);     // высота
    clearLine->setMaximumWidth(300);  // ширина
    clearLine->setCursor(Qt::PointingHandCursor);
    clearLine->setStyleSheet(R"(
    QFrame {
        background: qlineargradient(
            x1: 0, y1: 0, x2: 1, y2: 0,
            stop: 0 transparent,
            stop: 0.5 red,
            stop: 1 transparent
        );
        border: none;
        margin-left: 20px;
        margin-right: 20px;
    }
)");

    popupLayout->addWidget(clearLine);

    // Обработка клика по линии
    clearLine->installEventFilter(this); // Добавляем обработку событий


    // КОНТЕЙНЕР С ТЕГАМИ
    QWidget *tagContainer = new QWidget;
    tagsLayout = new QVBoxLayout(tagContainer);
    tagsLayout->setContentsMargins(0, 5, 0, 0);
    tagsLayout->setSpacing(4);

    // Прокручиваемая область
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidget(tagContainer);
    scrollArea->setMaximumHeight(120); // Ограничим высоту

    popupLayout->addWidget(scrollArea);

    // Доступные теги
    // /////////////////////////////////////////////////////////////////////
    // ------------------- !!! УКАЗАТЬ ПУТЬ К БД !!! -------------------- //
    // /////////////////////////////////////////////////////////////////////
    QString dbPath = "/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/tracks.db";
    allTags = loadTagsFromDatabase(dbPath);

    // /////////////////////////////////////////////////////////////////
    // ----------- !!! УКАЗАТЬ ПУТЬ К .txt с тегами !!! ------------- //
    // /////////////////////////////////////////////////////////////////
    tagFilePath = "/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/tags.txt";
    loadSelectedTags();

    // Глобальный фильтр событий для обработки кликов вне popup
    QApplication::instance()->installEventFilter(this);

    // Фильтрация тегов при изменении текста
    connect(input, &QLineEdit::textChanged, this, &RPMSelection::filterTags);
}

RPMSelection::~RPMSelection() {
    saveSelectedTags(); // сохранение при завершении
}

// Показ всплывающего окна
void RPMSelection::showPopup()
{
    // Получаем глобальные координаты кнопки (или поля ввода)

    int verticalOffset = 12; // можно менять, например, 8–20
    QPoint globalPos = mainButton->mapToGlobal(
        QPoint(mainButton->width() / 2 - popupWidget->width() / 2,
               mainButton->height() + verticalOffset));

    popupWidget->move(globalPos);
    popupWidget->show();
    popupWidget->raise();
    popupWidget->activateWindow();

    filterTags(input->text()); // обновить список предложений
    input->setFocus();         // чтобы курсор сразу был в поле
}

// Фильтрация и отображение подходящих тегов
void RPMSelection::filterTags(const QString &text)
{
    suggestionsList->clear();

    for (const QString &tag : allTags) {
        if (!selectedTags.contains(tag) &&
            (text.isEmpty() || tag.contains(text, Qt::CaseInsensitive))) {
            suggestionsList->addItem(tag);
        }
    }

    suggestionsList->setVisible(suggestionsList->count() > 0);
}

// Добавление нового тега
void RPMSelection::addTag(QListWidgetItem *item)
{
    QString tag = item->text();
    if (selectedTags.contains(tag))
        return;

    selectedTags.insert(tag);
    saveSelectedTags();

    // Добавляем сохранение
    if (!isLoadingTags) {
        saveSelectedTags();  // сохраняем сразу при выборе нового тега
    }

    // Виджет для нового тега
    QLabel *label = new QLabel(tag);
    label->setStyleSheet("QLabel { border-bottom: 1px solid gray; padding: 2px; margin-right: 5px; }");
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QPushButton *removeBtn = new QPushButton("✕");
    removeBtn->setFixedSize(16, 16);
    removeBtn->setStyleSheet("QPushButton { border: none; color: red; font-weight: bold; }");

    QHBoxLayout *tagRow = new QHBoxLayout;
    tagRow->addWidget(label);
    tagRow->addStretch();
    tagRow->addWidget(removeBtn);
    tagRow->setContentsMargins(0, 0, 0, 0);

    QWidget *tagWidget = new QWidget;
    tagWidget->setLayout(tagRow);
    tagWidget->setMaximumHeight(0);  // начинаем с высоты 0

    tagsLayout->addWidget(tagWidget);

    // Анимация появления
    QPropertyAnimation *animShow = new QPropertyAnimation(tagWidget, "maximumHeight");
    animShow->setDuration(200);
    animShow->setStartValue(0);
    animShow->setEndValue(30);  // финальная высота
    animShow->start(QAbstractAnimation::DeleteWhenStopped);

    // Удаление тега
    connect(removeBtn, &QPushButton::clicked, this, [=]() {
        selectedTags.remove(tag);
        saveSelectedTags(); // !!!!!!!!!!!!!!!!!!!!!!

        // Анимация исчезновения
        QPropertyAnimation *animHide = new QPropertyAnimation(tagWidget, "maximumHeight");
        animHide->setDuration(200);
        animHide->setStartValue(tagWidget->height());
        animHide->setEndValue(0);

        connect(animHide, &QPropertyAnimation::finished, tagWidget, &QWidget::deleteLater);
        animHide->start(QAbstractAnimation::DeleteWhenStopped);

        filterTags(input->text());  // обновить список предложений

        saveSelectedTagsToFile(tagFilePath);
    });

    input->clear();
    if (!isLoadingTags) {
        showPopup();  // только при ручном добавлении
    }
}

// Загрузка всех жанров из базы данных SQLite
QStringList RPMSelection::loadTagsFromDatabase(const QString &dbPath)
{
    QStringList tags;
    sqlite3 *db;
    if (sqlite3_open(dbPath.toStdString().c_str(), &db) != SQLITE_OK) {
        qWarning() << "Ошибка открытия БД:" << sqlite3_errmsg(db);
        return tags;
    }

    const char *sql = "SELECT name FROM genres;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *text = sqlite3_column_text(stmt, 0);
            if (text) {
                tags << QString::fromUtf8(reinterpret_cast<const char *>(text));
            }
        }
    } else {
        qWarning() << "Ошибка выполнения запроса:" << sqlite3_errmsg(db);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tags;
}

// Обработка событий: клавиатура, мышь и т.д.
bool RPMSelection::eventFilter(QObject *obj, QEvent *event)
{
    // Навигация по списку предложений
    if (obj == input && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Down) {
            suggestionsList->setFocus();
            suggestionsList->setCurrentRow(0);
            return true;
        }

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (suggestionsList->count() > 0) {
                addTag(suggestionsList->item(0));
                return true;
            }
        }
    }

    if (obj == input && event->type() == QEvent::FocusIn) {
        showPopup();
    }

    // Закрытие popup при клике вне него
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QWidget *clickedWidget = QApplication::widgetAt(mouseEvent->globalPosition().toPoint());

        if (popupWidget->isVisible() &&
            clickedWidget != popupWidget &&
            !popupWidget->isAncestorOf(clickedWidget) &&
            clickedWidget != mainButton &&
            !mainButton->isAncestorOf(clickedWidget)) {
            popupWidget->hide();
        }

        if (clickedWidget == clearLine) {
            clearAllTags();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

// Сохраняем выбранные теги в QSettings и файл
void RPMSelection::saveSelectedTags() {
    QSettings settings("MyCompany", "AppMusic");
    settings.beginGroup("RPMSelection");
    settings.setValue("tags", QStringList(selectedTags.begin(), selectedTags.end()));
    settings.endGroup();

    // Сохраняем также в .txt файл рядом с приложением
    saveSelectedTagsToFile(tagFilePath);
}

// Загружаем ранее выбранные теги из настроек
void RPMSelection::loadSelectedTags() {
    isLoadingTags = true;

    QSettings settings("MyCompany", "AppMusic");
    settings.beginGroup("RPMSelection");
    QStringList savedTags = settings.value("tags").toStringList();
    settings.endGroup();

    for (const QString &tag : savedTags) {
        if (!selectedTags.contains(tag)) {
            QListWidgetItem *dummy = new QListWidgetItem(tag);
            addTag(dummy);
            delete dummy;
        }
    }

    isLoadingTags = false;
}

// СБРОС ВЫБРАННЫХ ЖАНРОВ
void RPMSelection::clearAllTags() {
    // Удаляем виджеты тегов с анимацией
    for (int i = tagsLayout->count() - 1; i >= 0; --i) {
        QWidget *tagWidget = tagsLayout->itemAt(i)->widget();
        if (tagWidget) {
            QPropertyAnimation *animHide = new QPropertyAnimation(tagWidget, "maximumHeight");
            animHide->setDuration(200);
            animHide->setStartValue(tagWidget->height());
            animHide->setEndValue(0);

            connect(animHide, &QPropertyAnimation::finished, tagWidget, &QWidget::deleteLater);
            animHide->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    selectedTags.clear();

    saveSelectedTags();

    filterTags(input->text()); // обновить предложения
}

// Сохранение выбранных тегов в .txt
void RPMSelection::saveSelectedTagsToFile(const QString& filePath) {
    qDebug() << "Сохраняем в файл: " << tagFilePath;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString &tag : selectedTags) {
            out << tag << "\n";
        }
    }
}
