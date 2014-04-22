mytarget.commands = make -C ../bf-ko
QMAKE_EXTRA_TARGETS += mytarget
PRE_TARGETDEPS += mytarget

TEMPLATE = lib
TARGET   =
CONFIG  += no_link target_predeps staticlib

QMAKE_POST_LINK = @rm *.a
