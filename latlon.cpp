#include "latlon.h"

LatLon::LatLon(double lat, double lon)
{
    this->latitude = lat;
    this->longitude = lon;
}

LatLon::LatLon(){}

double LatLon::getLat(){
	return latitude;
}

double LatLon::getLon(){
	return longitude;
}

void LatLon::setLat(double lat){
    this->latitude = lat;
}

void LatLon::setLon(double lon){
    this->longitude = lon;
}
