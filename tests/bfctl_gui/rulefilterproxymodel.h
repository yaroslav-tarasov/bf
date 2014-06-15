#ifndef RULEFILTERPROXYMODEL_H
#define RULEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class RuleFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit RuleFilterProxyModel(QObject *parent = 0);
//    QVariant         data       (const QModelIndex &index, int role) const;
//    bool             setData    (const QModelIndex &index, const QVariant &value, int role);
    int              columnCount(const QModelIndex &parent)      const;
signals:
    
public slots:
    \
protected:
    virtual bool	filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const;
};

#endif // RULEFILTERPROXYMODEL_H
