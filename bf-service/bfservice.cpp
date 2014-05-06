#include "bfservice.h"
#include "bfserviceprivate.h"
#include "qsyslog.h"
#include "bfrules.h"
#include "bfconfig.h"
#include <QDebug>
#include "qwaitfordone.h"

BfService::BfService(int argc, char **argv,const QString &name):QtService<QCoreApplication>(argc, argv, name)
{
    setObjectName("BfService");

    setServiceDescription(tr("Barrier mini-firewall service."));
    setStartupType(QtServiceController::AutoStartup);

    d = new BfServicePrivate;
    privateThread = new QThread(this);
    d->moveToThread(privateThread);

    connect(privateThread, SIGNAL(started()), d, SLOT(started()));
    connect(privateThread, SIGNAL(finished()), d, SLOT(finished()));

    // QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfService::BfService()"));

}

BfService::~BfService()
{
    // qLogInfo(objectName()) << tr("Destroyed");
}

void BfService::createApplication(int &argc, char **argv)
{
    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("Enter BfService::createApplication"));
    QtService<QCoreApplication>::createApplication(argc, argv);

    BFConfig bc(this);

    BfRules::getFromFile(BFConfig::getRulesCachePath());



//    InitMain::setupSearchPath();

//    if (QDir("translations:").exists("adpService_ru.qm"))
//    {
//        translator = new QTranslator(this);
//        translator->load("adpService_ru.qm", QDir("translations:").absolutePath());
//        QCoreApplication::installTranslator(translator);
//    }

//    if (!config())
//        throw QString();

//    if (!StukInteraction::instance("Adp service")->init())
//        qDebug() << tr("Could not init STUK");

//    connect(StukInteraction::instance(), SIGNAL(becomeMain()), SLOT(onBecomeMaster()));
//    connect(StukInteraction::instance(), SIGNAL(becomeReserv()), SLOT(onBecomeSlave()));

//    connect(outerWorker, SIGNAL(cksBad()), StukInteraction::instance(), SLOT(aftnLost()));
//    connect(outerWorker, SIGNAL(cksGood()), StukInteraction::instance(), SLOT(aftnFound()));

//    connect(outerWorker, SIGNAL(pivpUndefined()), StukInteraction::instance(), SLOT(pivpUndefined()));
//    connect(outerWorker, SIGNAL(pivpBad()), StukInteraction::instance(), SLOT(pivpLost()));
//    connect(outerWorker, SIGNAL(pivpGood()), StukInteraction::instance(), SLOT(pivpFound()));
}

void BfService::start()
{
    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfService::start()"));

//    initLog4Qt();

//    qLogInfo(objectName()) << tr("=========> Service started <=========");
//    qLogInfo(objectName()) << tr("%1, Version %2").arg(qApp->applicationName()
//                                                        , qApp->applicationVersion());
//    qLogInfo(objectName()) << tr("Build date: %1\n").arg(__DATE__);

//    qLogInfo(objectName()) << tr("Log level = %1").arg(config()->logLevel());
//    qLogInfo(objectName()) << tr("Log path: '%1'").arg(QFileInfo(config()->logPath()).canonicalFilePath());
//    qLogInfo(objectName()) << tr("Aerodromes: '%1'").arg(config()->allAirports().join("', '"));

     privateThread->start();
}

void BfService::startThreads()
{
//    qLogInfo(objectName()) << tr("Starting threads...");
//    threadsStarted = true;


//    qLogInfo(objectName()) << tr("Threads started");
}

void BfService::stop()
{
     QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfService::stop()"));
     //QWaitForDone w(d);

     d->stop();

     // w.start(20000);

     privateThread->quit();
     if (!privateThread->wait(20000))
     {
         privateThread->terminate();
     }

//    if (threadsStarted)
//        stopThreads();

//    qLogInfo(objectName()) << tr("Service stopped");
//    using namespace Log4Qt;
//    LogManager::rootLogger()->removeAllAppenders();
}

void BfService::stopThreads()
{
//    qLogInfo(objectName()) << tr("Stopping threads...");
//    threadsStarted = false;

//    qLogInfo(objectName()) << tr("Threads stopped");
}
