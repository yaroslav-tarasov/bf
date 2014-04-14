#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QStringList>

#include <iostream>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/netlink.h>


#include "bfcontrol.h"
#include "trx_data.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList cmdline_args = QCoreApplication::arguments();

    BFControl *bfc = new BFControl(&a);


    if(bfc->create(/*NETLINK_USERSOCK,sizeof(filter_rule_t)*/)==0)
    {

    //filter_rule_t fr(IPPROTO_UDP,18000,2000);
    filter_rule_t fr;
    int action = cmd_utils::parse_cmd_args(argc, argv,&fr);

    if (action == CMD_NEW_RULE) {
        qDebug() << "CMD_NEW_RULE\n";
        printf("Src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", fr.base_rule.s_addr.addr, fr.base_rule.d_addr.addr, fr.base_rule.proto, fr.base_rule.src_port, fr.base_rule.dst_port);

    } else if (action == CMD_PRINT_RULES) {
        qDebug() <<"CMD_PRINT_RULES\n";

        QEventLoop wait_for_done;
        QObject::connect(bfc, SIGNAL(done()), &wait_for_done, SLOT(quit()));

        int ret = bfc->sendMsg(MSG_GET_RULES,&fr,sizeof(filter_rule_t));

        if (ret < 0) {
            return EXIT_FAILURE;
        }

        wait_for_done.exec();



    } else if (action == CMD_DEL_RULE) {
        qDebug() << "CMD_DEL_RULE\n";


    } else if (action == CMD_PRINT_HELP) {
        qDebug() << "CMD_DEL_RULE\n";
        // exit_printhelp();
    }

    }
    else
    {
        qCritical() << "Can't connect to bf module \n";
    }
    std::cout << "Press  any key" << std::endl;
    std::cin.get();
    bfc->close();
    QTimer::singleShot(10, &a, SLOT(quit()));
    return a.exec();
}
