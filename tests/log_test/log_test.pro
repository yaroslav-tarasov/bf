#-------------------------------------------------
#
# Project created by QtCreator 2014-05-21T07:41:05
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = log_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
        ../../bf-service/filelogger.cpp \
        ../../bf-service/logger.cpp \
        ../../bf-service/qsyslog.cpp \
        ../../bf-service/bfconfig.cpp

HEADERS += ../../bf-service/filelogger.h \
        ../../bf-service/logger.h \
        ../../bf-service/qsyslog.h \
        ../../bf-service/bfconfig.h

INCLUDEPATH += ../../bf-service

OBJECTS_DIR = ./tmp
MOC_DIR = ./moc
