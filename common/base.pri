
BAR_SRC_PATH=../../bar/CentOS/src

#############
# build paths
contains(TEMPLATE, app): DESTDIR = $$PWD/../../bin
contains(TEMPLATE, lib): DESTDIR = $$PWD/../../lib
OBJECTS_DIR = ./tmp
MOC_DIR = ./moc

###################
# compilation flags
QMAKE_CXXFLAGS += -std=c++0x

HEADERS +=  ../bf-ko/trx_data.h \
    ../common/barrier_integration.h \
    ../common/bf_defs.h

INCLUDEPATH+= ../exts
INCLUDEPATH+= $$PWD
