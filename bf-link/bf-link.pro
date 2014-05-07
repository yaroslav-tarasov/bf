#-------------------------------------------------
#
# Project created by QtCreator 2014-04-09T14:48:54
#
#-------------------------------------------------

QT       += network xml

QT       -= gui

TARGET = bf-link
TEMPLATE = lib

CONFIG   += thread

QMAKE_CXXFLAGS += -std=c++0x

DEFINES += BFLINK_LIBRARY

SOURCES += \ #bflink.cpp \
           bfcontrol.cpp \
           netlinksocket.cpp \
           bflocalcontrol.cpp \
           bfremotecontrol.cpp

HEADERS += \ #bflink.h\
        bfcontrol.h \
        bf-link_global.h\
        netlinksocket.h \
        bflocalcontrol.h \
        bfremotecontrol.h \
        trx_data.h


INCLUDEPATH += ../bf-ko

DESTDIR = ../../lib
OBJECTS_DIR = ./tmp
MOC_DIR = ./moc

include(../common/libnl.pri)

