#include "mainwindow.h"
#include "circularprogressbar.h"
#include "ui_mainwindow.h"
#include "collectionbutton.h"
#include "homebutton.h"
#include "rpmselection.h"
#include "likebutton.h"
#include "nextbutton.h"
#include "playpausebutton.h"
#include "previousbutton.h"
#include "volumeslider.h"
#include "glasscover.h"
#include "woodbackgroundwidget.h"

#include <QDir>
#include <QUrl>
#include <QPainter>
#include <QDebug>
#include <QPropertyAnimation>
#include <QTimer>
#include <QCloseEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mpegfile.h>



// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Инициализация UI из формы .ui
    qApp->installEventFilter(this); // Установка фильтра событий на приложение
    setFixedSize(1200, 800); // Фиксированный размер окна
    setStyleSheet("background-color: #2c2c2c;"); // Фоновый цвет

    // === Фон в виде деревянной текстуры ===
    WoodBackgroundWidget *woodBg = new WoodBackgroundWidget(this);
    woodBg->setGeometry(0, 0, 1200, 800); // Занимает всё окно
    woodBg->setImagePath("/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/images/gray-wood-texture.jpg");
    woodBg->show(); // Отобразить фоновый виджет
    woodBg->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // === Кнопки навигации: предыдущий, воспроизведение/пауза, следующий ===
    PreviousButton *prevBtn = new PreviousButton(this);
    prevBtn->move(495, 650); // Позиционирование
    connect(prevBtn, &PreviousButton::clicked,this, &MainWindow::playPreviousTrack); // Обработчик клика

    PlayPauseButton *playBtn = new PlayPauseButton(this);
    playBtn->move(575, 650);

    NextButton *nextBtn = new NextButton(this);
    nextBtn->move(655, 650);
    connect(nextBtn, &NextButton::clicked, this, &MainWindow::playNextTrack);

    // === Кнопка "лайк" ===
    LikeButton *likeBtn = new LikeButton(this);
    likeBtn->move(13, 650);

    // === Кнопка коллекции ===
    CollectionButton *collectBtn = new CollectionButton(this);
    collectBtn->move(13, 200);

    // === Кнопка "домой" ===
    HomeButton *homeBtn = new HomeButton(this);
    homeBtn->move(13, 120);

    // === Виджет выбора жанров (RPM) ===
    RPMSelection *rpmWidget = new RPMSelection;
    layout()->addWidget(rpmWidget);
    rpmWidget->setParent(this);
    rpmWidget->show();
    rpmWidget->move(275, -20);

    connect(collectBtn, &CollectionButton::collectionRequested,
            this, &MainWindow::showCollectionPage);

    // === Поисковый виджет ===
    searchWidget = new SearchWidget(this);
    searchWidget->setFixedSize(900, 100);
    repositionSearchWidget(); // Центрировать по X
    searchWidget->raise(); // Поверх остальных виджетов

    // Скрывать/показывать кнопки prev/next в зависимости от playBtn
    connect(playBtn, &PlayPauseButton::visibilityChanged,
            prevBtn, &PreviousButton::setButtonVisibleExternally);
    connect(playBtn, &PlayPauseButton::visibilityChanged,
            nextBtn, &NextButton::setButtonVisibleExternally);

    // === Виниловый проигрыватель и тонарм ===
    vinyl = new VinylWidget(this);
    vinyl->setFixedSize(1200, 900);
    repositionVinyl();

    tonearm = new Tonearm(this);
    tonearm->setFixedSize(600, 600);
    tonearm->setStyleSheet("background-color: rgba(0, 255, 0, 50);"); // Полупрозрачный для отладки
    repositionTonearm();
    tonearm->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // === Кнопка аккаунта ===
    accountButton = new AccountButton(this);
    accountButton->move(width() - accountButton->width() - 20, 20);
    accountButton->raise(); // Поверх всего

    // === Регулировка громкости ===
    VolumeSlider *volumeSlider = new VolumeSlider(this);
    volumeSlider->setColors(QColor(40, 40, 40), QColor(0, 180, 160), Qt::white);
    volumeSlider->move(1100, 400);
    volumeSlider->setValue(50);
    connect(volumeSlider, &QSlider::valueChanged, this, [=](int value) {
        m_audioOutput->setVolume(static_cast<qreal>(value) / 100.0);
    });

    // === Стеклянная крышка (анимация открытия/закрытия) ===
    glassCover = new GlassCover(this);
    glassCover->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    glassCover->show();
    glassCover->open();

    // Поверхность тонарма и кнопок
    tonearm->raise();
    playBtn->raise();
    nextBtn->raise();
    prevBtn->raise();
    likeBtn->raise();
    volumeSlider->raise();

    // === Инициализация аудио ===
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.5);

    // /////////////////////////////////////////////////////////////////////
    // ------------------ !!! УКАЗАТЬ ПУТЬ К БД !!! --------------------- //
    // /////////////////////////////////////////////////////////////////////
    QString dbPath = "/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/tracks.db";
    loadTracksFromDatabase(dbPath);

    // Отслеживание изменения файла с тегами
    tagFileWatcher = new QFileSystemWatcher(this);
    tagFileWatcher->addPath(tagsFilePath);

    connect(tagFileWatcher, &QFileSystemWatcher::fileChanged, this, [=](const QString &path) {
        qDebug() << "Файл тегов изменён, обновляем список треков...";
        loadTracksFromDatabase("/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/tracks.db");

        // Переустановим слежение, так как fileChanged срабатывает один раз
        tagFileWatcher->addPath(tagsFilePath);

        // Проверим, остался ли текущий трек в списке
        if (m_currentTrackIndex >= m_trackList.size()) {
            m_currentTrackIndex = 0;
        }

        if (!m_trackList.isEmpty()) {
            QString currentPath = m_player->source().toLocalFile();
            if (!m_trackList.contains(currentPath)) {
                qDebug() << "Текущий трек больше не соответствует фильтру — переключаемся.";
                playCurrentTrack();  // Запускаем первый подходящий трек
            }
        } else {
            qDebug() << "Нет подходящих треков по новым тегам.";
            m_player->stop();
            vinyl->stopSpinning();
            isPlaying = false;
        }
    });

    // Обработка клика play/pause
    connect(playBtn, &PlayPauseButton::clicked, this, [=](bool playing) {
        if (playing) {
            playCurrentTrack(); // Запуск воспроизведения
        } else {
            isPlaying = false;
            m_player->pause();
            vinyl->stopSpinning(); // Остановка вращения винила при паузе
        }
    });

    // === Прогресс-бар воспроизведения (круговой) ===
    progressBar = new CircularProgressBar(this);
    progressBar->resize(vinyl->size());
    progressBar->move(0, 350);
    progressBar->setGlowColor(QColor(0, 255, 200));
    progressBar->setThickness(8);
    // progressBar->setAttribute(Qt::WA_TransparentForMouseEvents);
    progressBar->raise();

    // Таймер для обновления прогресса
    QTimer *progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &MainWindow::updateTrackProgress);
    progressTimer->start(100); // Обновляем каждую 0.1 секунды

    // Перемотка трека через прогресс-бар
    connect(progressBar, &CircularProgressBar::progressChanged, this, [=](qreal progress) {
        if (m_player->duration() > 0) {
            qint64 newPosition = static_cast<qint64>(progress * m_player->duration());
            m_player->setPosition(newPosition);
        }
    });

    progressBar->stackUnder(tonearm);

    // === лэйбл для скраббинга ===
    m_scrubTimeLabel = new QLabel(this);
    m_scrubTimeLabel->setStyleSheet(
        "color: white;"
        "background: rgba(0,0,0,150);"
        "padding: 2px 5px;"
        "border-radius: 3px;"
        );
    m_scrubTimeLabel->setVisible(false);

    // Ловим клики/отпускания мыши прямо на progressBar
    progressBar->installEventFilter(this);

    // Обновляем текст таймкода по сигналу прогресса
    connect(progressBar, &CircularProgressBar::progressChanged,
            this, &MainWindow::onScrubProgress);
}

// Деструктор
MainWindow::~MainWindow()
{
    delete ui; // Освобождение UI
}

// Переопределение showEvent: корректировка позиции кнопки аккаунта
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    accountButton->move(width() - accountButton->width() - 20, 10);

    if (glassCover) {
        glassCover->raise();
        glassCover->open(); // Открываем стеклянную крышку при показе окна
    }
}

// Переопределение closeEvent: анимация закрытия стеклянной крышки
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (glassCover && glassCover->isOpen()) {
        glassCover->close();
        // Ждем окончания анимации
        QEventLoop loop;
        connect(glassCover->animation(), &QPropertyAnimation::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }

    QMainWindow::closeEvent(event);
}

// Переопределение resizeEvent: адаптация компонентов при изменении размера
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (accountButton) {
        accountButton->move(width() - accountButton->width(), 0);
    }

    repositionSearchWidget();
    repositionVinyl();
    repositionTonearm();
}

// Вспомогательные методы для позиционирования
void MainWindow::repositionSearchWidget()
{
    if (!searchWidget) return;
    searchWidget->move(200, 10);
    searchWidget->raise();
}

void MainWindow::repositionVinyl()
{
    if (!vinyl) return;
    int x = (width() - vinyl->width()) / 2;
    int y = height() - vinyl->height() + 450;
    vinyl->move(x, y);
}

void MainWindow::repositionTonearm()
{
    if (!tonearm) return;
    tonearm->move(10, 450);
}

// Анимация опускания тонарма на пластинку
void MainWindow::startAnimation()
{
    if (!tonearm) return;

    QPropertyAnimation *anim = new QPropertyAnimation(tonearm, "angle");
    anim->setDuration(500);
    anim->setStartValue(0);
    anim->setEndValue(-30);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// Показ страницы коллекции
void MainWindow::showCollectionPage()
{
    if (m_collectionPage) {
        m_collectionPage->deleteLater();
    }

    // Удаляем старую страницу, если есть
    m_collectionPage = new CollectionPage(this);
    setCentralWidget(m_collectionPage);
    m_collectionPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Кнопка "Назад"
    QPushButton* backBtn = new QPushButton("← Назад", m_collectionPage);
    backBtn->setStyleSheet("QPushButton { font-size: 16px; padding: 5px 10px; }");
    backBtn->move(20, 20);
    connect(backBtn, &QPushButton::clicked, this, [this]() {
        m_collectionPage->deleteLater();
        m_collectionPage = nullptr;
    });
}

// Воспроизведение текущего трека (загрузка, установка обложки, запуск)
void MainWindow::playCurrentTrack()
{
    if (m_trackList.isEmpty()) return;

    const QString &trackPath = m_trackList[m_currentTrackIndex];
    QUrl currentUrl = QUrl::fromLocalFile(trackPath);

    if (isLoading) {
        qDebug() << "Already loading a track, skipping...";
        return;
    }

    // Если источник изменился — загружаем новый трек
    if (m_player->source().url() != currentUrl) {
        isLoading = true;
        isPlaying = false;

        m_player->stop();
        vinyl->stopSpinning();
        m_player->setSource(currentUrl);

        disconnect(m_player, &QMediaPlayer::mediaStatusChanged, nullptr, nullptr);

        connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::LoadedMedia) {
                isLoading = false;
                isPlaying = true;
                m_player->play();
                vinyl->startSpinning();
                searchWidget->setTrackName(trackPath); // Обновить название
            } else if (status == QMediaPlayer::InvalidMedia) {
                isLoading = false;
                isPlaying = false;
                qWarning() << "Failed to load track:" << trackPath;
            }
        });
    } else {
        // Если трек уже загружен — просто воспроизводим его
        if (m_player->playbackState() != QMediaPlayer::PlayingState) {
            isPlaying = true;
            m_player->play();
            vinyl->startSpinning();
        }
    }

    // Получение обложки из ID3 тега
    QImage cover;
    TagLib::MPEG::File mpegFile(trackPath.toStdString().c_str());
    TagLib::ID3v2::Tag *id3v2tag = mpegFile.ID3v2Tag();
    if (id3v2tag) {
        TagLib::ID3v2::FrameList frames = id3v2tag->frameListMap()["APIC"];
        if (!frames.isEmpty()) {
            auto *picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
            if (picFrame) {
                QByteArray imageData(picFrame->picture().data(), picFrame->picture().size());
                cover.loadFromData(imageData);
            }
        }
    }

    vinyl->setCoverImage(cover); // Установка изображения обложки
}

// Переход к следующему треку
void MainWindow::playNextTrack()
{
    qDebug() << "[DEBUG] playNextTrack(), current index BEFORE:" << m_currentTrackIndex;
    if (m_trackList.isEmpty()) return;

    m_currentTrackIndex = (m_currentTrackIndex + 1) % m_trackList.size();
    qDebug() << "[DEBUG] playNextTrack(), current index AFTER:" << m_currentTrackIndex;
    playCurrentTrack();
    vinyl->resetRotation(); // Сброс угла вращения винила

    qDebug() << "Sender:" << sender();
}

// Переход к предыдущему треку
void MainWindow::playPreviousTrack()
{
    if (m_trackList.isEmpty()) return;

    m_currentTrackIndex = (m_currentTrackIndex - 1 + m_trackList.size()) % m_trackList.size();
    playCurrentTrack();
    vinyl->resetRotation();
}

// Обновление прогресса воспроизведения
void MainWindow::updateTrackProgress()
{
    if (!m_player || !progressBar) return;

    qint64 pos = m_player->position(); // Текущее время в миллисекундах
    qint64 dur = m_player->duration(); // Общее время трека в миллисекундах
    if (dur > 0) {
        progressBar->setProgress(static_cast<qreal>(pos) / dur);
    }
}

// Фильтрация глобальных событий, управление фокусом и таймкодом
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 1) Глобальный клик — сбрасываем фокус у реального фокус-виджета внутри searchWidget
    if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent*>(event);
        QPoint gp = me->globalPosition().toPoint();
        QWidget *w  = QApplication::widgetAt(gp);
        QWidget *fw = QApplication::focusWidget();

        if (fw && (fw == searchWidget || searchWidget->isAncestorOf(fw))
            && (!w || !searchWidget->isAncestorOf(w))) {
            fw->clearFocus();
        }
    }

    // 2) Для прогресс-бара — показываем/прячем таймкод
    if (obj == progressBar) {
        if (event->type() == QEvent::MouseButtonPress)
            m_scrubTimeLabel->setVisible(true);
        else if (event->type() == QEvent::MouseButtonRelease)
            m_scrubTimeLabel->setVisible(false);
    }

    return QMainWindow::eventFilter(obj, event);
}

// Загрузка треков из SQLite БД с учётом тегов
void MainWindow::loadTracksFromDatabase(const QString &dbPath)
{
    QStringList allowedTags = loadAllowedTagsFromFile("/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/tags.txt");

    // фильтрация: любой или все теги
    // Установить true, чтобы все выбранные теги присутствовали у трека
    // Установить false, если достаточно хотя бы одного совпадения
    bool matchAllSelectedTags = true;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "tracks_connection");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qWarning() << "Не удалось открыть БД:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT id, path FROM tracks ORDER BY id")) {
        qWarning() << "Ошибка SELECT:" << query.lastError().text();
        db.close();
        return;
    }

    m_trackList.clear();

    while (query.next()) {
        int trackId = query.value(0).toInt();
        QString path = query.value(1).toString();
        if (!QFile::exists(path)) { // пропустить отсутствующие файлы
            qWarning() << "Файл не найден, пропускаем:" << path;
            continue;
        }

        // / Получаем теги из таблиц genres и track_genres
        QSqlQuery tagQuery(db);
        tagQuery.prepare(R"(
            SELECT g.name FROM genres g
            JOIN track_genres tg ON g.id = tg.genre_id
            WHERE tg.track_id = :trackId
        )");
        tagQuery.bindValue(":trackId", trackId);
        if (!tagQuery.exec()) {
            qWarning() << "Ошибка при получении тегов:" << tagQuery.lastError().text();
            continue;
        }

        QStringList trackTags;
        while (tagQuery.next()) {
            trackTags << tagQuery.value(0).toString();
        }

        // Проверка соответствия тегов
        bool shouldAdd = true;

        if (allowedTags.isEmpty()) {
            // Если файл тегов пустой — добавляем все треки
            shouldAdd = true;
        } else if (matchAllSelectedTags) {
            // Все теги из allowedTags должны быть у трека
            shouldAdd = std::all_of(allowedTags.begin(), allowedTags.end(), [&](const QString &tag) {
                return trackTags.contains(tag, Qt::CaseInsensitive);
            });
        } else {
            // Хотя бы один тег должен совпасть
            shouldAdd = std::any_of(trackTags.begin(), trackTags.end(), [&](const QString &tag) {
                return allowedTags.contains(tag, Qt::CaseInsensitive);
            });
        }

        if (shouldAdd) {
            m_trackList.append(path);
        } else {
            qDebug() << "Трек не прошёл фильтр по тегам, пропускаем:" << path;
        }
    }

    db.close();
    m_currentTrackIndex = 0; // Сброс индекса
}

// Чтение списка допустимых тегов из файла
QStringList MainWindow::loadAllowedTagsFromFile(const QString &filePath)
{
    QStringList tags;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            QString line = QString::fromUtf8(file.readLine()).trimmed();
            if (!line.isEmpty()) {
                tags << line;
            }
        }
        file.close();
    } else {
        qWarning() << "Не удалось открыть файл с тегами:" << filePath;
    }
    return tags;
}

// Обновление метки времени при скраббинге
void MainWindow::onScrubProgress(qreal progress)
{
    // Берём длительность из плеера
    qint64 dur = m_player->duration(); // в миллисекундах
    qint64 pos = static_cast<qint64>(progress * dur);

    int totalSeconds = pos / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    // Форматируем MM:SS
    QString txt = QString("%1:%2")
                      .arg(minutes, 2, 10, QChar('0'))
                      .arg(seconds, 2, 10, QChar('0')); // Формат MM:SS

    m_scrubTimeLabel->setText(txt);
    m_scrubTimeLabel->adjustSize();

    // Центрируем по горизонтали над прогресс-баром
    int px = progressBar->x() + progressBar->width()/2 - m_scrubTimeLabel->width()/2;
    int py = progressBar->y() - m_scrubTimeLabel->height() - 8; // 8px отступ сверху
    m_scrubTimeLabel->move(px, py);
}
