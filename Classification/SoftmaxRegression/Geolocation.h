#pragma once

#include <limits>
#define EARTH_RADIUS 6371000.0
#define DEG_TO_RAD 0.01745329
#define RAD_TO_DEG 57.295788


struct LatLon
{
	// input must be in degrees
	LatLon(double _lat, double _lon) : lat(_lat), lon(_lon), lat_rad(_lat*DEG_TO_RAD), lon_rad(_lon*DEG_TO_RAD) { ; }
	double distanceFrom(const LatLon& p2);

	// in degrees
	double lat;
	double lon;

	// in radians, for computations
	double lat_rad;
	double lon_rad;
};


class Geolocation
{
public:
	Geolocation(double _fov, double _ground_level) : fov(_fov), 
													 ground_level(_ground_level), 
												     center(LatLon(0,0)) {; }

	// geolocation on an image must be done after a call to updateParams
	// heading angle is measured from the North axis
	void updateParams(unsigned _im_width, unsigned _im_height, double _heading, double _alt, LatLon _center);
	
	// targ_row, targ_col = coordinate of target in the image matrix
	// returns latlon coordinates
	LatLon targLatLon(unsigned targ_row, unsigned targ_col);

private:
	double fov;
	double heading;
	double alt;
	LatLon center;
	unsigned im_width;
	unsigned im_height;

	// the actual height used in calculations will be alt - ground_level.
	double ground_level;
};


// for testing and adjusting FOV to be optimal
void test();
double getBestFOV(unsigned targ_row, unsigned targ_col, unsigned im_width, unsigned im_height, double heading, double alt,
	LatLon center, LatLon targ, int num_samples, double ground_level);
