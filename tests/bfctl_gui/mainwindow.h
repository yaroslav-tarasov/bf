#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QModelIndex>

#include "bflocalcontrol.h"

namespace Ui {
class MainWindow;
}

class RulesTableModel;
class QSettings;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showEvent(QShowEvent *ev);
    void closeEvent(QCloseEvent *ev);
    void changeEvent(QEvent* event);
private slots:
    void dataChanged(QModelIndex i1,QModelIndex i2);
    void applyChanges();

private:
    void writePositionSettings();
    void readPositionSettings();
private:
    Ui::MainWindow * ui;
    RulesTableModel* mRulesModel;
    QList<QAction*>  mRuleTableActions;
    bool             mGuiStateRestored;
    bool             mUncommitted;
    BFLocalControl*  mBfc;
};

#endif // MAINWINDOW_H
