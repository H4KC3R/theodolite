#-------------------------------------------------
#
# Project created by QtCreator 2016-10-18T09:13:59
#
#-------------------------------------------------

QT  += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theodolite
TEMPLATE = app
CONFIG += c++14

SOURCES += main.cpp\
        mainwindow.cpp \
    theodolite.cpp \
    dialog.cpp \
    gms.cpp \
    theodolitemeasure.cpp \
    led.cpp

HEADERS  += mainwindow.h \
    theodolite.h \
    com_pub.hpp \
    dialog.h \
    gms.h \
    meanskofcn.h \
    theodolitemeasure.h \
    led.h

FORMS    += mainwindow.ui \
    dialog.ui

win32:RC_FILE = theodolite.rc

RESOURCES += \
    sources.qrc
