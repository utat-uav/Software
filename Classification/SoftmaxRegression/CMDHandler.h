#pragma once
#include "FileManager.h"
#include "BarcodeReader.h"

class Classifier;

class CMDHandler
{
public:
	CMDHandler(const string &folderPath, const string &programPath);
	~CMDHandler();

	void startCMDInput();
	void trimString(std::string &str);

	bool isInitialized();

private:
	bool initialized;

	string programPath;
	Classifier *classifier;

	void stitch(vector<Mat> &images);
};

