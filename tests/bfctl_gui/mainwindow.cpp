#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bflocalcontrol.h"
#include "rulestablemodel.h"

#include <QSettings>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mGuiStateRestored(false),
    mUncommitted(false)
{
    ui->setupUi(this);
    BFLocalControl bfc(this);

    mRulesModel = new RulesTableModel(this);
    ui->tableView->setModel(mRulesModel);

    if(bfc.init()==0)
    {
        filter_rule_t fr;

        RulesTableModel::rules_list_t  model_list;
        QList<filter_rule_ptr > ruleslst;

        int ret =  bfc.getRulesSync(fr,  ruleslst);
        if(ret>=0)
        {
            foreach (filter_rule_ptr p,ruleslst)
            {
                if(!policy_rule(&p->base) )
                {
                    model_list.append(RuleEntry(*p));
                }
            }

            mRulesModel->setRules(model_list);
        }
    }

}

MainWindow::~MainWindow()
{
    QSettings s;
    s.setValue("mainwindow/pos", pos());
    s.setValue("rulestable/horizontalheader", ui->tableView->horizontalHeader()->saveState());
    delete ui;
}

void MainWindow::showEvent(QShowEvent *ev)
{
    QSettings s;
    if(!mGuiStateRestored) {
        move(s.value("mainwindow/pos").toPoint());
        ui->tableView->horizontalHeader()->restoreState(s.value("rulestable/horizontalheader").toByteArray());
        mGuiStateRestored = true;
    }

    QMainWindow::showEvent(ev);
}

void MainWindow::closeEvent(QCloseEvent* ev) {
    if(mUncommitted) {
        QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Uncommitted data")
                             , tr("There are uncommitted data. Commit changes?")
                             , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(reply == QMessageBox::Yes) {
            //TODO// onCommitChangesAct();
            //TODO// m_ExitScheduled = true;
            ev->ignore();
        } else if(reply == QMessageBox::No) {
            QMainWindow::closeEvent(ev);
        } else {
            ev->ignore();
        }
    } else {
        QMainWindow::closeEvent(ev);
    }
}
