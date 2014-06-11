#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rulestablemodel.h"
#include "ruledelegate.h"

#include <QSettings>
#include <QMessageBox>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mGuiStateRestored(false),
    mUncommitted(false)
{
    ui->setupUi(this);
    mBfc = new BFLocalControl(this);


    mRulesModel = new RulesTableModel(this);
    ui->tableView->setItemDelegate(new RuleDelegate);
    ui->tableView->setModel(mRulesModel);


    if(mBfc->init()==0)
    {
        filter_rule_t fr;

        RulesTableModel::rules_list_ptr_t  model_list;
        QList<filter_rule_ptr > ruleslst;

        int ret =  mBfc->getRulesSync(fr,  ruleslst);
        if(ret>=0)
        {
            foreach (filter_rule_ptr p,ruleslst)
            {
                if(!policy_rule(&p->base) )
                {
                    model_list.append(p);
                }
            }

            mRulesModel->setRules(model_list);

            connect( mRulesModel, SIGNAL( dataChanged(QModelIndex,QModelIndex)),
                     this, SLOT( dataChanged(QModelIndex,QModelIndex)) );
        }
    }

    connect(ui->pbApply,SIGNAL(clicked()),SLOT(applyChanges()));
    ui->pbApply->setEnabled(mUncommitted);
}

MainWindow::~MainWindow()
{
     delete ui;
}

void MainWindow::writePositionSettings()
{
    QSettings s;

    s.beginGroup( "mainwindow" );

    s.setValue( "geometry", saveGeometry() );
    s.setValue( "savestate", saveState() );
    s.setValue( "maximized", isMaximized() );
    if ( !isMaximized() ) {
        s.setValue( "pos", pos() );
        s.setValue( "size", size() );
    }

    s.endGroup();
    s.setValue("rulestable/horizontalheader", ui->tableView->horizontalHeader()->saveState());

}

void MainWindow::readPositionSettings()
{
    QSettings s;

    s.beginGroup( "mainwindow" );

    restoreGeometry(s.value( "geometry", saveGeometry() ).toByteArray());
    restoreState(s.value( "savestate", saveState() ).toByteArray());
    move(s.value( "pos", pos() ).toPoint());
    resize(s.value( "size", size() ).toSize());
    if ( s.value( "maximized", isMaximized() ).toBool() )
        showMaximized();

    s.endGroup();

}


void MainWindow::showEvent(QShowEvent *ev)
{
    QSettings s;
    if(!mGuiStateRestored) {
        readPositionSettings();
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
            writePositionSettings();
            QMainWindow::closeEvent(ev);
        } else {
            ev->ignore();
        }
    } else {
        writePositionSettings();
        QMainWindow::closeEvent(ev);
    }


}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        // retranslate other widgets which weren't added in designer
        // retranslate();
    }

    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

// {SRCIP,SRCPORT,DSTIP,DSTPORT,PROTO,CHAIN,POLICY,OFF};
void MainWindow::dataChanged(QModelIndex i1,QModelIndex )
{
    mUncommitted = !mRulesModel->getDirty().isEmpty();
    ui->pbApply->setEnabled(mUncommitted);
}


void MainWindow::applyChanges()
{
   RulesTableModel::rules_list_ptr_t dirty_rules = mRulesModel->getDirty();
   foreach(const filter_rule_ptr& item, dirty_rules) {
        mBfc->updateRule(*item);
   }

   mRulesModel->clearDirty();
   mUncommitted = false;
   ui->pbApply->setEnabled(mUncommitted);
}
