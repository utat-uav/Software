#include "stdafx.h"
#include "CMDHandler.h"
#include "Classifier.h"
#include "Segmenter.h"


CMDHandler::CMDHandler(const string &folderPath, const string &programPath)
	: programPath(programPath)
{
	classifier = new Classifier(folderPath, programPath);
	initialized = classifier->initialize();
}

CMDHandler::~CMDHandler()
{
	delete classifier;
}

void CMDHandler::stitch(vector<Mat> &images)
{
	Mat masterImage(images[0].rows, images[0].cols, CV_32FC1);

	images[0].copyTo(masterImage);

	for (int i = 1; i < images.size(); ++i)
	{
		hconcat(masterImage, images[i], masterImage);
	}

	imshow("showing images", masterImage);
	waitKey(0);
}

void CMDHandler::startCMDInput()
{
	cout << endl;
	cout << "// CNN COMMAND-LINE MODE INITIATED" << endl << endl;
	while (!cin.eof())
	{
		cout << ">> ";

		string line;
		getline(cin, line);
		stringstream ss(line);

		string command;
		ss >> command;

		if (ss.fail())
		{
			cout << "No command entered" << endl;
			continue;
		}

		if (command == "testBatch")
		{
			string batchPath;
			std::getline(ss, batchPath);
			trimString(batchPath);

			if (ss.fail())
			{
				cout << "Insufficient or invalid arguments. Type 'help' for more information" << endl;
				continue;
			}

			try
			{
				cout << "Not implemented yet" << endl;
			}
			catch(cv::Exception& e)
			{
				cout << "Something went wrong" << endl;
			}

			cout << endl;
		}
		else if (command == "classify")
		{
			string imagePath;
			std::getline(ss, imagePath);
			trimString(imagePath);

			if (ss.fail())
			{
				cout << "Insufficient or invalid arguments. Type 'help' for more information" << endl;
				continue;
			}

			// Check file validity
			ifstream ifile(imagePath);
			if (!ifile) {
				cout << imagePath << " is not a valid file path" << endl;
				continue;
			}
			ifile.close();

			try
			{
				classifier->classify(imagePath);
			}
			catch (cv::Exception& e)
			{
				cout << "Something went wrong" << endl;
			}

			cout << endl;
		}
		else if (command == "zbar")
		{
			string imagePath;
			std::getline(ss, imagePath);
			trimString(imagePath);

			if (ss.fail())
			{
				cout << "Insufficient or invalid arguments. Type 'help' for more information" << endl;
				continue;
			}

			cout << imagePath << endl;

			BarcodeReader barcodeReader;
			barcodeReader.scanImage(imagePath);
		}
		else if (command == "help")
		{
			cout << "Commands are: " << endl;
			cout << "      testBatch <folderPath>" << endl;
			cout << "      classify <imagePath>" << endl;
			cout << "      zbar <imagePath>" << endl;
			cout << endl;
		}
		else
		{
			cout << "Command is invalid. Type 'help' for more information" << endl;
		}
	}
}

bool CMDHandler::isInitialized()
{
	return this->initialized;
}

void CMDHandler::trimString(std::string &str)
{
	if (str.size() == 0) return;

	while (str[0] == ' ')
	{
		str.erase(0, 1);
	}

	while (str[str.size() - 1] == ' ')
	{
		str.pop_back();
	}
}