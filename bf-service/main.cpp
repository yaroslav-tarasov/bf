#include <QtCore/QObject>
#include "bfservice.h"


int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName("bf-service");
	QCoreApplication::setOrganizationName("VNIIRA");
	// QCoreApplication::setApplicationVersion(QString("%1").arg(VERSION_FULL));


    BfService service(argc, argv, "bf-service");

    return service.exec();
}
