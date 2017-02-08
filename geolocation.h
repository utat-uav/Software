#include "gpsdata.h"
#include "latlon.h"

#define GEOLOCATION_H


#pragma once

class Geolocation {
  public:
    Geolocation(double FOV_x, double FOV_y, int width, int height, GPSData gpsData);
    ~Geolocation();
    LatLon pixelToLocation(int x, int y);

    

  private:
    void calcDistance(int x, int y, double* east, double* north);
    void rotateAbout(const double axis[4], double theta, double inout[4]);
    void rotateX(double u[3], double theta);
    void rotateY(double u[3], double theta);
    void rotateZ(double u[3], double theta);
    double findAngle(int pixelPos, int maxPixelPos, double FOV);
    // no longer in commission
    double findXoverZ(int x, int width, double FOV, double tilt);

    double altitude, latitude, longitude, FOV_x, FOV_y, roll, pitch, yaw;
    int width, height;

    static void printVector(const double u[3]);
};

