#ifndef LATLON_H
#define LATLON_H


class LatLon
{
public:
    LatLon();
    LatLon(double lat, double lon);
    double getLat();
    double getLon();
    void setLat(double lat);
    void setLon(double lon);
private:
	double latitude;
	double longitude;

};

#endif // LATLON_H
