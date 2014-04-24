#-------------------------------------------------
#
# Project created by QtCreator 2014-04-17T17:28:01
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = tests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp

INCLUDEPATH += ../bf-ko ../bf-link

DESTDIR = ../../bin
LIBS += -L../../lib -lbf-link -lnl
OBJECTS_DIR = ./tmp
MOC_DIR = ./moc
