#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileSystemWatcher>

#include "circularprogressbar.h"
#include "collectionpage.h"
#include "previousbutton.h"
#include "vinylwidget.h"
#include "tonearm.h"
#include "searchwidget.h"
#include "accountbutton.h"
#include "glasscover.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void startAnimation();
    void showCollectionPage();
    void playCurrentTrack();
    void playPreviousTrack();
    void playNextTrack();
    // void setupPlaybackQueueFromFileGenres();
    void onScrubProgress(qreal progress);

private:
    Ui::MainWindow *ui;
    VinylWidget *vinyl;
    Tonearm *tonearm;
    SearchWidget *searchWidget;
    AccountButton *accountButton;
    GlassCover *glassCover = nullptr;

    CollectionPage* m_collectionPage = nullptr;

    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QStringList m_trackList;
    int m_currentTrackIndex = 0;

    void loadTracksFromFolder(const QString &folderPath);
    void loadTracksFromDatabase(const QString &dbPath);


    void repositionSearchWidget();
    void repositionVinyl();
    void repositionTonearm();

    PreviousButton *prevBtn = nullptr;
    void updateTrackProgress();
    CircularProgressBar *progressBar = nullptr;
    QLabel *m_scrubTimeLabel;
    QStringList loadSelectedGenresFromFile(const QString &filePath);

    bool isLoading = false;
    bool isPlaying = false;

    bool m_handlingEndOfMedia = false;

    bool m_skipNextEndEvent = false;

    bool m_ignorePlaybackEnded = false;

    QFileSystemWatcher *tagFileWatcher;
    QString tagsFilePath = "/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/tags.txt";

    QStringList loadAllowedTagsFromFile(const QString &filePath);

    bool m_matchAllTags = false; // По умолчанию: совпадает хотя бы один тег
};

#endif // MAINWINDOW_H
