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
    
};

#endif // BFSERVICEPRIVATE_H
