#include <QDebug>
//
#include "bfservice.h"
#include "bfserviceprivate.h"
#include "qsyslog.h"
#include "filelogger.h"
#include "bfrules.h"
#include "bfconfig.h"
#include "qwaitfordone.h"
#include "barrier_integration.h"
//
#include <iostream>
#include <signal.h>

//
#include "cbau.h"
#include "status.h"


int BfService::sigIntFd [2];
int BfService::sigUsrFd [2];
int BfService::sigTermFd[2];
int BfService::sigHupFd [2];

using namespace logging;

BfService::BfService(int argc, char **argv,const QString &name):QtService<QCoreApplication>(argc, argv, name)
{
    setObjectName("BfService");
    initSignalHandles();

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

    StatusValueType state = StatusValueOk;
    int8_t opCode = BAU_OP_SUCCESS;
    BAu_sendMessage(BAU_FW_RULE_ADD, opCode, "woooooooot" );


//    if (QDir("translations:").exists("adpService_ru.qm"))
//    {
//        translator = new QTranslator(this);
//        translator->load("adpService_ru.qm", QDir("translations:").absolutePath());
//        QCoreApplication::installTranslator(translator);
//    }

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

     d->stop();

     privateThread->quit();
     if (!privateThread->wait(1000))
     {
         privateThread->terminate();
     }

}

void BfService::stopThreads()
{
//    qLogInfo(objectName()) << tr("Stopping threads...");
//    threadsStarted = false;

//    qLogInfo(objectName()) << tr("Threads stopped");
}

void BfService::terminate() {
    qDebug() << "Terminating application...";
    QCoreApplication::exit();
}

void BfService::termSignalHandler(int unused) {
    Q_UNUSED(unused)
    char a = 1;
    if(::write(sigTermFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "BfService::termSignalHandler: can't write signal number!";
    }
}

void BfService::hupSignalHandler(int unused) {
    Q_UNUSED(unused)
    char a = 1;
    if(::write(sigHupFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "BfService::hupSignalHandler: can't write signal number!";
    }
}

void BfService::intSignalHandler(int unused) {
    Q_UNUSED(unused);
    char a = 1;
    if(::write(sigIntFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "BfService::intSignalHandler: can't write signal number!";
    }
}

void BfService::usrSignalHandler(int unused) {
    Q_UNUSED(unused);
    char a = 1;
    if(::write(sigUsrFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "BfService::usrSignalHandler: can't write signal number!";
    }
}

int BfService::installSignalHandlers()
{
    struct sigaction intr, usr, term, hup;

    intr.sa_handler = &BfService::intSignalHandler;
    sigemptyset(&intr.sa_mask);
    intr.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &intr, NULL) < 0) {
        perror("sigaction(INT)");
        return -1;
    }

    usr.sa_handler = &BfService::usrSignalHandler;
    sigemptyset(&usr.sa_mask);
    usr.sa_flags = SA_SIGINFO;
    if(sigaction(SIGUSR2, &usr, NULL) < 0) {
        perror("sigaction(USR2)");
        return -1;
    }

    term.sa_handler = &BfService::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = SA_SIGINFO;
    if(sigaction(SIGTERM, &term, NULL) < 0) {
        perror("sigaction(TERM)");
        return -1;
    }

    hup.sa_handler = &BfService::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = SA_SIGINFO;
    if(sigaction(SIGHUP, &hup, NULL) < 0) {
        perror("sigaction(HUP)");
        return -1;
    }

}

void BfService::initSignalHandles()
{

    if(::socketpair(AF_UNIX, SOCK_STREAM, 0, sigIntFd))
        qFatal("Couldn't create INT socketpair");
    if(::socketpair(AF_UNIX, SOCK_STREAM, 0, sigUsrFd))
        qFatal("Couldn't create USR2 socketpair");
    if(::socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermFd))
        qFatal("Couldn't create TERM socketpair");
    if(::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupFd))
        qFatal("Couldn't create HUP socketpair");

    m_SockNotifierInt = new QSocketNotifier(sigIntFd[1], QSocketNotifier::Read, this);
    connect(m_SockNotifierInt, SIGNAL(activated(int)), this, SLOT(handleSigInt()));

    m_SockNotifierUsr = new QSocketNotifier(sigUsrFd[1], QSocketNotifier::Read, this);
    connect(m_SockNotifierUsr, SIGNAL(activated(int)), this, SLOT(handleSigUsr()));

    m_SockNotifierTerm = new QSocketNotifier(sigTermFd[1], QSocketNotifier::Read, this);
    connect(m_SockNotifierTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));

    m_SockNotifierHup = new QSocketNotifier(sigHupFd[1], QSocketNotifier::Read, this);
    connect(m_SockNotifierHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));


}
void BfService::handleSigInt() {
    m_SockNotifierInt->setEnabled(false);
    char tmp;
    if(::read(sigIntFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "BfService::handleSigInt: can't read signal number!";
    }

    qDebug() << "SIGINT unix signal";

    stop();
    //TODO// saveState();
    terminate();

    // do Qt stuff
    m_SockNotifierInt->setEnabled(true);
}

void BfService::handleSigUsr() {
    m_SockNotifierUsr->setEnabled(false);
    char tmp;
    if(::read(sigUsrFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "BfService::handleSigUsr: can't read signal number!";
    }

    qDebug() << "SIGUSR unix signal";
    //TODO//saveState();

    // do Qt stuff
    m_SockNotifierUsr->setEnabled(true);
}

void BfService::handleSigTerm() {
    m_SockNotifierTerm->setEnabled(false);
    char tmp;
    if(::read(sigTermFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "BfService::handleSigTerm: can't read signal number!";
    }

    qDebug() << "SIGTERM unix signal";
    stop();
    //TODO//saveState();
    terminate();

    // do Qt stuff
    m_SockNotifierTerm->setEnabled(true);
}

void BfService::handleSigHup() {
    m_SockNotifierHup->setEnabled(false);
    char tmp;
    if(::read(sigHupFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "BfService::handleSigHup: can't read signal number!";
    }

    qDebug() << "SIGHUP unix signal";
    //TODO//saveState();

    // do Qt stuff
    m_SockNotifierHup->setEnabled(true);
}


