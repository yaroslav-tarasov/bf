#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include "bfcontrolbase.h"

namespace Ui {
class AddDialog;
}

class AddDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddDialog(QWidget *parent = 0);
    ~AddDialog();
    const filter_rule_t& getRule() const { return mFr;}
    void  setChain(bf_chain_t chain);
signals:
    void accept_rule(filter_rule_t);
    void reject_rule();
private slots:
    void rbHostname_toggled(bool);
    void rbAnyone_toggled(bool);
    void accept();
    void reject();
private:
    filter_rule_t  mFr;
    Ui::AddDialog *ui;
};

#endif // ADDDIALOG_H
