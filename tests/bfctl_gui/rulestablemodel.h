#ifndef RULESTABLEMODEL_H
#define RULESTABLEMODEL_H

#include <QAbstractTableModel>
#include "trx_data.h"

typedef  filter_rule_t  RuleEntry;


class RulesTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    typedef  QList<filter_rule_ptr > rules_list_t;
    explicit RulesTableModel(QObject *parent = 0);
    virtual ~RulesTableModel();
    int              rowCount   (const QModelIndex &parent) const;
    int              columnCount(const QModelIndex &parent) const;
    QVariant         data       (const QModelIndex &index, int role) const;
    QVariant         headerData (int section, Qt::Orientation orientation, int role) const;
    bool             setData    (const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags    flags      (const QModelIndex& index) const;

    void             setRules   (const rules_list_t& items); // filter_rule_ptr
    const RuleEntry& rule       (const QModelIndex& index) const;
    rules_list_t     rules      (const QModelIndexList& indexes) const;
    void             removeItems(const QModelIndexList& indexes);
    void             removeItem (const QModelIndex& index);
signals:
    
public slots:
private:
    rules_list_t     mItems;
};

#endif // RULESTABLEMODEL_H
