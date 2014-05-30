#ifndef BFSERVICEPRIVATE_H
#define BFSERVICEPRIVATE_H

#include <QObject>
#include <QEvent>
#include "bfcontrol.h"
#include "bflocalserver.h"

class BfServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit BfServicePrivate(QObject *parent = 0);
    virtual bool     event   (QEvent *e);
protected:

private:
    BFControl*      mBfc;
    BFLocalServer*  mLocalServer;
signals:
    void done();

public slots:
    void started();
    void stop();
    void finished();
private slots:
    void gotLog      (filter_rule_t);
    void onSignalHUP ();
    void onSignalINT ();
    void onSignalTERM();
    void onSignalUSR ();


};

#endif // BFSERVICEPRIVATE_H
