#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
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

    filter_rule_t fr;
    std::string thename;
    int action = cmd_utils::parse_cmd_args(argc, argv,&fr,thename);

    if (action == CMD_NEW_RULE) {
        qDebug() << "CMD_NEW_RULE\n";
        bfc->addRule(fr);
        printf("Add new rule src_addr: %X; dst_addr: %X; proto: %d; src_port: %d dst_port: %d\n", fr.base_rule.s_addr.addr, fr.base_rule.d_addr.addr, fr.base_rule.proto, fr.base_rule.src_port, fr.base_rule.dst_port);

    } else if (action == CMD_PRINT_RULES) {
        qDebug() <<"CMD_PRINT_RULES\n";

#ifdef TEST_ASYNC_GET_RULES
        bfc->getRulesAsync(fr);
#else
        QList<BFControl::filter_rule_ptr > ruleslst;
        bfc->getRulesSync(fr,  ruleslst);

        int i=0;

        foreach (BFControl::filter_rule_ptr rule,ruleslst){
            qDebug() << "rule #" << i++ << "  " << rule->base_rule.src_port << "  " << rule->base_rule.dst_port << "  " << cmd_utils::get_proto_name(rule->base_rule.proto);
            filter_rule_t fr = *static_cast<filter_rule_t*>(rule.data());
            //qDebug() << fr;
        }
#endif


    } else if (action == CMD_DEL_RULE) {
        qDebug() << "CMD_DEL_RULE\n";
        bfc->deleteRule(fr);

    } else if (action == CMD_GET_FROM_FILE) {
        QList<BFControl::filter_rule_ptr > ruleslst;
        qDebug() << "CMD_GET_FROM_FILE\n";
        QFile  file(QString::fromStdString(thename)) ;
        if (!file.open(QIODevice::ReadOnly))  return -1;
        QDataStream in(&file);

        while(!in.atEnd()){
            filter_rule_t fr;
            in >>  fr;
            ruleslst.append(BFControl::filter_rule_ptr(new filter_rule_t(fr)));;
        }
        file.close();

        if(ruleslst.size()>0)
            bfc->sendRulesSync(ruleslst);

    } else if (action == CMD_PRINT_HELP) {
        qDebug() << "CMD_PRINT_HELP";
        // exit_printhelp();
         qDebug() << "Something wrong in command string";
    }

    }
    else
    {
        qCritical() << "Can't connect to bf module \n";
    }
 //#ifdef TEST_ASYNC_GET_RULES
    std::cout << "Press  any key" << std::endl;
    std::cin.get();
 //#endif
    bfc->close();
    QTimer::singleShot(10, &a, SLOT(quit()));
    return a.exec();
}
