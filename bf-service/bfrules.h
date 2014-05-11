#ifndef BFRULES_H
#define BFRULES_H

#include <QObject>
#include <QMutex>
#include "bfcontrol.h"

class BfRules : public QObject
{
    Q_OBJECT
public:
    static QList<BFControl::filter_rule_ptr > loadFromFile(const QString& thename);
    static bool   saveToFile(const QString &thename);
    static void loadFromList(const QList<BFControl::filter_rule_ptr > &list);
    static QHash<filter_rule_base,BFControl::filter_rule_ptr> sRules;
    static QList<BFControl::filter_rule_ptr > getByPattern(const filter_rule_t  &fr);
signals:
    
public slots:

private:
    static QMutex  mMux;
    explicit BfRules(QObject *parent = 0);
    Q_DISABLE_COPY(BfRules)
};

#endif // BFRULES_H
