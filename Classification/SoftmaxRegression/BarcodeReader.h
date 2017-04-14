#pragma once
#include "stdafx.h"

class BarcodeReader
{
public:
	BarcodeReader();
	string scanImage(const string &imagePath);
	string scanImage(const Mat &image);
	~BarcodeReader();
};

