#include "qsyslog.h"
#include <QTime>
#include <QDebug>
#include <QFile>
#include <syslog.h>

using namespace logging;


std::string QSyslog::appName("BfService");

QSyslog::QSyslog()
{
    ::openlog(appName.c_str(), LOG_PID , LOG_DAEMON); // Не забываем также про LOG_PERROR, думаем
}

QSyslog::~QSyslog()
{
    ::closelog();
}

//////////////////////
//   Соответствие между логгером и syslog
///////////////////////////////////////////
//
// enum level
// {
//     L_FATAL,
//     L_ERROR,
//     L_WARN,
//     L_INFO,
//     L_DEBUG,
//     L_TRACE
// };

//#define	LOG_EMERG	0	/* system is unusable */
//#define	LOG_ALERT	1	/* action must be taken immediately */
//#define	LOG_CRIT	2	/* critical conditions */
//#define	LOG_ERR		3	/* error conditions */
//#define	LOG_WARNING	4	/* warning conditions */
//#define	LOG_NOTICE	5	/* normal but significant condition */
//#define	LOG_INFO	6	/* informational */
//#define	LOG_DEBUG	7	/* debug-level messages */

void QSyslog::syslog(int level, QString message)
{
    // QTime rightNow = QTime::currentTime();
    // qDebug() << rightNow.toString() + " " + QString::number(level) + " " + message;
    static const int syslog_levels [] = {LOG_EMERG,LOG_ERR,LOG_WARNING,LOG_INFO, LOG_DEBUG, LOG_DEBUG};
    // Для syslog без уровня TRACE, ибо нечего захламлять системный лог
    if(level < L_TRACE && level >= L_FATAL)
        ::syslog(syslog_levels[level], (const char *)message.toLocal8Bit().constData());
}

void QSyslog::write_msg( level l, const char* msg)
{
    ::syslog(l, msg);
}

void QSyslog::write_msg( level l, const QString& msg)
{
    syslog(l, QString (msg));
}



