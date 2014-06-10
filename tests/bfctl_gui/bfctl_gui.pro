#-------------------------------------------------
#
# Project created by QtCreator 2014-06-03T19:15:06
#
#-------------------------------------------------

QT       += core gui
QT       += network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bfctl_gui
TEMPLATE = app

include(../../common/base.pri)

SOURCES += main.cpp\
    mainwindow.cpp \
    rulestablemodel.cpp \
    ruledelegate.cpp

HEADERS  += mainwindow.h \
    rulestablemodel.h \
    ruledelegate.h

FORMS    += mainwindow.ui


INCLUDEPATH += ../../bf-ko ../../bf-link ../../bfctl

DESTDIR = ../../../bin
LIBS += -L../../../lib -lbf-link -lnl
OBJECTS_DIR = ./tmp
MOC_DIR = ./moc


TRANSLATIONS = translations/bfctl_gui_ru.ts

RESOURCES += \
    bfctl_gui.qrc

