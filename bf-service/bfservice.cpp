#include <QDebug>
//
#include "bfservice.h"
#include "bfserviceprivate.h"
#include "qsyslog.h"
#include "filelogger.h"
#include "bfrules.h"
#include "bfconfig.h"
#include "qwaitfordone.h"
//
#include <iostream>
//
#include "cbau.h"
#include "status.h"

using namespace logging;

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

    // BFConfig::instance(); // Что бы не зацикливаться в qDebug()

    logging::add_file_writer();
    logging::add_syslog_writer();

}

BfService::~BfService()
{
    // qLogInfo(objectName()) << tr("Destroyed");
    T_INFO(QString("Destroyed"));
}

void BfService::createApplication(int &argc, char **argv)
{
    T_INFO(QString("Enter BfService::createApplication"));

#if 0
    for (int i=0;i < argc;i++)
    {
        T_INFO(argv[i]);
    }
#endif

    QtService<QCoreApplication>::createApplication(argc, argv);


    BfRules::loadFromFile(BFConfig::instance().rulesCachePath());

    StatusValueType state = StatusValueBad;
    int8_t opCode = BAU_OP_SUCCESS;
    BAu_sendIntegrityMessage(BAU_INTEGRITY_END,"damned fucking component", opCode);


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
    T_INFO(QString("BfService::start()"));

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
     T_INFO(QString("BfService::stop()"));
     // QWaitForDone w(d); Для синхронного получение правил бессмысленно

     d->stop();

     // w.start(20000);

     privateThread->quit();
     if (!privateThread->wait(1000))
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
