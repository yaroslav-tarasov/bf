#ifndef BFSERVICEPRIVATE_H
#define BFSERVICEPRIVATE_H

#include <QObject>
#include "bfcontrol.h"


class BfServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit BfServicePrivate(QObject *parent = 0);

private:
    BFControl *mBfc;
signals:
    
public slots:
    void started();
    void finished();
private slots:
    void gotLog(filter_rule_t);
};

#endif // BFSERVICEPRIVATE_H
