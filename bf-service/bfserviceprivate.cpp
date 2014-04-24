#include "bfserviceprivate.h"
#include "qsyslog.h"

BfServicePrivate::BfServicePrivate(QObject *parent) :
    QObject(parent)
{

    mBfc = new BFControl();

}

void BfServicePrivate::started()
{
    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfServicePrivate::started()"));
    if(mBfc->create())
    {
        QObject::connect(mBfc,SIGNAL(log(filter_rule_t)),this,SLOT());

        mBfc->subscribeLog(getpid());
    }
}


void BfServicePrivate::finished()
{
    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfServicePrivate::finished()"));


}

void BfServicePrivate::gotLog(filter_rule_t fr)
{
    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("gotLog(filter_rule_t)"));

}
