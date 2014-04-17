#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QFile>

#include <iostream>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/netlink.h>

#include "trx_data.h"
//#include "utils.h"

inline int intv_rnd(int a, int b)
{
    qsrand(qrand());
    return a + qrand() * (b-a);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList cmdline_args = QCoreApplication::arguments();

    if(cmdline_args.size()>1)
    {
    const QString thename = cmdline_args[1];
    int n = 160;
    QFile  file(thename) ;
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))  return -1;
    QDataStream out(&file);

    for(int i=0;i<n;i++){
        filter_rule_t fr(intv_rnd(1,100)>50?IPPROTO_UDP:IPPROTO_TCP,intv_rnd(100,36000),intv_rnd(100,36000));
        out <<  fr;
    }
    file.close();

    return 0; // a.exec();
    }
}
