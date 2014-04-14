#-------------------------------------------------
#
# Project created by QtCreator 2014-04-09T17:16:19
#
#-------------------------------------------------

QT       += core thread

QT       -= gui

TARGET = bfctl
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp \
    #netlinksocket.cpp \
    bfcontrol.cpp \
    utils.cpp

HEADERS += \
    #netlinksocket.h \
    bfcontrol.h \
    utils.h

INCLUDEPATH += ../bf-ko ../bf-link

DESTDIR = ../../bin
LIBS += -L../../lib -lbf-link -lnl

