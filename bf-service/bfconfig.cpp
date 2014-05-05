#include "bfconfig.h"
#include <QProcessEnvironment>
#include <QDebug>

BFConfig::BFConfig(QObject *parent) :
    QObject(parent)
{
    QStringList environment = QProcessEnvironment::systemEnvironment().toStringList();

    qDebug() << environment;
    qDebug() << QProcessEnvironment::systemEnvironment().value("_");
}


QString BFConfig::getRulesCachePath()
{
    return QString("/sintez/sintez/bin/.rules");
}
