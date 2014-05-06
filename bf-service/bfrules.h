#ifndef BFRULES_H
#define BFRULES_H

#include <QObject>
#include "bfcontrol.h"

class BfRules : public QObject
{
    Q_OBJECT
public:
    static QList<BFControl::filter_rule_ptr > getFromFile(const QString& thename);
    static bool   saveToFile(const QString &thename);
    static void getFromList(const QList<BFControl::filter_rule_ptr > &list);
    static QHash<filter_rule_base,BFControl::filter_rule_ptr> sRules;
    static QList<BFControl::filter_rule_ptr > getByPattern(const filter_rule_t  &fr);
signals:
    
public slots:

private:
    explicit BfRules(QObject *parent = 0);
    Q_DISABLE_COPY(BfRules)
};

#endif // BFRULES_H
