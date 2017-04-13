#include "stdafx.h"
#include "CMDHandler.h"


CMDHandler::CMDHandler(string folderPath, string _programPath)
{
}

CMDHandler::~CMDHandler()
{
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

			// call segmenter first
			// segmentedImages[0] is shape, segmentedImages[1] is letter
			Segmenter segm(imagePath);
			vector<cv::Mat> segmentedImages = segm.segment();

			string savePath = programPath.substr(0, programPath.find_last_of('\\'));
			char finalChar = '0';
			float finalConfidence = 0;
			bool falsePositiveNoted = false;
			for (unsigned i = 0; i < segmentedImages.size(); ++i)
			{
				if (segmentedImages[i].empty())
				{
					if (!falsePositiveNoted)
					{
						falsePositiveNoted = true;
						cout << "Likely a false positive ..." << endl;
					}
					continue;
				}
					
				cv::imwrite(savePath + "\\label.jpg", segmentedImages[i]);
				float confidence = 0;
				char c = '-';
				try 
				{
					
				}
				catch (cv::Exception &e)
				{
					continue;
				}

				if (confidence >= finalConfidence)
				{
					finalConfidence = confidence;
					finalChar = c;
				}
			}

			cout << "Classified as " << finalChar << " with " << finalConfidence << " confidence" << endl;
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