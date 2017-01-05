#include "gpsdata.h"

GPSData::GPSData(double yaw, double pitch, double roll, double latitude, double longitude, double altitude)
{
	this.yaw = yaw;
	this.pitch = pitch;
	this.roll = roll;
	this.latitude = latitude;
	this.longitude = longitude;
	this.altitude = altitude;
}

double GPSData::getYaw(){
	return yaw;
}

double GPSData::getPitch(){
	return pitch;
}

double GPSData::getRoll(){
	return roll;
}

double GPSData::getLatitude(){
	return latitude;
}

double GPSData::getLongitude(){
	return longitude;
}

double GPSData::getAltitude(){
	return altitude;
}

double GPSData::getRoll(){
	return altitude;
}

void GPSData::setYaw(double yaw){
	this.yaw = yaw;
}

void GPSData::setPitch(double pitch){
	this.pitch = pitch;
}

void GPSData::setRoll(double roll){
	this.roll = roll;
}

void GPSData::setLatitude(double latitude){
	this.latitude = latitude;
}

void GPSData::setLongitude(double longitude){
	this.longitude = longitude;
}

void GPSData::setAltitude(double altitude){
	this.altitude = altitude;
}