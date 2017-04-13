#pragma once

#include "stdafx.h"

class Classifier
{
public:
	Classifier(const string &folderPath, const string &programPath);
	~Classifier();

	bool initialize();
	int classify(const string &imagePath);
	static char getCharFromIdx(int idx);

private:
	string folderPath;
	string programPath;

	Session *session;

	void processImage(Mat &image);
	void classifyCharacterHelper(const Mat &image, char &c, double &confidence);
	void classifyCharacter(const Mat &image, char &c, double &confidence);
};

