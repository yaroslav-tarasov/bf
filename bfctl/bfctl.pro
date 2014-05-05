#-------------------------------------------------
#
# Project created by QtCreator 2014-04-09T17:16:19
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = bfctl
CONFIG   += console thread
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp \
    utils.cpp

HEADERS += \
    utils.h \
    trx_data.h


INCLUDEPATH += ../bf-ko ../bf-link

DESTDIR = ../../bin
LIBS += -L../../lib -lbf-link -lnl
OBJECTS_DIR = ./tmp
MOC_DIR = ./moc
