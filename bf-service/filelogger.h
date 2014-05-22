#ifndef FILELOGGER_H
#define FILELOGGER_H

//#include <QObject>
#include "logger.h"


class FileLogger : public logging::Writer
{
public:
    explicit FileLogger();
    virtual void write_msg( logging::level l, const char* msg);
    virtual void write_msg( logging::level l, const QString &msg);

    static FileLogger& instance();          // singleton accessor
signals:
    
public slots:

private:
    Q_DISABLE_COPY(FileLogger)

};

namespace logging
{

inline void add_file_writer()
{
    Logger& __lg__ = Logger::instance();
    __lg__.add_writer(Logger::writer_t(new FileLogger()));
};


}
#endif // FILELOGGER_H
