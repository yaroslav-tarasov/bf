#include "qsyslog.h"
#include <QTime>
#include <QDebug>

#include <syslog.h>

QString QSyslog::appName = "BfService";

QSyslog::QSyslog(QObject *parent) :
    QObject(parent)
{
}

QSyslog::~QSyslog()
{
    ::closelog();
}

QSyslog& QSyslog::instance()
{
    static QSyslog singleton;
    static bool init = false;
    if( !init )
    {
        ::openlog(appName.toStdString().c_str(), LOG_PID , LOG_DAEMON);
        init = true;
    }
    return singleton;
}


void QSyslog::syslog(int level, QString message)
{
    QTime rightNow = QTime::currentTime();
    qDebug() << rightNow.toString() + " " + QString::number(level) + " " + message;
    ::syslog(level, (const char *)message.toLatin1());
}
