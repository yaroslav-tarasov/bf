#ifndef BFSERVICE_H
#define BFSERVICE_H

#include <QObject>
#include <QThread>
#include <QSocketNotifier>

#include <qtservice.h>

class BfServicePrivate;

class BfService : public QObject,public QtService<QCoreApplication>
{
    Q_OBJECT
public:
    explicit BfService(int argc, char **argv,const QString &name);
    virtual ~BfService();
protected:
    void         createApplication(int &argc, char **argv);
    virtual void start            ();
    void         startThreads     ();
    virtual void stop             ();
    void         stopThreads      ();
    void         terminate        ();
private:
   QThread*  privateThread;
   BfServicePrivate *d;

public:
    // Unix signal handlers.
    static void intSignalHandler     (int unused);
    static void usrSignalHandler     (int unused);
    static void termSignalHandler    (int unused);
    static void hupSignalHandler     (int unused);
    static int  installSignalHandlers();
    void        initSignalHandles    ();

public slots:
    // Qt signal handlers.
    void        handleSigInt();
    void        handleSigUsr();
    void        handleSigTerm();
    void        handleSigHup();

private:
    QSocketNotifier *m_SockNotifierInt;
    QSocketNotifier *m_SockNotifierUsr;
    QSocketNotifier *m_SockNotifierTerm;
    QSocketNotifier *m_SockNotifierHup;

    static int sigIntFd[2];
    static int sigUsrFd[2];
    static int sigTermFd[2];
    static int sigHupFd[2];

signals:
    
public slots:
    
};

#endif // BFSERVICE_H
