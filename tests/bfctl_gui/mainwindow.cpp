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

//    QAction* addAct = new QAction(tr("Add"), this);
//    QAction* deleteAct = new QAction(tr("Delete"), this);
//    QAction* deleteAllAct = new QAction(tr("Delete"), this);

//    connect(addAct, SIGNAL(triggered()), SLOT(onAddRuleAct()));
//    connect(deleteAct, SIGNAL(triggered()), SLOT(onDeleteRuleAct()));
//    connect(deleteAllAct, SIGNAL(triggered()), SLOT(onDeleteAllRulesAct()));

//    mRuleTableActions << addAct;
//    mRuleTableActions << deleteAct;
//    mRuleTableActions << deleteAllAct;

//    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
//    fileMenu->addAction(tr("Commit changes")
//                        , this
//                        , SLOT(onCommitChangesAct()));
//    fileMenu->addSeparator();
//    fileMenu->addAction(tr("Exit")
//                        , this
//                        , SLOT(close()));


    mRulesModel = new RulesTableModel(this);
    ui->tableView->setItemDelegate(new RuleDelegate);
    ui->tableView->setModel(mRulesModel);


    if(mBfc->init()==0)
    {
        filter_rule_t fr;

        RulesTableModel::rules_list_t  model_list;
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

    // mRulesModel->getDirty();


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
    const filter_rule_t& fr =  mRulesModel->rule(i1);
    mBfc->updateRule(fr);
}
