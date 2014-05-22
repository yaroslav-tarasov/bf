#-------------------------------------------------
#
# Консольная  управляющая программа для модуля ядра
#
#-------------------------------------------------

TEMPLATE = app
TARGET = bfctl

include(../common/base.pri)

QT       += core network
QT       -= gui

CONFIG   += console thread
CONFIG   -= app_bundle

HEADERS += \
    utils.h

SOURCES += main.cpp \
    utils.cpp


#############
# include paths

INCLUDEPATH += ../bf-ko ../bf-link


#############
# lib paths

LIBS += -L../../lib  -lbf-link

include(../common/libnl.pri)
