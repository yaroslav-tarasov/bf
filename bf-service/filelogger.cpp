#include "filelogger.h"
#include "bfconfig.h"
#include <QFile>
#include <QTextStream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace logging;

namespace
{


inline void checkAndRotate(QFile& f) {
    BFConfig& cfg = BFConfig::instance();
    if(f.size() > cfg.logMaxFileSize()) {
        if(cfg.logCount() == 0) {
            f.remove();
        } else {
            for(quint8 i = cfg.logCount(); i > 1; i--) {
                QFile::remove(cfg.logFile() + "." + QString::number(i));
                QFile::rename(cfg.logFile() + "." + QString::number(i - 1)
                            , cfg.logFile() + "." + QString::number(i));
            }
            QFile::remove(cfg.logFile() + ".1");
            QFile::rename(cfg.logFile(), cfg.logFile() + ".1");
        }
    }
}



}

FileLogger::FileLogger()
{
}


void FileLogger::write_msg(logging::level l, const char* msg)
{
    BFConfig& cfg = BFConfig::instance();
    if(cfg.logFile().isEmpty()) {
        std::cerr << Logger::msg_string(l,msg).toStdString();
        return;
    }
    QFile log(cfg.logFile());
    checkAndRotate(log);
    if(!log.open(QIODevice::Append)) {
        std::cerr << Logger::msg_string(l,msg).toStdString();
    } else {
        QTextStream out(&log);
        out << Logger::msg_string(l,msg) << "\n";
        log.close();
    }
}

void  FileLogger::write_msg(logging::level l, const QString &msg)
{
    BFConfig& cfg = BFConfig::instance();
    if(cfg.logFile().isEmpty()) {
        std::cerr << Logger::msg_string(l,msg).toStdString();
        return;
    }
    QFile log(cfg.logFile());
    checkAndRotate(log);
    if(!log.open(QIODevice::Append)) {
        std::cerr << Logger::msg_string(l,msg).toStdString();
    } else {
        QTextStream out(&log);
        out << Logger::msg_string(l,msg) << "\n";
        log.close();
    }
}

