#ifndef BFLINK_GLOBAL_H
#define BFLINK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BFLINK_LIBRARY)
#  define BFLINKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define BFLINKSHARED_EXPORT Q_DECL_IMPORT
#endif


const int commandWaitTime = 1000; // в миллисекундах

#endif // BFLINK_GLOBAL_H
