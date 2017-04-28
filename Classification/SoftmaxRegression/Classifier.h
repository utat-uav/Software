#pragma once

#include "stdafx.h"

class Color;

class Classifier
{
public:

	struct Results
	{
		char character;
		double characterConfidence;
		string shape;
		string description;
		string shapeColor;
		string characterColor;

		// More to be added
	};

	struct Params
	{
		Params()
		{
			lLowThresh = 0.1;
			lHighThresh = 0.80;
			varThresh = 0.001;
			confidenceThresh = 0.98;
			numRots = 18;
		}

		// for deciding if color should be grayscale
		double lLowThresh;
		double lHighThresh;
		double varThresh;

		// character classification
		int numRots;
		double confidenceThresh;
	};

	Classifier(const string &folderPath, const string &programPath);
	~Classifier();

	bool initialize();
	int classify(const string &imagePath, Results &results);
	int classify(const Mat &imageMat, Results &results);
	static char getCharFromIdx(int idx);

	// recognized AUVSI colors
	unordered_map<string, Color> knownColors;
	vector<Color> decisionBoundaryColors;

private:
	string folderPath;
	string programPath;
	Params params;

	Session *session;

	void processImage(Mat &image);
	void classifyCharacterHelper(const vector<Mat> &images, vector<char> &chars, vector<double> &confidences);
	void classifyCharacter(const Mat &image, char &c, double &confidence);
	void classifyColors(const Mat &image, vector<Mat> segmentedImages, string& shapeColor, string& letterColor);
	void classifyColorsHelper(const Color& color, Color& closest);
};

