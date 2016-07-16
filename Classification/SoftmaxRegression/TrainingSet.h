#pragma once
#include "stdafx.h"
#include "InputImage.h"

class TrainingSet
{
private:
	vector<InputImage *> data;

public:
	TrainingSet(string path, bool testSet = false, bool noMemoryConserve = true);
	~TrainingSet();

	vector<InputImage *>* getData();
	int getDataSize();
	
	InputImage* operator[](int i);
};

