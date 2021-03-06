#-------------------------------------------------
#
#  Фиктивный проект для сборки модуля ядра
#
#-------------------------------------------------

mytarget.commands = make -C ../bf-ko
QMAKE_EXTRA_TARGETS += mytarget
PRE_TARGETDEPS += mytarget

TEMPLATE = lib
TARGET   =
CONFIG  += no_link target_predeps staticlib
CONFIG -= qt

QMAKE_POST_LINK = @rm *.a

OTHER_FILES += ../bf-ko/bf_cmd.c \
               ../bf-ko/bf_module.c \
               ../bf-ko/nl_int.c \
               ../bf-ko/bf_config.h \
               ../bf-ko/nl_int.h


