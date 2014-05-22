#ifndef QSYSLOG_H
#define QSYSLOG_H

#include <logger.h>


class QSyslog : public logging::Writer
{

public:
    explicit QSyslog();
    ~QSyslog();
    virtual void write_msg(logging::level l, const QString &msg);
    virtual void write_msg(logging::level l, const char* msg);

    static void setAppName(const std::string& appName) { QSyslog::appName = appName; }

    static void syslog(int level, QString message);

private:

    Q_DISABLE_COPY(QSyslog)

    static std::string appName;
};


namespace logging
{

inline void add_syslog_writer()
{
    Logger& __lg__ = Logger::instance();
    __lg__.add_writer(Logger::writer_t(new QSyslog));
};

}


#endif // QSYSLOG_H
