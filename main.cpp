#include "mainwindow.h"
#include <QApplication>
#include <QProcess>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName( "UTAT-UAV" );
    QCoreApplication::setOrganizationDomain( "http://utat.skule.ca/?page_id=7733" );
    QCoreApplication::setApplicationName( "CV-Interface" );

    MainWindow w;
    w.show();

    return a.exec();
}
