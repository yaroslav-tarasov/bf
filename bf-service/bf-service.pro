######################################################################
# Automatically generated by qmake (2.01a) ?? ??? 27 18:51:52 2014
######################################################################

TEMPLATE = app
TARGET =

include(../common/base.pri)

DEPENDPATH += .

QT -=  gui
QT += core network xml

HEADERS += \
    bfservice.h \
    bfserviceprivate.h \
    qsyslog.h \
    bfrules.h \
    bfconfig.h \
    filelogger.h \
    logger.h \
    signalcatcher.h \
    utils.h

# Input
SOURCES += main.cpp \
    bfservice.cpp \
    bfserviceprivate.cpp \
    qsyslog.cpp \
    bfrules.cpp \
    bfconfig.cpp \
    filelogger.cpp \
    logger.cpp \
    signalcatcher.cpp \
    utils.cpp

#############
# include  paths
INCLUDEPATH += ../bf-ko ../bf-ko/base ../bf-link
INCLUDEPATH += .
INCLUDEPATH += $$BAR_SRC_PATH/BAudit/BAu \
               $$BAR_SRC_PATH/BAudit/common \
               $$BAR_SRC_PATH/defs

#############
# lib paths
LIBS += -L../../lib -lbf-link
LIBS += -L$$BAR_SRC_PATH/lib -lBAu

include(../exts/qtservice/src/qtservice.pri)
include(../common/libnl.pri)

