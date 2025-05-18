// collectionpage.h
#ifndef COLLECTIONPAGE_H
#define COLLECTIONPAGE_H

#include <QWidget>
#include <QScrollArea>
#include <QPropertyAnimation>

class VinylDisk : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)
public:
    explicit VinylDisk(const QString& title, QWidget* parent = nullptr);

    int yOffset() const;
    void setYOffset(int offset);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString m_title;
    int m_yOffset;
    QPropertyAnimation* m_animation;
};

class CollectionPage : public QScrollArea
{
    Q_OBJECT
public:
    explicit CollectionPage(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QWidget* m_container;
    QList<VinylDisk*> m_disks;

    void setupDisks();
    void updateLayout();
};

#endif // COLLECTIONPAGE_H
