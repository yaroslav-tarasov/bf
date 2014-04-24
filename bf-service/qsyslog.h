#ifndef QSYSLOG_H
#define QSYSLOG_H

#include <QObject>

class QSyslog : public QObject
{
    Q_OBJECT

public:
    ~QSyslog();

    static void setAppName(QString appName) { QSyslog::appName = appName; }
    static QSyslog& instance();          // singleton accessor

    static void syslog(int level, QString message);
private:
    QSyslog(QObject *parent = 0);

    static QString appName;
    
};

#endif // QSYSLOG_H
