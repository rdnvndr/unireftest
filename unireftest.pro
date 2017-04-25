#-------------------------------------------------
#
# Project created by QtCreator 2017-02-02T08:40:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_uts
TEMPLATE = app
QT += sql

SOURCES += main.cpp\
        mainwindow.cpp \
    dialogconnect.cpp \
    querythread.cpp

HEADERS  += mainwindow.h \
    dialogconnect.h \
    querythread.h

FORMS    += mainwindow.ui \
    dialogconnect.ui

RESOURCES += \
    qrc.qrc
