#include "stdafx.h"
#include "Utils.h"

#include "Classifier.h"
#include "BarcodeReader.h"

Mat Utils::rotateImage(const Mat& image, int degrees) {
	// get rotation matrix for rotating the image around its center
	Point2f center(image.cols / 2.0, image.rows / 2.0);
	Mat rot = getRotationMatrix2D(center, degrees, 1.0);

	// determine bounding rectangle
	Rect bbox = RotatedRect(center, image.size(), degrees).boundingRect();

	// adjust transformation matrix
	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

	Mat output(image.rows, image.cols, image.depth());
	warpAffine(image, output, rot, bbox.size());

	return output;
}


void Utils::cropImage(Mat& image)
{
	// Convert colours
	threshold(image, image, 40.0, 255, CV_THRESH_BINARY);

	// Get bounding boxes
	int largestRow = 0, smallestRow = INT_MAX, largestCol = 0, smallestCol = INT_MAX;
	for (int i = 0; i < image.rows; ++i)
	{
		for (int j = 0; j < image.cols; ++j)
		{
			uchar test = image.at<uchar>(i, j);
			if (image.at<uchar>(i, j) > 0)
			{
				if (i < smallestRow)
					smallestRow = i;
				if (j < smallestCol)
					smallestCol = j;
				if (i > largestRow)
					largestRow = i;
				if (j > largestCol)
					largestCol = j;
			}
		}
	}
	Rect cropRect(smallestCol, smallestRow, largestCol - smallestCol + 1, largestRow - smallestRow + 1);

	// Modify rect for smallest possible square dimensions
	int diff;
	if (cropRect.width > cropRect.height)
	{
		diff = cropRect.width - cropRect.height;
		cropRect.y -= round((double)diff / 2);
		if (cropRect.y <= 0)
			cropRect.y = 0;
		cropRect.height += diff;
		if (cropRect.y + cropRect.height >= image.rows)
			cropRect.height = image.rows - cropRect.y - 1;
	}
	else
	{
		diff = cropRect.height - cropRect.width;
		cropRect.x -= round((double)diff / 2);
		if (cropRect.x <= 0)
			cropRect.x = 0;
		cropRect.width += diff;
		if (cropRect.x + cropRect.width >= image.cols)
			cropRect.width = image.cols - cropRect.x - 1;
	}

	// Crop image
	try
	{
		image = image(cropRect);
	}
	catch (cv::Exception &e)
	{
		cout << "hmmm something went wrong" << endl;
	}
}

bool Utils::allInOneClassify(const Mat& image, Classifier *classifier, string &description)
{
	assert(classifier != NULL);
	assert(!image.empty());

	// Test if barcode
	Mat grayscale(image.rows, image.cols, CV_8UC1);
	cv::cvtColor(image, grayscale, cv::COLOR_RGB2GRAY);
	BarcodeReader barcodeReader;
	string barcodeResult = barcodeReader.scanImage(grayscale);
	if (barcodeResult != "")
	{
		description = barcodeResult;
		return true;
	}

	Classifier::Results results;
	classifier->classify(image, results);

	cout << results.characterConfidence << endl;
	if (results.characterConfidence != 0)
	{
		description = results.description;
		return true;
	}

	// Did not classify as anything
	description = "Unclassified";
	return true;
}
