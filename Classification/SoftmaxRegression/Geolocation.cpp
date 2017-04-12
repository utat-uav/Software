#include "stdafx.h"
#include "Geolocation.h"



// first point will be taken as the center from which we measure distances
double LatLon::distanceFrom(const LatLon& p2)
{
	double x1 = lon_rad * cos(lat_rad);
	double y1 = lat_rad;
	double x2 = p2.lon_rad * cos(lat_rad);
	double y2 = p2.lat_rad;
	double diff_x = x1 - x2;
	double diff_y = y1 - y2;
	return EARTH_RADIUS * sqrt(diff_x * diff_x + diff_y * diff_y);
}


void Geolocation::updateParams(unsigned _im_width, unsigned _im_height, double _heading, double _alt, LatLon _center)
{
	im_width = _im_width;
	im_height = _im_height;
	heading = _heading;
	alt = _alt;
	center = _center;
}


LatLon Geolocation::targLatLon(unsigned targ_row, unsigned targ_col)
{
	double fov_rad = DEG_TO_RAD * fov;
	double heading_rad = DEG_TO_RAD * heading;

	// pixel to meter conversion
	int p_x = targ_col - im_width / 2;
	int p_y = im_height / 2 - targ_row;
	double w_m = (alt - ground_level) * tan(fov_rad / 2.0);
	double m_x = p_x / (im_width / 2.0) * w_m;
	double m_y = p_y / (im_width / 2.0) * w_m;

	// rotate vector based on heading
	double cosConv = cos(heading_rad);
	double sinConv = sin(heading_rad);
	double d_x = cosConv*m_x + sinConv*m_y;
	double d_y = -sinConv*m_x + cosConv*m_y;

	// scale dx and dy
	d_x /= EARTH_RADIUS;
	d_y /= EARTH_RADIUS;

	double center_x = center.lon_rad * cos(center.lat_rad);
	double center_y = center.lat_rad;
	double coord_x = center_x + d_x;
	double coord_y = center_y + d_y;

	// convert back to latlon
	double targ_lat = coord_y;
	double targ_lon = coord_x / cos(center.lat_rad);
	return LatLon(RAD_TO_DEG * targ_lat, RAD_TO_DEG * targ_lon);
}


//--------------------------------------------------------------------
// TEST FUNCTIONS
//--------------------------------------------------------------------

// sample geolocation test
void test()
{
	// test settings
	double alt = 397.0;
	double ground_level = 188.0;

	// col corresponds to width, row corresponds to height (in order)
	unsigned im_width = 1566;
	unsigned im_height = 1033;
	unsigned targ_col = 139;
	unsigned targ_row = 71;
	double heading = 140.0;
	LatLon center(43.783137, -79.464309);
	LatLon targ(43.783226, -79.462765); // for evaluating the quality of prediction
	cout.precision(12);
	cout << "best fov: " << getBestFOV(targ_row, targ_col, im_width, im_height, heading, alt, center, targ, 300, 188.0) << endl;
	
	double test_fov = 75.0;
	Geolocation g(test_fov, ground_level);
	g.updateParams(im_width, im_height, heading, alt, center);
	LatLon predicted_targ = g.targLatLon(targ_row, targ_col);
	cout << "prediction: " << predicted_targ.lat << " " << predicted_targ.lon << endl;
}


// find the best fov for a given image and test coordinates
double getBestFOV(unsigned targ_row, unsigned targ_col, unsigned im_width, unsigned im_height, double heading, double alt,
	LatLon center, LatLon targ, int num_samples, double ground_level)
{
	double min_dist = FLT_MAX;
	double best_fov = -1;
	LatLon best_pred(0, 0);

	double min = 10;
	double max = 130;

	for (int i = 0; i < num_samples; ++i)
	{
		double fov = min + i * (max - min) / (num_samples - 1);
		Geolocation g(fov, ground_level);
		g.updateParams(im_width, im_height, heading, alt, center);
		LatLon predicted_targ = g.targLatLon(targ_row, targ_col);
		double dist = targ.distanceFrom(predicted_targ);
		if (dist < min_dist)
		{
			min_dist = dist;
			best_fov = fov;
			best_pred = predicted_targ;
		}
	}

	cout << "best pred: " << best_pred.lat << " , " << best_pred.lon << endl;
	return best_fov;
}

