TEMPLATE = subdirs
CONFIG  += ordered

#CONFIG += target_predeps
mytarget.commands = make -C bf-ko
QMAKE_EXTRA_TARGETS += mytarget
PRE_TARGETDEPS += mytarget
#mytarget.depends = FORCE

svnbuildhook.depends = mytarget
CONFIG(debug,debug|release):svnbuildhook.target = Makefile.Debug
CONFIG(release,debug|release):svnbuildhook.target = Makefile.Release
QMAKEEXTRATARGETS += svnbuildhook

# Directories
SUBDIRS += bf-link bfctl bf-service bf-ko-build


