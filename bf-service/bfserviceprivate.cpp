#include "bfserviceprivate.h"

BfServicePrivate::BfServicePrivate(QObject *parent) :
    QObject(parent)
{

    mBfc = new BFControl();

}
