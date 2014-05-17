#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <QFile>
#include <QProcessEnvironment>

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

    if(bfc->create()==0)
    {

    filter_rule_t fr;
    std::string thename;
    int action = cmd_utils::parse_cmd_args(argc, argv,&fr,thename);

    if (action == CMD_APPEND) {
        qDebug() << "CMD_APPEND";
        int ret = bfc->addRule(fr);
        if(ret>0)
            qDebug() << "Add new rule:" << fr;
        else if(ret==-BF_ERR_ALREADY_HAVE_RULE)
            qDebug() << "Already have this rule:" << fr;

    } else if (action == CMD_LIST) {
        qDebug() <<"CMD_LIST";

#ifdef TEST_ASYNC_GET_RULES
        bfc->getRulesAsync(fr);
#else
        QList<BFControl::filter_rule_ptr > ruleslst;
        int ret =  bfc->getRulesSync(fr,  ruleslst);

        int i=0;

        foreach (BFControl::filter_rule_ptr rule,ruleslst){
            filter_rule_t fr = *static_cast<filter_rule_t*>(rule.data());
            qDebug() << "rule #" << i++ << "  " << fr;
        }

        qDebug() << "getRulesSync(fr,  ruleslst) returns = "  << ret;
#endif


    } else if (action == CMD_DELETE) {
        qDebug() << "CMD_DELETE";
        int ret = bfc->deleteRule(fr);
        if(ret>0)
            qDebug() << "Rule deleted:" << fr;
        else if(ret==-BF_ERR_MISSING_RULE)
            qDebug() << "Do not have this rule:" << fr;

    } else if (action == CMD_FLUSH) {
        qDebug() << "CMD_FLUSH";
        bfc->deleteRules(fr);
    } else if (action == CMD_UPDATE) {
        qDebug() << "CMD_UPDATE";
        int ret = bfc->updateRule(fr);
        if(ret>0)
            qDebug() << "Rule updated:" << fr;
        else if(ret==-BF_ERR_MISSING_RULE)
            qDebug() << "Do not have this rule:" << fr;

    } else if (action == CMD_SET_POLICY) {
        qDebug() << "CMD_SET_POLICY";
        bfc->setChainPolicy(fr);
    } else if (action == CMD_GET_FROM_FILE) {
        QList<BFControl::filter_rule_ptr > ruleslst;
        qDebug() << "CMD_GET_FROM_FILE";
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
 #ifdef TEST_ASYNC_GET_RULES
    std::cout << "Press  any key" << std::endl;
    std::cin.get();
 #endif
    bfc->close();
    QTimer::singleShot(10, &a, SLOT(quit()));
    return a.exec();
}
