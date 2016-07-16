#pragma once
#include "FileManager.h"
#include "TrainingSet.h"
#include "BarcodeReader.h"
#include "Segmenter.h"

class ConvolutionalNeuralNetwork
{
private:
	bool initialized;
	vector<vector<vector<vector<float>>>> Wconv1;
	vector<vector<vector<vector<float>>>> Wconv2;
	vector<float> bconv1;
	vector<float> bconv2;
	vector<float> bfc1;
	vector<float> bfc2;
	vector<vector<float>> Wfc1;
	vector<vector<float>> Wfc2;

	string programPath;

	void initConvTensor(string serializedConv, vector<vector<vector<vector<float>>>> &convLayer, int dim1, int dim2, int dim3, int dim4);
	void initBTensor(string serializedB, vector<float> &b);
	void initWTensor(string serializedW, vector<vector<float>> &w, int dim1, int dim2);
	void maxPool2x2(Mat &image, Mat &outputImage);
	void convolve(int imageSize, vector<Mat> &images, vector<Mat> &outputImages, vector<vector<vector<vector<float>>>> &w, vector<float> &b);
	void matMul(Mat &input, Mat &output, vector<vector<float>> &w, vector<float> &b, bool rectify);
	void softMax(Mat &input, vector<float> &output);
	float softPlus(float num);

	void stitch(vector<Mat> &images);

public:
	ConvolutionalNeuralNetwork(string folderPath, string _programPath);
	~ConvolutionalNeuralNetwork();

	char classify(InputImage *image, float &confidence);
	char classifyWithRotation(InputImage *image, float &confidence);
	void testBatch(TrainingSet &data);
	void startCMDInput();

	bool isInitialized();
};

