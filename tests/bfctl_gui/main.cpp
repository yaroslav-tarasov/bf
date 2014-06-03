#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("vniira");
    QCoreApplication::setOrganizationDomain("vniira.ru");
    QCoreApplication::setApplicationName("BF Control GUI");

    MainWindow w;

    w.show();
    
    return a.exec();
}
