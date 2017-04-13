#include "stdafx.h"
#include "Classifier.h"

#include "Segmenter.h"
#include "Utils.h"

Classifier::Classifier(const string &folderPath, const string &programPath)
	: folderPath(folderPath), programPath(programPath)
{
}

Classifier::~Classifier()
{
}

bool Classifier::initialize()
{
	return true;
}

int Classifier::classify(const string &imagePath)
{
	// call segmenter first
	// segmentedImages[0] is shape, segmentedImages[1] is letter
	Segmenter segm(imagePath);
	vector<cv::Mat> segmentedImages = segm.segment();

	string savePath = programPath.substr(0, programPath.find_last_of('\\'));
	char finalChar = '-';
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
			// classify D:\Workspace\UAV\Test Flights\Flight3_output\im0024roi0.jpg
			cv::Mat image = cv::imread(savePath + "\\label.jpg", cv::IMREAD_GRAYSCALE);
			processImage(image);


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

	return -1;
}

void Classifier::processImage(Mat &image)
{
	Utils::cropImage(image);

	int borderWidth = max(image.rows, image.cols) * 0.2;

	Mat buf(image.rows + borderWidth * 2, image.cols + borderWidth * 2, image.depth());

	copyMakeBorder(image, buf, borderWidth, borderWidth,
		borderWidth, borderWidth, BORDER_CONSTANT);

	image.release();
	image = buf;

	cv::resize(image, image, cv::Size(40, 40));

	// Debugging
	//cv::imshow("test", image);
	//cv::waitKey(0);
	//cv::destroyWindow("test");
}
