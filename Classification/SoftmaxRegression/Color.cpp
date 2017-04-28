#include "stdafx.h"
#include "Color.h"


Color::Color(Vec3f rgb, string name) : rgb_(rgb), name(name)
{
	Mat3f matRgb(rgb), matHls;
	cvtColor(matRgb, matHls, CV_RGB2HLS);
	hls_ = matHls.at<Vec3f>(0);
}

Color::Color(Vec3i rgb, string name) : name(name)
{
	Mat(rgb).copyTo(rgb_);
	rgb_ /= 255.0;
	Mat3f matRgb(rgb_), matHls;
	cvtColor(matRgb, matHls, CV_RGB2HLS);
	hls_ = matHls.at<Vec3f>(0);
}

double Color::distanceFrom (const Color& other) const
{
	// x = hue, y = lightness, z = saturation
	Vec2f selfxy = Utils::polarToCartesian(Vec2f(hls_[1], hls_[0] * DEG_TO_RAD));
	Vec2f otherxy = Utils::polarToCartesian(Vec2f(other.hls_[1], other.hls_[0] * DEG_TO_RAD));
	Vec2f xyDiff = selfxy - otherxy;
	return xyDiff.dot(xyDiff);
}


double Color::colorVariance() const
{
	// use rgb values
	double var = 0;
	var += pow(rgb_[0] - rgb_[1], 2.0);
	var += pow(rgb_[0] - rgb_[2], 2.0);
	var += pow(rgb_[1] - rgb_[2], 2.0);
	return var;
}


void Color::visualize(bool wait) const
{
	Mat patch(100, 100, CV_8UC3, Scalar(255.0 * rgb_[2], 255.0 * rgb_[1], 255.0 * rgb_[0])); 
	imshow(name, patch);

	if (wait)
		waitKey(0);
}


// AUVSI colors 
// http://auvsi-suas-competition-interoperability-system.readthedocs.io/en/latest/specification.html
unordered_map<string, Color> Color::getAUVSIColors()
{
	vector<Color> colors(10);
	colors[0] = Color(Vec3f(1.0, 1.0, 1.0), "white");
	colors[1] = Color(Vec3f(0.0, 0.0, 0.0), "black");
	colors[2] = Color(Vec3f(0.5, 0.5, 0.5), "gray");
	colors[3] = Color(Vec3f(1.0, 0.0, 0.0), "red");
	colors[4] = Color(Vec3f(0.0, 0.0, 1.0), "blue");
	colors[5] = Color(Vec3f(0.0, 0.5, 0.0), "green");
	colors[6] = Color(Vec3f(1.0, 1.0, 0.0), "yellow");
	colors[7] = Color(Vec3f(0.5, 0.0, 0.5), "purple");
	colors[8] = Color(Vec3f(0.53, 0.38, 0.125), "brown");
	colors[9] = Color(Vec3f(1.0, 0.55, 0.3), "orange");

	// put into hashmap
	unordered_map<string, Color> colorMapping;
	for (int i = 0; i < colors.size(); ++i)
	{
		colorMapping.insert(make_pair(colors[i].name, colors[i]));
	}

	return colorMapping;
}


vector<Color> Color::getDecisionBoundaryColors()
{
	vector<Color> colors;
	unordered_map<string, Color> knownColors = getAUVSIColors();

	for (auto it = knownColors.begin(); it != knownColors.end(); ++it)
	{
		string name = it->first;
		if (name == "black" || name == "gray" || name == "white")
			continue;
		colors.push_back(it->second);
	}

	// 1 color with 2 names at each boundary,
	// take care to not double-count.

	// purple needs to assert its dominance in dark-brown / red territory 
	colors.push_back(Color(Vec3i(107, 0, 12), "purple"));

	// brown boundaries
	addBoundaryColor(Vec3i(112, 28, 0), "brown", "red", colors);
	addBoundaryColor(Vec3i(179, 62, 0), "brown", "red", colors);
	addBoundaryColor(Vec3i(214, 125, 0), "brown", "orange", colors);
	addBoundaryColor(Vec3i(168, 135, 0), "brown", "yellow", colors);
	addBoundaryColor(Vec3i(102, 7, 0), "brown", "purple", colors);

	// red boundaries 
	addBoundaryColor(Vec3i(245, 69, 0), "red", "orange", colors);
	addBoundaryColor(Vec3i(235, 0, 31), "red", "purple", colors);

	// orange boundaries
	addBoundaryColor(Vec3i(255, 175, 36), "orange", "yellow", colors);

	// purple boundaries
	addBoundaryColor(Vec3i(129, 0, 209), "purple", "blue", colors);
	
	// blue boundaries
	addBoundaryColor(Vec3i(31, 255, 199), "blue", "green", colors);

	// green boundaries
	addBoundaryColor(Vec3i(200, 255, 36), "green", "yellow", colors);

	// yellow boundaries 
	// ... 

	return colors;
}


void Color::addBoundaryColor(Vec3i rgb, string name1, string name2, vector<Color>& colors)
{
	colors.push_back(Color(rgb, name1));
	colors.push_back(Color(rgb, name2));
}