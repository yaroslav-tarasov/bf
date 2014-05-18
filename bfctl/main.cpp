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

const int fieldWidth = 10;
int fieldWidthSaved;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream qout(stdout);
    fieldWidthSaved = qout.fieldWidth();

    QStringList cmdline_args = QCoreApplication::arguments();

    BFControl *bfc = new BFControl(&a);

    if(bfc->create()==0)
    {

    filter_rule_t fr;
    std::string thename;
    int action = cmd_utils::parse_cmd_args(argc, argv,&fr,thename);

    if (action == CMD_APPEND) {
        //qDebug() << "CMD_APPEND";
        int ret = bfc->addRule(fr);
        if(ret>=0){
            qout << "Add new rule:" << endl;
            printHeader2(qout);
            qout << qSetFieldWidth(fieldWidth) << fr;
            qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }
        else if(ret==-BF_ERR_ALREADY_HAVE_RULE)
        {
            qout << "Already have this rule:" << endl;
            printHeader2(qout);
            qout << qSetFieldWidth(fieldWidth) << fr;
            qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }
    } else if (action == CMD_LIST) {
        //qDebug() <<"CMD_LIST";

#ifdef TEST_ASYNC_GET_RULES
        bfc->getRulesAsync(fr);
#else
        QList<BFControl::filter_rule_ptr > ruleslst;
        int ret =  bfc->getRulesSync(fr,  ruleslst);
        int i;
        qout.setFieldAlignment(QTextStream::AlignLeft);

        // qout.setPadChar('-');

        bf_chain_t chains[2] = {CHAIN_INPUT,CHAIN_OUTPUT};
        bf_policy_t policies[2] = {static_cast<bf_policy_t>(ruleslst[0]->policy),static_cast<bf_policy_t>(ruleslst[1]->policy)};
        const int ch_num = sizeof(chains)/sizeof(chains[0]);
        for(int ch = 0; ch < ch_num; ++ch )
        {
            if( fr.base.chain==chains[ch] || fr.base.chain==CHAIN_ALL)
            {
                qout << "Chain " << get_chain_name(chains[ch])  << " (policy "<< get_policy_name(policies[ch]) << ")" << qSetFieldWidth(fieldWidthSaved) << endl;
                printHeader(qout);
            }

            i=0;

        foreach (BFControl::filter_rule_ptr rule,ruleslst){
            filter_rule_t fr_out = *static_cast<filter_rule_t*>(rule.data());
            if (i<ch_num)
            {
                i++;continue;
            }

            {
                if (fr_out.base.chain==chains[ch] && (fr.base.chain==chains[ch] || fr.base.chain==CHAIN_ALL ) )
                {
                    qout  << qSetFieldWidth(fieldWidth) << i++ - ch_num << fr_out;
                    qout << qSetFieldWidth(fieldWidthSaved) << endl;
                }
            }
        }
        qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }
        qout << qSetFieldWidth(fieldWidthSaved) << endl;
#endif


    } else if (action == CMD_DELETE) {
        //qDebug() << "CMD_DELETE";
        int ret = bfc->deleteRule(fr);
        if(ret>=0){
            qout << "Rule deleted:" << endl;
            printHeader2(qout);
            qout << qSetFieldWidth(fieldWidth) << fr;
            qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }
        else if(ret==-BF_ERR_MISSING_RULE)
        {
            qout << "Do not have this rule:"<< endl;
            printHeader2(qout);
            qout << qSetFieldWidth(fieldWidth) << fr;
            qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }

    } else if (action == CMD_FLUSH) {
        //qDebug() << "CMD_FLUSH";
        bfc->deleteRules(fr);
    } else if (action == CMD_UPDATE) {
        //qDebug() << "CMD_UPDATE";
        int ret = bfc->updateRule(fr);
        if(ret>=0)
        {
            qout << "Rule updated:"<< endl;
            printHeader2(qout);
            qout << qSetFieldWidth(fieldWidth) << fr;
            qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }
        else if(ret==-BF_ERR_MISSING_RULE)
        {
            qout << "Do not have this rule:"<< endl;
            printHeader2(qout);
            qout << qSetFieldWidth(fieldWidth) << fr;
            qout << qSetFieldWidth(fieldWidthSaved) << endl;
        }

    } else if (action == CMD_SET_POLICY) {
        //qDebug() << "CMD_SET_POLICY";
        bfc->setChainPolicy(fr);
    } else if (action == CMD_GET_FROM_FILE) {
        QList<BFControl::filter_rule_ptr > ruleslst;
        //qDebug() << "CMD_GET_FROM_FILE";
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
        // qDebug() << "CMD_PRINT_HELP";
        // exit_printhelp();
        qout << "Something wrong in command string";
    }

    }
    else
    {
        qCritical() << "Can't connect to bf module \n";
    }

 #ifdef TEST_ASYNC_GET_RULES
    std::cout << "Press  any key" << std::endl;
    std::cin.get();
    bfc->close();
    QTimer::singleShot(10, &a, SLOT(quit()));
    return a.exec();
#else
    bfc->close();
    return 0;
#endif
}
