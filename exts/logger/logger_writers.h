#ifndef LOGGER_WRITERS_H
#define LOGGER_WRITERS_H
#include "logger.h"

namespace tlog
{

inline QString msg_string( level l, const char* msg)
{
    QString msg_str;
    msg_str = QString("%1 %2: %3").arg(QDateTime::currentDateTime().toString("[yyyy.MM.dd hh:mm:ss.zzz]")).arg(level_name(l)).arg(msg);

    return msg_str;
}

struct console_writer
    : public writer
{

    void write_msg( level l, const char* msg)
    {
    #if defined _WIN32 && !defined NDEBUG
       OutputDebugStringA((msg_string(l, msg) + "\n").c_str());
    #endif
       if (l = ERROR)
           std::cerr << msg_string( l, msg) << std::endl;
       else
           std::cout << msg_string( l, msg) << std::endl;
    }
};

inline void add_console_writer()
{
    tlog::logger& __lg__ = tlog::logger::get_logger();
    __lg__.add_writer(writer_ptr(new console_writer()));
};


class syslog_writer
    : public writer
{
public:
    syslog_writer()
    {
        remap.insert(make_pair(L_TRACE, LOG_DEBUG ));
        remap.insert(make_pair(L_DEBUG, LOG_DEBUG ));
        remap.insert(make_pair(L_INFO,  LOG_INFO  ) );
        remap.insert(make_pair(L_WARN , LOG_WARNING ) );
        remap.insert(make_pair(L_ERROR, LOG_ERR));
    }


    void write_msg( level l, const char* msg)
    {
        QString msg_str(msg_string( l, msg));
        syslog(remap[l],msg_str.toStdString().c_str());
    }
private:
    map<level, int> remap;
};

inline void add_syslog_writer()
{
    tlog::logger& __lg__ = tlog::logger::get_logger();
    __lg__.add_writer(writer_ptr(new syslog_writer()));
};


}
#endif // LOGGER_WRITERS_H
