#include "collectionpage.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QLinearGradient>

VinylDisk::VinylDisk(const QString& title, QWidget* parent)
    : QWidget(parent), m_title(title), m_yOffset(0)
{
    setFixedSize(300, 300);
    m_animation = new QPropertyAnimation(this, "yOffset");
    m_animation->setDuration(200);
}

void VinylDisk::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Центр диска
    QPointF center(width()/2, height()/2);
    int radius = qMin(width(), height())/2 - 10;

    // Фон (чёрный винил)
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(center, radius, radius);

    // Бороздки (концентрические круги)
    painter.setPen(QPen(QColor(60, 60, 60), 1));
    for(int r = radius-10; r > 20; r -= 4) {
        painter.drawEllipse(center, r, r);
    }

    // Центральная этикетка
    painter.setBrush(QColor(200, 200, 200));
    painter.drawEllipse(center, 20, 20);

    // Название диска
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    painter.drawText(rect().adjusted(0, 0, 0, -20), Qt::AlignCenter, m_title);
}

void VinylDisk::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);
    m_animation->stop();
    m_animation->setStartValue(m_yOffset);
    m_animation->setEndValue(-10);
    m_animation->start();
}

void VinylDisk::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    m_animation->stop();
    m_animation->setStartValue(m_yOffset);
    m_animation->setEndValue(0);
    m_animation->start();
}

int VinylDisk::yOffset() const { return m_yOffset; }

void VinylDisk::setYOffset(int offset)
{
    m_yOffset = offset;
    update();
}

CollectionPage::CollectionPage(QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_container = new QWidget(this);
    setWidget(m_container);

    setupDisks();
}

void CollectionPage::setupDisks()
{
    // Очищаем старые диски
    qDeleteAll(m_disks);
    m_disks.clear();

    // Создаем 10 виниловых дисков
    for(int i = 0; i < 10; ++i) {
        VinylDisk* disk = new VinylDisk(QString("Диск %1").arg(i+1), m_container);
        m_disks.append(disk);
    }

    updateLayout();
}

void CollectionPage::updateLayout()
{
    if(m_disks.isEmpty()) return;

    const int diskHeight = m_disks.first()->height();
    const int spacing = 20; // Расстояние между дисками
    const int margin = 50;  // Отступ от краёв

    int y = margin;
    for(VinylDisk* disk : m_disks) {
        disk->move((width() - disk->width())/2, y);
        y += diskHeight + spacing;
    }

    m_container->setFixedSize(width(), y + margin);
}

void CollectionPage::resizeEvent(QResizeEvent* event)
{
    QScrollArea::resizeEvent(event);
    updateLayout();
}
