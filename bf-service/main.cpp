#include <qtservice.h>
#include <QtCore/QObject>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/msg.h>
#include <linux/rtnetlink.h>
#include "bfcontrol.h"

class MyService : public QtService<QCoreApplication>
 {
 public:
     MyService(int argc, char **argv,const QString &name):QtService<QCoreApplication>(argc, argv, name){};
     ~MyService(){};

 protected:
     void start(){};
     void stop(){exit(0);};
     void pause(){};
     void resume(){};
     void processCommand(int code){};
 };

int main(int argc, char **argv)
{
	QCoreApplication::setApplicationName("myService");
	QCoreApplication::setOrganizationName("VNIIRA");
	// QCoreApplication::setApplicationVersion(QString("%1").arg(VERSION_FULL));

	 MyService service(argc, argv,"myService");
	 return service.exec();
}
