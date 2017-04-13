#pragma once
#include "FileManager.h"
#include "BarcodeReader.h"
#include "Segmenter.h"

class CMDHandler
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

	void stitch(vector<Mat> &images);

public:
	CMDHandler(string folderPath, string _programPath);
	~CMDHandler();

	void startCMDInput();
	void trimString(std::string &str);

	bool isInitialized();
};

