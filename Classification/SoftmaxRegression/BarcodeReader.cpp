#include "stdafx.h"
#include "BarcodeReader.h"


BarcodeReader::BarcodeReader()
{
}


BarcodeReader::~BarcodeReader()
{
}

string BarcodeReader::scanImage(const string &imagePath)
{
	Mat image = imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
	return scanImage(image);
}

/*
 * Requires grayscale image
 */
string BarcodeReader::scanImage(const Mat &image)
{
	// Create a reader
	ImageScanner scanner;

	// Configure the reader
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	//cvtColor(image, image, CV_BGR2GRAY); // Crashes if not BGR
	int width = image.cols;
	int height = image.rows;
	uchar *raw = (uchar *)image.data;

	// Wrap image
	zbar::Image zImage(width, height, "Y800", raw, width*height);

	int output = scanner.scan(zImage);

	// Extract results
	int numResults = 0;
	string resultstr = "";
	for (Image::SymbolIterator symbol = zImage.symbol_begin(); symbol != zImage.symbol_end(); ++symbol)
	{
		// do something useful with results
		stringstream ss;
		ss << symbol->get_type_name()
			<< " result \"" << symbol->get_data() << '\"';
		resultstr = ss.str();
		cout << resultstr << endl;
		++numResults;
	}

	if (numResults == 0)
	{
		cout << "Invalid barcode" << endl;
	}

	// Clean up
	zImage.set_data(NULL, 0);

	return resultstr;
}
