#pragma once
#include "stdafx.h"

namespace Utils
{
	Mat rotateImage(const Mat& image, int degrees);
	void cropImage(Mat& image);
}
