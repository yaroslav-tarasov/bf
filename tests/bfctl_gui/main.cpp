#include <QApplication>
#include <QTranslator>

#include "mainwindow.h"
#include "bf_defs.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("vniira");
    QCoreApplication::setOrganizationDomain("vniira.ru");
    QCoreApplication::setApplicationName("BF Control GUI");

    QTranslator translator(0);
    QString translationFile = QString::fromUtf8("bfctl_gui_ru");
    translator.load(translationFile, BACS_DEFAULT_TRANSLATIONS_DIR);
    a.installTranslator(&translator);

    MainWindow w;

    w.show();
    
    return a.exec();
}
