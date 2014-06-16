#include "ruledelegate.h"

#include <QComboBox>
#include <QApplication>

#include "trx_data.h"
#include "bfctl_gui_defs.h"

// TODO разобраться и применить
//// get the QMetaEnum object
//const QMetaObject &mo = staticMetaObject;
//int enum_index = mo.indexOfEnumerator("bf_switch_rules_t");
//QMetaEnum metaEnum = mo.enumerator(enum_index);


//// convert to a string
//bf_switch_rules_t key = SW_NO;
//QByteArray str = metaEnum.valueToKey(key);
//qDebug() << "Value as str:" << str;

RuleDelegate::RuleDelegate()
{
}


QWidget *RuleDelegate::createEditor(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return editor;
    int column = index.model()->headerData(index.column(),Qt::Horizontal,bfmodel::IdRole).toInt();

    if (column ==  bfmodel::OFF) {
        QComboBox *pe = new QComboBox(editor);
        int current = index.model()->data(index, Qt::DisplayRole).toUInt();
        QStringList lst;
        lst << get_sw_name(SW_NO) << get_sw_name(SW_YES);
        if (pe) {
            pe->addItems(lst);
            pe->setCurrentIndex(current);
        }
        return pe;
    }
    else if (column ==  bfmodel::POLICY) {
        QComboBox *pe = new QComboBox(editor);
        int current = index.model()->data(index, Qt::DisplayRole).toUInt();
        QStringList lst;
        lst << get_policy_name(POLICY_DROP) << get_policy_name(POLICY_ACCEPT);
        if (pe) {
            pe->addItems(lst);
            pe->setCurrentIndex(current);
        }
        return pe;
    }



    return editor;
}

void RuleDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!index.isValid()) return;
    int column = index.model()->headerData(index.column(),Qt::Horizontal,bfmodel::IdRole).toInt();
    if (column == bfmodel::POLICY || column == bfmodel::OFF ) {
        QComboBox *pe = qobject_cast<QComboBox *>(editor);
        if (pe) {
            int value = index.model()->data(index, Qt::EditRole).toUInt();
            pe->setCurrentIndex(value);
        }
    }

}

void RuleDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid()) return;
    int column = index.model()->headerData(index.column(),Qt::Horizontal,bfmodel::IdRole).toInt();

    if (column == bfmodel::POLICY || column == bfmodel::OFF ) {
        QComboBox *pe = qobject_cast<QComboBox *>(editor);
        if (pe) {
            model->setData(index, pe->currentIndex());
        }
    }

}

void RuleDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void RuleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItemV4 o = option;


  o.text = index.model()->data(index, Qt::DisplayRole).toString();
  o.displayAlignment = Qt::AlignCenter;
  int column = index.model()->headerData(index.column(),Qt::Horizontal,bfmodel::IdRole).toInt();

//  if (index.column() < 6) o.state = QStyle::State_ReadOnly;
   QVariantList l = index.model()->data(index, bfmodel::DirtyRole).toList();

   if (l.contains(column))
      o.font.setBold(true);

  int value = index.model()->data(index, Qt::DisplayRole).toUInt();

  switch(column)
  {
      case bfmodel::SRCIP:     if(value ==0) o.text = index.model()->data(index, Qt::DisplayRole).toString()/*"ALL"*/; break;
      case bfmodel::SRCPORT:   if(value ==0) o.text = "ALL"; break;
      case bfmodel::DSTIP:     if(value ==0) o.text = index.model()->data(index, Qt::DisplayRole).toString()/*"ALL"*/; break;
      case bfmodel::DSTPORT:   if(value ==0) o.text = "ALL"; break;
      case bfmodel::PROTO:     o.text = get_proto_name(value); break;
      case bfmodel::CHAIN:     o.text = get_chain_name(static_cast<bf_chain_t>(value));  break;
      case bfmodel::POLICY:    o.text = get_policy_name(static_cast<bf_policy_t>(value));   break;
      case bfmodel::OFF:       o.text = get_sw_name(static_cast<bf_switch_rules_t>(value));  break;
  }


  QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &o, painter);
}

