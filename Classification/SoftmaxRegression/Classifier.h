#pragma once

#include "stdafx.h"

class Classifier
{
public:
	struct Results
	{
		char character;
		double characterConfidence;
		// More to be added
	};

	Classifier(const string &folderPath, const string &programPath);
	~Classifier();

	bool initialize();
	int classify(const string &imagePath, Results &results);
	static char getCharFromIdx(int idx);

private:
	string folderPath;
	string programPath;

	Session *session;

	void processImage(Mat &image);
	void classifyCharacterHelper(const Mat &image, char &c, double &confidence);
	void classifyCharacter(const Mat &image, char &c, double &confidence);
};

