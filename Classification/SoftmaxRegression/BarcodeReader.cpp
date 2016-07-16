#include "stdafx.h"
#include "BarcodeReader.h"


BarcodeReader::BarcodeReader()
{
}


BarcodeReader::~BarcodeReader()
{
}

void BarcodeReader::scanImage(string imagePath)
{
	// Create a reader
	ImageScanner scanner;

	// Configure the reader
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	// Get image
	Mat image = imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
	//cvtColor(image, image, CV_BGR2GRAY); // Crashes if not BGR
	int width = image.cols;
	int height = image.rows;
	uchar *raw = (uchar *) image.data;

	// Wrap image
	zbar::Image zImage(width, height, "Y800", raw, width*height);

	int output = scanner.scan(zImage);

	// Extract results
	for (Image::SymbolIterator symbol = zImage.symbol_begin(); symbol != zImage.symbol_end(); ++symbol) {
		// do something useful with results
		cout << symbol->get_type_name()
			<< " result \"" << symbol->get_data() << '"' << endl;
	}

	// Clean up
	zImage.set_data(NULL, 0);
}