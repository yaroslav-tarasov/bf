#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include <iostream>

#include "netlinksocket.h"
#include "bfcontrol.h"
//#include "netlink_socket.h"

#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/if_ether.h>
#include "trx_data.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    BFControl *bfc = new BFControl(&a);
    NetlinkSocket *pNS = new NetlinkSocket(bfc) ;

    pNS->create(NETLINK_USERSOCK,sizeof(filter_rule_t));
    filter_rule_t fr(17,18000,2000);
    pNS->sendMsg(MSG_GET_RULES,&fr,sizeof(filter_rule_t));
    //QObject::connect( pNS,SIGNAL(finished()),&a,SLOT(quit()));
    QObject::connect( pNS,SIGNAL(data(QByteArray)),bfc,SLOT(onDataArrival(QByteArray)),Qt::DirectConnection);
    //QObject::connect( pNS,SIGNAL(readyRead()),bfc,SLOT(onData())/*,Qt::DirectConnection*/);

    // pNS->startListener(2,33); // PROTO, buf size


    std::cout << "Press  any key" << std::endl;
    std::cin.get();
    pNS->close();
    QTimer::singleShot(10, &a, SLOT(quit()));
    return a.exec();
}
