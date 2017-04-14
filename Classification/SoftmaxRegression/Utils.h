#pragma once
#include "stdafx.h"

class Classifier;

namespace Utils
{
	Mat rotateImage(const Mat& image, int degrees);
	void cropImage(Mat& image);

	bool allInOneClassify(const Mat& image, Classifier *classifier, string &description);
}
