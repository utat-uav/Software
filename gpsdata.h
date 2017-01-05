#ifndef GPSDATA_H
#define GPSDATA_H


#pragma once

class GPSData {
  public:
    GPSData(double yaw, double pitch, double roll, double latitude, double longitude, double altitude);
    double getYaw();
    double getPitch();
    double getRoll();
    double getLatitude();
    double getLongitude();
    double getAltitude();
    void setYaw(double yaw);
    void setPitch(double pitch);
    void setRoll(double roll);
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    void setAltitude(double altitude);
  private:
  	double yaw;
  	double pitch;
  	double roll;
  	double latitude;
  	double longitude;
  	double altitude;
};  
#endif // GPSDATA_H
	