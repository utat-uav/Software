#pragma once
#include "stdafx.h"

class BarcodeReader
{
public:
	BarcodeReader();
	void scanImage(string imagePath);
	~BarcodeReader();
};

