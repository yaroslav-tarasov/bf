#ifndef BFCONFIG_H
#define BFCONFIG_H

#include <QObject>

class BFConfig : public QObject
{
    Q_OBJECT
public:
    explicit BFConfig(QObject *parent = 0);
    static QString getRulesCachePath();
signals:
    
public slots:
    
};

#endif // BFCONFIG_H
