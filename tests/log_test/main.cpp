#include <QCoreApplication>
#include "logger.h"
#include "filelogger.h"
#include "qsyslog.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    logging::add_file_writer();
    logging::add_syslog_writer();

    T_INFO(QString("Info"));

    return 0;// a.exec();
}
