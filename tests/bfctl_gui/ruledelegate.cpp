#include "ruledelegate.h"

#include <QComboBox>
#include <QApplication>

#include "trx_data.h"


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

    if (index.column() == 7) {
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
    else if (index.column() == 6) {
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

    if (index.column() == 7 || index.column() == 6 ) {
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

    if (index.column() == 7 || index.column() == 6 ) {
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
  int value = index.model()->data(index, Qt::DisplayRole).toUInt();

  o.text = index.model()->data(index, Qt::DisplayRole).toString();
  o.displayAlignment = Qt::AlignCenter;

//  if (index.column() < 6) o.state = QStyle::State_ReadOnly;
//  if (index.column() > 5) o.font.setBold(true);

  switch(index.column())
  {
      case 0: if(value ==0) o.text = "ALL"; break;
      case 1: if(value ==0) o.text = "ALL"; break;
      case 2: if(value ==0) o.text = "ALL"; break;
      case 3: if(value ==0) o.text = "ALL"; break;

      case 4: o.text = get_proto_name(value); break;
      case 5: o.text = get_chain_name(static_cast<bf_chain_t>(value));  break;
      case 6: o.text = get_policy_name(static_cast<bf_policy_t>(value));  break;
      case 7: o.text = get_sw_name(static_cast<bf_switch_rules_t>(value));  break;
  }

  QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &o, painter);
}
