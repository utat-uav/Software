#ifndef LATLON_H
#define LATLON_H


class LatLon
{
public:
    LatLon();
    double getLat();
    double getLon();
    void setLat();
    void setLon();
private:
	double latitude;
	double longitude;

};

#endif // LATLON_H