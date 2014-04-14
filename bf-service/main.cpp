#include <qtservice.h>
#include <QtCore/QObject>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/msg.h>
#include <linux/rtnetlink.h>

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

class netlink_wrap
{
	void create_sock()
	{
        static long sequence_number = 0;

        // Must be in libnl? but we do not have it in CentOS
        // struct nl_sock *sock;
        // sock = nl_socket_alloc();

	struct nl_object *obj;
        struct nl_addr *naddr = rtnl_addr_get_local((struct rtnl_addr *) obj);


	
	struct sockaddr_nl sa;

        memset (&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;
        sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

        int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        bind(fd, (struct sockaddr*)&sa, sizeof(sa));


        struct nlmsghdr *nh;    /* The nlmsghdr with payload to send. */
        //struct sockaddr_nl sa;
        struct iovec iov = { (void *) nh, nh->nlmsg_len };
        struct msghdr msg;

        msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
        memset (&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;
        nh->nlmsg_pid = 0;
        nh->nlmsg_seq = ++sequence_number;
        /* Request an ack from kernel by setting NLM_F_ACK. */
        nh->nlmsg_flags |= NLM_F_ACK;

        sendmsg (fd, &msg, 0);
    }

};

int main(int argc, char **argv)
{
	QCoreApplication::setApplicationName("myService");
	QCoreApplication::setOrganizationName("VNIIRA");
	// QCoreApplication::setApplicationVersion(QString("%1").arg(VERSION_FULL));

	 MyService service(argc, argv,"myService");
	 return service.exec();
}
