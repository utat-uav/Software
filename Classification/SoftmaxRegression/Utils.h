#pragma once
#include "stdafx.h"
#include "Classifier.h"
#define PI 3.141592653589793238462643383279502884L 
#define DEG_TO_RAD 0.017453292


namespace Utils
{
	Mat rotateImage(const Mat& image, int degrees);
	void cropImage(Mat& image);

	bool allInOneClassify(const Mat& image, Classifier *classifier, Classifier::Results &results);
	Vec2f polarToCartesian(Vec2f polarCoords);
}
