#-------------------------------------------------
#
#  Библиотека связи с модулем ядра
#
#-------------------------------------------------

TEMPLATE = lib
TARGET = bf-link

include(../common/base.pri)

QT       += network xml
QT       -= gui

CONFIG   += thread

DEFINES += BFLINK_LIBRARY

HEADERS += \
        bfcontrol.h \
        bf-link_global.h\
        netlinksocket.h \
        bflocalcontrol.h \
        bfremotecontrol.h \
        errorreceiver.h \
        timerproxy.h \
        qwaitfordone.h \
        bflocalserver.h \
    bflocalcontrolprivate.h \
    bfremotecontrolprivate.h \
    bfcontrolbase.h

SOURCES += \
        bfcontrol.cpp \
        netlinksocket.cpp \
        bflocalcontrol.cpp \
        bfremotecontrol.cpp \
        qwaitfordone.cpp \
        bflocalserver.cpp \
    bflocalcontrolprivate.cpp \
    bfremotecontrolprivate.cpp \
    bfcontrolbase.cpp

#############
# include paths

INCLUDEPATH += ../bf-ko

include(../common/libnl.pri)

