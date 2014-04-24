#ifndef BFSERVICE_H
#define BFSERVICE_H

#include <QObject>
#include <QThread>

#include <qtservice.h>

class BfServicePrivate;

class BfService : public QObject,public QtService<QCoreApplication>
{
    Q_OBJECT
public:
    explicit BfService(int argc, char **argv,const QString &name);
    virtual ~BfService();
protected:
    void createApplication(int &argc, char **argv);
    virtual void start();
    void startThreads();
    virtual void stop();
    void stopThreads();
private:
   QThread*  privateThread;
   BfServicePrivate *d;
signals:
    
public slots:
    
};

#endif // BFSERVICE_H
