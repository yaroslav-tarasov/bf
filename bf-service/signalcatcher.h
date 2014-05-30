#ifndef SIGNALCATCHER_H
#define SIGNALCATCHER_H

#include <QObject>
#include <QSocketNotifier>

class SignalCatcher : public QObject
{
    Q_OBJECT

public:
    static void init();
    static SignalCatcher& instance();
private:
    // Unix signal handlers.
    static void intSignalHandler     (int unused);
    static void usrSignalHandler     (int unused);
    static void termSignalHandler    (int unused);
    static void hupSignalHandler     (int unused);
    static int  installSignalHandlers();
    void        initSignalHandles    ();

public slots:
    // Qt signal handlers.
    void        handleSigInt ();
    void        handleSigUsr ();
    void        handleSigTerm();
    void        handleSigHup ();


signals:
    void      sigInt ();
    void      sigUsr ();
    void      sigTerm();
    void      sigHup ();
public slots:

private:
    explicit SignalCatcher(QObject *parent = 0);
    Q_DISABLE_COPY(SignalCatcher)

    QSocketNotifier *m_SockNotifierInt;
    QSocketNotifier *m_SockNotifierUsr;
    QSocketNotifier *m_SockNotifierTerm;
    QSocketNotifier *m_SockNotifierHup;

    static int sigIntFd[2];
    static int sigUsrFd[2];
    static int sigTermFd[2];
    static int sigHupFd[2];
    
    
};

#endif // SIGNALCATCHER_H
