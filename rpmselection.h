#pragma once

#ifndef RPMSELECTION_H
#define RPMSELECTION_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSet>
#include <QFrame>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDir>

class RPMSelection : public QWidget
{
    Q_OBJECT

public:
    explicit RPMSelection(QWidget *parent = nullptr);
    ~RPMSelection();

    // геттер для MainWindow
    QStringList selectedTagsList() const {
        return QStringList(selectedTags.begin(), selectedTags.end());
    }



protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void showPopup();
    void updatePopupContent();
    void filterTags(const QString &text);
    void addTag(QListWidgetItem *item);
    QStringList loadTagsFromDatabase(const QString &dbPath);
    void saveSelectedTags();
    void loadSelectedTags();
    void clearAllTags();

    void saveSelectedTagsToFile(const QString& filePath);


    bool isLoadingTags = false;

    QPushButton *mainButton;
    QLineEdit *input;
    QListWidget *suggestionsList;
    QVBoxLayout *tagsLayout;
    QWidget *popupWidget;

    QStringList allTags;

    QFrame *clearLine;

    QString tagFilePath;

    QSet<QString> selectedTags;

signals:
    // void tagsChanged(const QStringList &selectedTags);


    void tagsChanged();
};

#endif // RPMSELECTION_H
