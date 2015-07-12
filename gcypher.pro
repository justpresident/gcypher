#-------------------------------------------------
#
# Project created by QtCreator 2015-06-09T16:23:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gcypher
TEMPLATE = app


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    rijndael.cpp

HEADERS  += rijndael.h \
        store.h \
        mainwindow.h \
    cypher.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    res.qrc
