#include "stdafx.h"
#include "InputImage.h"
#include "TrainingSet.h"
#include "NeuralNetwork.h"
#include "FileManager.h"
#include "ConvolutionalNeuralNetwork.h"
#include "BarcodeReader.h"
#include "Identifier.h"

//--------------------------------------------------------------------
// PROTOTYPES
//--------------------------------------------------------------------

void credits();
void runCNN(string path, string programPath);
void runZBar(string path);


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

		string results;
		Identifier identifier(fileName, gpsLog, outputFolder, &results);
		identifier.analyze();
	}
	else if (mode == "-segment")
	{
		if (argc < 3)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		string fileName = argv[2];

		Segmenter segm(fileName);
		vector<cv::Mat> segmentedImages = segm.segment();

		int i = 0;
		for (vector<cv::Mat>::iterator it = segmentedImages.begin(); it != segmentedImages.end(); ++it)
		{
			++i;
			std::ostringstream stringStream;
			stringStream << "Image " << i;
			std::string copyOfStr = stringStream.str();
			cv::imshow(copyOfStr, (*it)*255);
		}
		if (i > 0)
		{
			cv::waitKey(0);
		}
	}
	else if (mode == "-help")
	{
		cout << "Press enter to continue..." << endl;
		cin.ignore();
	}

	//InputImage img("D:\\Workspace\\UAV\\classification-training\\Testingset\\test.png", true);

	//int test = InputImage::charToOneHotIndex('Z');
	//char test2 = InputImage::oneHotIndexToChar(test);

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
	cout << "UTAT-UAV COMPUTER VISION CLASSIFICATION V2.1" << endl;
	cout << endl;
	cout << "Author(s): Davis Wu, Tianxing Li" << endl;
	cout << "Creation Date: Dec 30, 2015" << endl;
	cout << "Latest Modification Date: April, 2017" << endl;
	cout << "Info: attempts to classify characters" << endl;
	cout << endl;
	cout << "Command line info: [-cnn <serializedNN>] to run pre-trained program" << endl;
	cout << "                   [-zbar <imagepath>] to read barcode" << endl;
	cout << "                   [-segment <imagepath>] to run the segmenter" << endl;
	cout << "                   [-identify <imagepath> <gpsLog> <outputFolder>] to identify and crop" << endl;
	cout << "                   [-help me] for help" << endl;
	cout << endl;
}

/*
* Does barcode scan
*/
void runZBar(string path)
{
	BarcodeReader barcodeReader;
	barcodeReader.scanImage(path);
}

/*
* Classifies with CNN
*/
void runCNN(string path, string programPath)
{
	cout << "Initializing convolutional neural network..." << endl;
	cout << "Program path is " << programPath << " and the cnn path is " << path << endl;
	ConvolutionalNeuralNetwork cnn(path, programPath);

	if (!cnn.isInitialized())
	{
		cout << "Initialization failed. Exiting program..." << endl;
		return;
	}

	// Commence command inputs
	cnn.startCMDInput();
}
