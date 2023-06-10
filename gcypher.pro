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
    cypher.cpp \
        main.cpp \
        mainwindow.cpp \
    rijndael.cpp

QMAKE_CXXFLAGS += -std=c++20 -Wcast-align -Wunused -Wshadow -Wpointer-arith -Wcast-qual -Wno-missing-braces -Wsuggest-override -Werror=suggest-override



HEADERS  += rijndael.h \
        store.h \
        mainwindow.h \
    cypher.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    res.qrc
