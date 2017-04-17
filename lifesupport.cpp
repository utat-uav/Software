#include "lifesupport.h"

LifeSupport::LifeSupport(QProcess *classifier, QTextBrowser *consoleOutput){
    this->classifier=classifier;
    this->consoleOutput=consoleOutput;
}

LifeSupport::~LifeSupport(){
    delete classifier;
    delete consoleOutput;
}

/*
 * avg is in degrees
 */
QPointF LatLon::convertToXY(double avgLat)
{
    return QPointF(lon * std::cos(avgLat * DEG_TO_RAD), lat);
}
