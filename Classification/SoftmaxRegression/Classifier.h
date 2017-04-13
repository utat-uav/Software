#pragma once

#include "stdafx.h"

class Classifier
{
public:
	Classifier(const string &folderPath, const string &programPath);
	~Classifier();

	bool initialize();
	int classify(const string &imagePath);

private:
	string folderPath;
	string programPath;

	void processImage(Mat &image);
};

