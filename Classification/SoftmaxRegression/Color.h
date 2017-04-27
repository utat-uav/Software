#pragma once
#include "stdafx.h"
#include "Utils.h"

class Color
{
public:
	Color() { ; }
	Color(Vec3f _rgb, string name = "");
	string name;
	double distanceFrom (const Color& other) const;
	Vec3f rgb() const { return rgb_; }
	Vec3f hls() const { return hls_; }

	// can be helpful for deciding if a color is likely grayscale
	double colorVariance() const;

	// uses cv::waitKey if wait == true
	void visualize(bool wait = true) const;

	// initializes known AUVSI colors along with their names
	static unordered_map<string, Color> getAUVSIColors();

private:
	Vec3f rgb_ = Vec3f(0.0, 0.0, 0.0);
	Vec3f hls_ = Vec3f(0.0, 0.0, 0.0);
};

