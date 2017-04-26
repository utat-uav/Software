#pragma once
#include "stdafx.h"
#include "Utils.h"

class Color
{
public:
	Color() { ; }
	Color(Vec3f _rgb, string name = "");
	string name;
	Vec3f rgb, hls;
	double distanceFrom (const Color& other) const;

	// can be helpful for deciding if a color is likely grayscale
	double colorVariance() const;

	// uses cv::waitKey if wait == true
	void visualize(bool wait = true) const;

	// initializes known AUVSI colors along with their names
	static unordered_map<string, Color> getAUVSIColors();
};

