#include "bfserviceprivate.h"
#include "qsyslog.h"
#include "bfrules.h"
#include "bfconfig.h"
#include "signalcatcher.h"

#include <QCoreApplication>

BfServicePrivate::BfServicePrivate(QObject *parent) :
    QObject(parent)
{
    mBfc = new BFControl(this);
    mLocalServer = new BFLocalServer(this);
}

void BfServicePrivate::started()
{
    T_INFO("BfServicePrivate::started()");

    SignalCatcher::init();
    connect(&SignalCatcher::instance(),SIGNAL(sigHup()),SLOT(onSignalHUP()));
    connect(&SignalCatcher::instance(),SIGNAL(sigInt()),SLOT(onSignalINT()));
    connect(&SignalCatcher::instance(),SIGNAL(sigTerm()),SLOT(onSignalTERM()));
    connect(&SignalCatcher::instance(),SIGNAL(sigUsr()),SLOT(onSignalUSR()));

    if(mBfc->create()==0)
    {
        QObject::connect(mBfc,SIGNAL(log(filter_rule_t)),this,SLOT(gotLog(filter_rule_t)));

        QList<filter_rule_ptr > rules_list;
        rules_list << BfRules::getByPattern(filter_rule_t(0,0,0,CHAIN_INPUT)) << BfRules::getByPattern(filter_rule_t(0,0,0,CHAIN_OUTPUT));

        foreach(filter_rule_ptr p,rules_list)
        {
            qDebug() << "BfServicePrivate::started" <<  *p;
        }

        mBfc->subscribeLog(getpid());
        T_INFO(QString("Try to subscribe with pid %1").arg(getpid()));

        filter_rule_t fr;
        memset(&fr,0,sizeof(filter_rule_t));
        fr.base.chain = CHAIN_INPUT;

        QList<filter_rule_ptr > fr_list;
        mBfc->getRulesSync(fr,fr_list,10000);
        //BfRules::loadFromList(fr_list);
        BfRules::saveToFile(BFConfig::instance().rulesCachePath());

    }
    else
    {
        T_ERROR(QString("Can't create netlink socket)"));
    }


    T_INFO(QString("Local server started with code %1").arg(mLocalServer->run()));
}

void BfServicePrivate::stop()
{
    T_INFO(QString("Enter BfServicePrivate::stop()"));
    filter_rule_t fr;
    memset(&fr,0,sizeof(filter_rule_t));
    fr.base.chain = CHAIN_INPUT;

    QList<filter_rule_ptr > fr_list;
    mBfc->getRulesSync(fr,fr_list,10000);
    //BfRules::loadFromList(fr_list);
    BfRules::saveToFile(BFConfig::instance().rulesCachePath());
    T_INFO(QString("Leave BfServicePrivate::stop()"));

}

void BfServicePrivate::finished()
{
    T_INFO(QString("Enter BfServicePrivate::finished()"));

    T_INFO(QString("Leave BfServicePrivate::finished()"));
}

void BfServicePrivate::gotLog(filter_rule_t fr)
{
    //QSyslog::instance().syslog(/*LOG_INFO*/6,QString("gotLog(filter_rule_t)"));

    qDebug() << fr;

}

bool     BfServicePrivate::event (QEvent *e)
{
    if (e->type() == QEvent::ThreadChange)
    {

    }

    return QObject::event(e);
}


void BfServicePrivate::onSignalHUP ()
{
    T_WARN(QString("HUP catched"));

}

void BfServicePrivate::onSignalINT ()
{
    T_WARN(QString("INT catched"));

}

void BfServicePrivate::onSignalTERM()
{
    T_WARN(QString("TERM catched"));
    qDebug() << "Terminating application...";
    QCoreApplication::exit();
}

void BfServicePrivate::onSignalUSR ()
{
    T_WARN(QString("USR catched"));

}
