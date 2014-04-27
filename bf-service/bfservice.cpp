#include "bfservice.h"
#include "bfserviceprivate.h"
#include "qsyslog.h"
#include <QDebug>

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

    QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfService::BfService()"));

}

BfService::~BfService()
{
    // qLogInfo(objectName()) << tr("Destroyed");
}

void BfService::createApplication(int &argc, char **argv)
{
    QtService<QCoreApplication>::createApplication(argc, argv);

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

//    QString fplStandard = tr("Unknown");
//    switch (config()->fplStandard())
//    {
//    case 0:
//        fplStandard = tr("FPL 2007");
//        break;
//    case 1:
//        fplStandard = tr("FPL 2012");
//        break;
//    case 2:
//        fplStandard = tr("Mixed mode");
//        break;
//    }
//    qLogInfo(objectName()) << tr("FPL standard: %1\n").arg(fplStandard);

//    StukInteraction::instance()->procStarted(StukInteraction::instance()->isControlled());

//    QStringList args = QCoreApplication::arguments();

//    if (!StukInteraction::instance()->isControlled() || args.contains("-f") || args.contains("--force"))
//    {
//        if (args.contains("-s") || args.contains("--slave"))
//            onBecomeSlave();
//        else
//            onBecomeMaster();
//    }

     privateThread->start();
}

void BfService::startThreads()
{
//    qLogInfo(objectName()) << tr("Starting threads...");
//    threadsStarted = true;

//    initAftnParseParameters();

//    internalWorker->start();

//    outerWorker->start();
//    //outerWorker->createSockets();

//    cleaner->start();

//    mPingServer->start(config()->servicePort());

//    mainDbWorkerThread->start();

//    if (config()->briefingEnabled())
//        briefingWorkerThread->start();

//    qLogInfo(objectName()) << tr("Threads started");
}

void BfService::stop()
{
     QSyslog::instance().syslog(/*LOG_INFO*/6,QString("BfService::stop()"));

     privateThread->quit();

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

//    cleaner->stop();
//    if (config()->briefingEnabled())
//    {
//        briefingWorkerThread->quit();
//        if (!briefingWorkerThread->wait(1000))
//        {
//            qLogWarn(objectName()) << tr("Terminating briefing worker");
//            briefingWorkerThread->terminate();
//        } else
//        {
//            qLogWarn("BriefingWorker") << tr("Stopped");
//        }
//    }
//    internalWorker->stop();
//    if (!internalWorker->wait(1000))
//    {
//        qLogWarn(objectName()) << tr("Terminating internar worker");
//        internalWorker->terminate();
//    }
//    outerWorker->stop();
//    if (!outerWorker->wait(1000))
//    {
//        qLogWarn(objectName()) << tr("Terminating outer worker");
//        outerWorker->terminate();
//    }
//    mainDbWorkerThread->quit();
//    if (!mainDbWorkerThread->wait(1000))
//    {
//        qLogWarn(objectName()) << tr("Terminating main db worker");
//        mainDbWorkerThread->terminate();
//    } else
//    {
//        qLogWarn("MainDbService") << tr("Stopped");
//    }
//    qLogInfo(objectName()) << tr("Threads stopped");
}
