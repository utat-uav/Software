#include "latlon.h"

LatLon::LatLon(double lat, double lon)
{
	this.latitude = lat;
	this.longitude = lon;
}

double LatLon::getLat(){
	return latitude;
}

double LatLon::getLon(){
	return longitude;
}

void Latlon::setLat(double lat){
	this.latitude = lat;
}

void Latlon::setLon(double lon){
	this.longitude = lon;
}
