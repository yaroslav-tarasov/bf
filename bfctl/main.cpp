#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <QFile>
#include <QProcessEnvironment>
#include <QSharedPointer>

#include <iostream>


#include "bfcontrol.h"
#include "trx_data.h"
#include "utils.h"
#include "cmds.h"


namespace {


int processCommand(int action,cmd_utils::cmd_args& ca)
{
    QTextStream qout(stdout);

    filter_rule_t& fr = ca.fr;
    std::string& thename = ca.file_name;

    BFControl bfc(QCoreApplication::instance());

    if(bfc.init()==0)
    {

    if (action == CMD_APPEND) {
        //qDebug() << "CMD_APPEND";
        cmds::addRule(fr, qout, bfc);
    } else if (action == CMD_LIST) {
        //qDebug() <<"CMD_LIST";

        cmds::listRules(fr, qout, bfc);

    } else if (action == CMD_DELETE) {
        //qDebug() << "CMD_DELETE";
        cmds::deleteRule(fr, qout, bfc);

    } else if (action == CMD_FLUSH) {
        //qDebug() << "CMD_FLUSH";
        bfc.deleteRules(fr);
    } else if (action == CMD_UPDATE) {
        //qDebug() << "CMD_UPDATE";
        cmds::updateRule(fr, qout, bfc);

    } else if (action == CMD_SET_POLICY) {
        //qDebug() << "CMD_SET_POLICY";
        cmds::setChainPolicy(fr, qout, bfc);

    } else if (action == CMD_LOAD_FROM_FILE) {

        cmds::loadFromFile( thename, bfc);

    } else if (action == CMD_PRINT_HELP) {
        // qDebug() << "CMD_PRINT_HELP";
        cmd_utils::exit_printhelp();
    }

    }
    else
    {
        qCritical() << "Can't connect to bf module \n";
        return -1;
    }

    bfc.close();
    return 0;
}


}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList cmdline_args = QCoreApplication::arguments();

    filter_rule_t fr;
    std::string thename;
    cmd_utils::cmd_args ca(fr,thename);
    int action = cmd_utils::parse_cmd_args(argc, argv,ca);
    int ret = processCommand(action,ca);

    return ret;

}
