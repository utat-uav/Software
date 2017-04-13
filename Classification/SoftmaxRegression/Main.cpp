#include "stdafx.h"
#include "Utils.h"
#include "FileManager.h"
#include "CMDHandler.h"
#include "BarcodeReader.h"
#include "Identifier.h"
#include "Segmenter.h"

//--------------------------------------------------------------------
// PROTOTYPES
//--------------------------------------------------------------------

void credits();
void runCNN(const string &path, const string &programPath);
void runZBar(const string &path);
void runIdentifier(const string &fileName, const string &gpsLog, const string &outputFolder);
void runAioIdentifier(const string &fileName, const string &gpsLog, const string &outputFolder, const string &programPath, const string &cnnPath);
void runSegmenter(const string &fileName);


//--------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------

int main(int argc, char** argv)
{
	// Test identifier with:
	// -identify "D:\Workspace\UAV\AUVSI2015Pictures\test1.jpg" "D:\Workspace\UAV\AUVSI2015Pictures\uav_gps.log" "D:\Workspace\UAV\AUVSI2015Pictures\Test Output"

	// Test cnn with:
	// -cnn "D:\Workspace\UAV\Software\Classification\Release\TrainedCNN"

	credits();

	if (argc < 2)
	{
		cout << "Insufficient arguments. Exiting program..." << endl;
		return -1;
	}

	string mode = argv[1];

	if (mode == "-cnn")
	{
		if (argc < 3)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// COMMENCE CONVOLUTIONAL NEURAL NETWORK" << endl << endl;
		string path = argv[2];
		string programPath = argv[0];

		runCNN(path, programPath);
	}
	else if (mode == "-zbar")
	{
		if (argc < 3)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// STARTING ZBAR..." << endl << endl;
		string path = argv[2];

		runZBar(path);
	}
	else if (mode == "-aio")
	{
		if (argc < 6)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// STARTING ALL IN ONE IDENTIFIER..." << endl << endl;
		string programPath = argv[0];
		string fileName = argv[2];
		string gpsLog = argv[3];
		string outputFolder = argv[4];
		string cnnPath = argv[5];

		runAioIdentifier(fileName, gpsLog, outputFolder, programPath, cnnPath);
	}
	else if (mode == "-identify")
	{
		if (argc < 5)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// STARTING IDENTIFIER..." << endl << endl;
		string fileName = argv[2];
		string gpsLog = argv[3];
		string outputFolder = argv[4];

		runIdentifier(fileName, gpsLog, outputFolder);
	}
	else if (mode == "-segment")
	{
		if (argc < 3)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		string fileName = argv[2];

		cout << "// STARTING SEGMENTER..." << endl << endl;
		runSegmenter(fileName);
	}
	else if (mode == "-help")
	{
		cout << "Press enter to continue..." << endl;
		cin.ignore();
	}

	return 0;
}


//--------------------------------------------------------------------
// FUNCTIONS
//--------------------------------------------------------------------

/*
* Program initialization
*/
void credits()
{
	cout << "UTAT-UAV COMPUTER VISION CLASSIFICATION V3.0" << endl;
	cout << endl;
	cout << "Author(s): Davis Wu, Tianxing Li" << endl;
	cout << "Creation Date: Dec 30, 2015" << endl;
	cout << "Latest Modification Date: April, 2017" << endl;
	cout << "Info: attempts to classify characters" << endl;
	cout << endl;
	cout << "Command line info: [-cnn <cnnPath>] to run pre-trained program" << endl;
	cout << "                   [-zbar <imagepath>] to read barcode" << endl;
	cout << "                   [-segment <imagepath>] to run the segmenter" << endl;
	cout << "                   [-identify <imagepath> <gpsLog> <outputFolder>] to identify and crop" << endl;
	cout << "                   [-aio <imagepath> <gpsLog> <outputFolder> <cnnPath>] to identify and crop" << endl;
	cout << "                   [-help me] for help" << endl;
	cout << endl;
}

void runZBar(const string &path)
{
	BarcodeReader barcodeReader;
	barcodeReader.scanImage(path);
}

void runCNN(const string &path, const string &programPath)
{
	cout << "Initializing convolutional neural network..." << endl;
	cout << "Program path is " << programPath << " and the cnn path is " << path << endl;
	CMDHandler cnn(path, programPath);

	if (!cnn.isInitialized())
	{
		cout << "Initialization failed. Exiting program..." << endl;
		return;
	}

	// Commence command inputs
	cnn.startCMDInput();
}

void runIdentifier(const string &fileName, const string &gpsLog, const string &outputFolder)
{
	string results;
	Identifier identifier(fileName, gpsLog, outputFolder, &results);
	identifier.analyze();
}

void runAioIdentifier(const string &fileName, const string &gpsLog, const string &outputFolder, 
	const string &programPath, const string &cnnPath)
{
	string results;
	Identifier identifier(fileName, gpsLog, outputFolder, &results, 188.0, programPath, cnnPath);
	identifier.analyze();
}

void runSegmenter(const string &fileName)
{
	Segmenter segm(fileName);
	vector<cv::Mat> segmentedImages = segm.segment();

	int i = 0;
	for (vector<cv::Mat>::iterator it = segmentedImages.begin(); it != segmentedImages.end(); ++it)
	{
		++i;
		std::ostringstream stringStream;
		stringStream << "Image " << i;
		std::string copyOfStr = stringStream.str();
		cv::imshow(copyOfStr, (*it) * 255);
	}
	if (i > 0)
	{
		cv::waitKey(0);
	}
}
