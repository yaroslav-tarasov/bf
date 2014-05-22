#include "logger.h"

namespace logging
{

QMap<Qt::HANDLE,int>  Logger::threads_;

Logger::Logger(QObject *parent) :
    QObject(parent)
{
    qInstallMsgHandler(MessageHandler);
}

void Logger::MessageHandler(QtMsgType type, const char *msg)
{
    Qt::HANDLE h = QThread::currentThreadId();
    if(++threads_[h]>1)
    {
        threads_[h]=1;
        return;
    };

    level  l = translateLevel(type);
    Logger::instance().log(l,msg);
    threads_[h]=0;
}


void Logger::add_writer(writer_t w)
{
    writers.append(w);
}

Logger& Logger::instance()
{
    static Logger singleton;
    static bool init = false;
    if( !init )
    {

        init = true;
    }
    return singleton;
}

void Logger::log( level l, const char* msg)
{
    foreach(writer_t w, writers)
    {
        w->write_msg(l,msg);
    }

}

void Logger::log( level l, const QString& msg)
{
    foreach(writer_t w, writers)
    {
        w->write_msg(l,msg);
    }

}


}
