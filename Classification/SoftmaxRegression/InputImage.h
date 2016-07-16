#pragma once
#include "stdafx.h"

class InputImage
{
private:
	Mat image;
	Mat originalImage;
	vector<int> label;
	char charLabel;
	int labelIndex;
	bool valid;
	void cropImage();

public:
	// If conserve memory, then original image will not be saved
	InputImage(string path, bool testImage = false, bool appendOne = true, bool noMemoryConservation = true);
	InputImage(Mat *directMat, char correctLabel, bool testImage = false, bool appendOne = true, bool noMemoryConservation = true);
	~InputImage();
	vector<float> getVectorizedImage();
	Mat* getImage();
	bool isValid();
	vector<int>* getLabelVector();
	char getCharLabel();
	int getLabelIndex();
	float **imageArray;
	Mat* getOriginalimage();
	static void rotateImage(Mat& image, int degrees);

	static int charToOneHotIndex(char c);
	static int oneHotIndexToChar(int i);
};

