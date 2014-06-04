#ifndef RULEDELEGATE_H
#define RULEDELEGATE_H

#include <QItemDelegate>

class RuleDelegate : public QItemDelegate
{
public:
    RuleDelegate();
    QWidget *createEditor(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:
    
public slots:
    
};

#endif // RULEDELEGATE_H
