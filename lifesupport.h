#ifndef LIFESUPPORT_H
#define LIFESUPPORT_H
#include <QObject>
#include <QProcess>
#include <QTextBrowser>
#include <QString>
#include <QPointF>

constexpr double EARTH_RADIUS_IN_METERS = 6372797.560856;
constexpr double DEG_TO_RAD = 0.017453292519943295769236907684886;

class LifeSupport : public QObject
{
public:
    LifeSupport(QProcess* classifier, QTextBrowser* consoleOutput) ;
    QProcess* classifier ;
    QTextBrowser* consoleOutput ;
    QString filePath, imagePath ;
    ~LifeSupport() ;
};

struct LatLon
{
    double lat = 0;
    double lon = 0;
    QPointF convertToXY(double avgLat);
    static LatLon xyToLatLon(QPointF xy, double avgLat);
};

struct TargetData
{
    QString imagePath;
    QString name;
    QString coord;
    int x;
    int y;
    LatLon latlon;
    QString desc;
};

#endif // LIFESUPPORT_H
