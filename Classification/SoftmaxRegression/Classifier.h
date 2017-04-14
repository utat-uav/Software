#pragma once

#include "stdafx.h"

class Classifier
{
public:
	struct Results
	{
		char character;
		double characterConfidence;
		string description;
		// More to be added
	};

	Classifier(const string &folderPath, const string &programPath);
	~Classifier();

	bool initialize();
	int classify(const string &imagePath, Results &results);
	int classify(const Mat &imageMat, Results &results);
	static char getCharFromIdx(int idx);

private:
	string folderPath;
	string programPath;

	Session *session;

	void processImage(Mat &image);
	void classifyCharacterHelper(const vector<Mat> &images, vector<char> &chars, vector<double> &confidences);
	void classifyCharacter(const Mat &image, char &c, double &confidence);
};

