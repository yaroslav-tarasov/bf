#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QSharedPointer>
#include <QMap>
#include <QThread>

namespace logging
{

enum level
{
    L_FATAL,
    L_ERROR,
    L_WARN,
    L_INFO,
    L_DEBUG,
    L_TRACE
};

inline QString level_name(level l)
{
    static QString names [] =
    {
        "FATAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "TRACE"
    };

    return names[l];
}

struct Writer
{
    virtual void write_msg( level l, const char* msg)=0;
    virtual void write_msg( level l, const QString &msg)=0;

};

inline  level translateLevel(QtMsgType type) {
    switch (type) {
        case QtDebugMsg:
            return L_INFO;
            break;
        case QtWarningMsg:
            return L_WARN;
            break;
        case QtCriticalMsg:
            return L_ERROR;
            break;
        case QtFatalMsg:
            return L_FATAL;
    }
    return L_TRACE; // И никогда сюда не попадем
}

class Logger : public QObject
{
    Q_OBJECT

public:
    typedef QSharedPointer<Writer> writer_t;

    inline static QString msg_string( level l, const char* msg)
    {
        return  QString("%1 %2: %3").arg(QDateTime::currentDateTime().toString("[yyyy.MM.dd hh:mm:ss.zzz]")).arg(level_name(l)).arg(msg);
    }
    inline static QString msg_string( level l, const QString& msg)
    {
        return  QString("%1 %2: %3").arg(QDateTime::currentDateTime().toString("[yyyy.MM.dd hh:mm:ss.zzz]")).arg(level_name(l)).arg(msg);
    }

    static Logger& instance  ();          // singleton accessor
    void           write_msg ( level l, const char* msg);
    void           add_writer( writer_t w );


    void           log( level l, const char* msg);
    void           log( level l, const QString& msg);

signals:
    
public slots:

private:
    explicit Logger(QObject *parent = 0);
    Q_DISABLE_COPY(Logger)
    static void MessageHandler(QtMsgType type, const char *msg);
    QVector<writer_t > writers;
    static QMap<Qt::HANDLE,int>  threads_;
};


}

#define LOG_MSG(l, msg)                                                \
{                                                                      \
    logging::Logger::instance().log((l), msg);                         \
}

#define T_DEBUG(message)                      LOG_MSG(logging::L_DEBUG, message)
#define T_TRACE(message)                      LOG_MSG(logging::L_TRACE, message)
#define T_INFO(message)                       LOG_MSG(logging::L_INFO , message)
#define T_WARN(message)                       LOG_MSG(logging::L_WARN, message)
#define T_ERROR(message)                      LOG_MSG(logging::L_ERROR, message)



#endif // LOGGER_H
