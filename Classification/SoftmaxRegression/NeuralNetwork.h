#pragma once
#include "stdafx.h"
#include "TrainingSet.h"

class NeuralNetwork
{
private:
	Mat parameters;
	float **parameterPointers;
	void predictHelper(Mat &V, vector<float> &predictions);

public:
	NeuralNetwork();
	NeuralNetwork(string serializedPath);
	~NeuralNetwork();
	void train(TrainingSet &trainingSet);
	void test(TrainingSet &trainingSet);
};

