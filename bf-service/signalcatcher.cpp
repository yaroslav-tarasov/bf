#include "signalcatcher.h"
#include <QDebug>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>


int SignalCatcher::sigIntFd [2];
int SignalCatcher::sigUsrFd [2];
int SignalCatcher::sigTermFd[2];
int SignalCatcher::sigHupFd [2];


SignalCatcher::SignalCatcher(QObject *parent):
    QObject(parent)
{
  initSignalHandles();
  installSignalHandlers();
}

void SignalCatcher::init()
{
    instance();
}


SignalCatcher& SignalCatcher::instance()
{
    static SignalCatcher sc;
    return sc;
}

void SignalCatcher::termSignalHandler(int unused) {
    Q_UNUSED(unused)
    char a = 1;
    if(::write(sigTermFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "SignalCatcher::termSignalHandler: can't write signal number!";
    }
}

void SignalCatcher::hupSignalHandler(int unused) {
    Q_UNUSED(unused)
    char a = 1;
    if(::write(sigHupFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "SignalCatcher::hupSignalHandler: can't write signal number!";
    }
}

void SignalCatcher::intSignalHandler(int unused) {
    Q_UNUSED(unused);
    char a = 1;
    if(::write(sigIntFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "SignalCatcher::intSignalHandler: can't write signal number!";
    }
}

void SignalCatcher::usrSignalHandler(int unused) {
    Q_UNUSED(unused);
    char a = 1;
    if(::write(sigUsrFd[0], &a, sizeof(a)) < 0) {
        qDebug() << "SignalCatcher::usrSignalHandler: can't write signal number!";
    }
}

int SignalCatcher::installSignalHandlers()
{
    struct sigaction intr, usr, term, hup;

    intr.sa_handler = &SignalCatcher::intSignalHandler;
    sigemptyset(&intr.sa_mask);
    intr.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &intr, NULL) < 0) {
        perror("sigaction(INT)");
        return -1;
    }

    usr.sa_handler = &SignalCatcher::usrSignalHandler;
    sigemptyset(&usr.sa_mask);
    usr.sa_flags = SA_SIGINFO;
    if(sigaction(SIGUSR2, &usr, NULL) < 0) {
        perror("sigaction(USR2)");
        return -1;
    }

    term.sa_handler = &SignalCatcher::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = SA_SIGINFO;
    if(sigaction(SIGTERM, &term, NULL) < 0) {
        perror("sigaction(TERM)");
        return -1;
    }

    hup.sa_handler = &SignalCatcher::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = SA_SIGINFO;
    if(sigaction(SIGHUP, &hup, NULL) < 0) {
        perror("sigaction(HUP)");
        return -1;
    }

    return 0;
}

void SignalCatcher::initSignalHandles()
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
void SignalCatcher::handleSigInt() {
    m_SockNotifierInt->setEnabled(false);
    char tmp;
    if(::read(sigIntFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "SignalCatcher::handleSigInt: can't read signal number!";
    }

    qDebug() << "SIGINT unix signal";

    emit sigInt();

    // do Qt stuff
    m_SockNotifierInt->setEnabled(true);
}

void SignalCatcher::handleSigUsr() {
    m_SockNotifierUsr->setEnabled(false);
    char tmp;
    if(::read(sigUsrFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "SignalCatcher::handleSigUsr: can't read signal number!";
    }

    qDebug() << "SIGUSR unix signal";
    emit sigUsr();

    // do Qt stuff
    m_SockNotifierUsr->setEnabled(true);
}

void SignalCatcher::handleSigTerm() {
    m_SockNotifierTerm->setEnabled(false);
    char tmp;
    if(::read(sigTermFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "SignalCatcher::handleSigTerm: can't read signal number!";
    }

    qDebug() << "SIGTERM unix signal";
    
    emit sigTerm();
    
    // do Qt stuff
    m_SockNotifierTerm->setEnabled(true);
}

void SignalCatcher::handleSigHup() {
    m_SockNotifierHup->setEnabled(false);
    char tmp;
    if(::read(sigHupFd[1], &tmp, sizeof(tmp)) < 0) {
        qDebug() << "SignalCatcher::handleSigHup: can't read signal number!";
    }

    qDebug() << "SIGHUP unix signal";
    
    emit sigHup();
    
    // do Qt stuff
    m_SockNotifierHup->setEnabled(true);
}
