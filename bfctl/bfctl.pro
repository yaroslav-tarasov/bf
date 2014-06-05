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
    utils.h \
    cmds.h

SOURCES += main.cpp \
    utils.cpp \
    cmds.cpp


#############
# include paths

INCLUDEPATH += ../bf-ko ../bf-link


#############
# lib paths

LIBS += -L../../lib  -lbf-link

include(../common/libnl.pri)
