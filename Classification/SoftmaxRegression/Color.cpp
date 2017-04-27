#include "stdafx.h"
#include "Color.h"


Color::Color(Vec3f _rgb, string name) : rgb_(_rgb), name(name)
{
	Mat3f matRgb(_rgb), matHls;
	cvtColor(matRgb, matHls, CV_RGB2HLS);
	hls_ = matHls.at<Vec3f>(0);
}


double Color::distanceFrom (const Color& other) const
{
	// x = hue, y = lightness, z = saturation
	Vec2f selfxy = Utils::polarToCartesian(Vec2f(1.0, hls_[0] * DEG_TO_RAD));
	Vec2f otherxy = Utils::polarToCartesian(Vec2f(1.0, other.hls_[0] * DEG_TO_RAD));
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
	Mat patch(100, 100, CV_8UC3, Scalar(255 * rgb_[2], 255 * rgb_[1], 255 * rgb_[0])); 
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
	colors[8] = Color(Vec3f(0.43, 0.263, 0.137), "brown");
	colors[9] = Color(Vec3f(1.0, 0.55, 0.3), "orange");

	// put into hashmap
	unordered_map<string, Color> colorMapping;
	for (int i = 0; i < colors.size(); ++i)
	{
		colorMapping.insert(make_pair(colors[i].name, colors[i]));
	}

	return colorMapping;
}