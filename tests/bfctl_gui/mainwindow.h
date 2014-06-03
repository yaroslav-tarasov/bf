#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

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
private:
    void writePositionSettings();
    void readPositionSettings();
private:
    Ui::MainWindow * ui;
    RulesTableModel* mRulesModel;
    bool             mGuiStateRestored;
    bool             mUncommitted;
};

#endif // MAINWINDOW_H
