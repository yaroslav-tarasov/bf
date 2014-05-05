#include "bfserviceprivate.h"
#include "qsyslog.h"
#include "bfrules.h"
#include "bfconfig.h"

BfServicePrivate::BfServicePrivate(QObject *parent) :
    QObject(parent)
{

    mBfc = new BFControl();

}

void BfServicePrivate::started()
{
    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfServicePrivate::started()"));
    if(mBfc->create()==0)
    {
        QObject::connect(mBfc,SIGNAL(log(filter_rule_t)),this,SLOT(gotLog(filter_rule_t)));

        mBfc->subscribeLog(getpid());
        QSyslog::instance().syslog(/*LOG_INFO*/6,QString("Try to subscribe with pid %1").arg(getpid()));

        filter_rule_t fr;
        memset(&fr,0,sizeof(filter_rule_t));
        fr.chain = CHAIN_INPUT;

        QList<BFControl::filter_rule_ptr > fr_list;
        mBfc->getRulesSync(fr,fr_list,10000);
        BfRules::getFromList(fr_list);
        BfRules::saveToFile(BFConfig::getRulesCachePath());

    }
    else
    {
        QSyslog::instance().syslog(/*LOG_INFO*/6,QString("Can't create netlink socket)"));
    }
}


void BfServicePrivate::finished()
{
    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfServicePrivate::finished()"));

    filter_rule_t fr;
    memset(&fr,0,sizeof(filter_rule_t));
    fr.chain = CHAIN_INPUT;

    QList<BFControl::filter_rule_ptr > fr_list;
    mBfc->getRulesSync(fr,fr_list,10000);
    BfRules::getFromList(fr_list);
    BfRules::saveToFile(BFConfig::getRulesCachePath());

}

void BfServicePrivate::gotLog(filter_rule_t fr)
{
    //QSyslog::instance().syslog(/*LOG_INFO*/6,QString("gotLog(filter_rule_t)"));

    qDebug() << fr;

}
