#include "mainwindow.h"
#include "geolocation.h"
#include "gpsdata.h"
#include "latlon.h"
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName( "UTAT-UAV" );
    QCoreApplication::setOrganizationDomain( "http://utat.skule.ca/?page_id=7733" );
    QCoreApplication::setApplicationName( "CV-Interface" );

    //********** TEST HERE **************//
    qDebug() << "hello world";
    double iLat = 43.65794, iLon = 79.40001, iAlt = 4020;
    GPSData* gps = new GPSData(0, 0, 0, iLat, iLon, iAlt);
    Geolocation* geo = new Geolocation(44, 44, 1007, 189, *gps);
    LatLon* result = &(geo->pixelToLocation(752, 36));
    qDebug() << QString("Lat: %1").arg(result->getLat());
    qDebug() << QString("Lon: %1").arg(result->getLon());


    MainWindow w;
    w.show();

    return a.exec();
}
