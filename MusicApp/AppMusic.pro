QT       += core gui sql widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    accountbutton.cpp \
    circularprogressbar.cpp \
    collectionbutton.cpp \
    collectionpage.cpp \
    glasscover.cpp \
    homebutton.cpp \
    likebutton.cpp \
    main.cpp \
    mainwindow.cpp \
    nextbutton.cpp \
    playpausebutton.cpp \
    previousbutton.cpp \
    rpmselection.cpp \
    searchwidget.cpp \
    tonearm.cpp \
    vinylwidget.cpp \
    volumeslider.cpp \
    woodbackgroundwidget.cpp

HEADERS += \
    accountbutton.h \
    circularprogressbar.h \
    collectionbutton.h \
    collectionpage.h \
    glasscover.h \
    homebutton.h \
    likebutton.h \
    mainwindow.h \
    nextbutton.h \
    playpausebutton.h \
    previousbutton.h \
    rpmselection.h \
    searchwidget.h \
    tonearm.h \
    vinylwidget.h \
    volumeslider.h \
    woodbackgroundwidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Библиотека libsndfile
INCLUDEPATH += /opt/homebrew/include
LIBS += -L/opt/homebrew/lib -lsndfile

LIBS += -lsqlite3

# Библиотека TagLib
INCLUDEPATH += /opt/homebrew/opt/taglib/include
LIBS += -L/opt/homebrew/opt/taglib/lib -ltag

