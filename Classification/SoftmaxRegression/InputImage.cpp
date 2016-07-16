#include "stdafx.h"
#include "InputImage.h"

InputImage::~InputImage()
{
	delete[] imageArray;
}

void InputImage::rotateImage(Mat& image, int degrees) {
	// get rotation matrix for rotating the image around its center
	Point2f center(image.cols / 2.0, image.rows / 2.0);
	Mat rot = getRotationMatrix2D(center, degrees, 1.0);

	// determine bounding rectangle
	Rect bbox = RotatedRect(center, image.size(), degrees).boundingRect();

	// adjust transformation matrix
	rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

	warpAffine(image, image, rot, bbox.size());
}

InputImage::InputImage(Mat *directMat, char correctLabel, bool testImage, bool appendOne, bool noMemoryConservation)
{
	imageArray = NULL;
	directMat->copyTo(image);

	if (noMemoryConservation)
		image.copyTo(originalImage);

	if (testImage)
	{
		cropImage();
		//imshow("Cropped", image);
		//waitKey(0);
	}

	// Check data validity
	if (image.empty())
	{
		valid = false;
		return;
	}

	if (image.rows != 28 || image.cols != 28)
	{
		resize(image, image, Size(28, 28));
	}

	// Make binary (1 channel with a 0 or 255 value)
	threshold(image, image, 128.0, 255.0, THRESH_BINARY);

	// Reshape image into 400 rows and 1 column
	image = image.reshape(0, DATA_SIZE);

	valid = true;

	// Get label character
	charLabel = correctLabel;

	// Get label
	int labelArray[LABEL_SIZE] = { 0 };
	labelIndex = charToOneHotIndex(correctLabel);
	labelArray[labelIndex] = 1;

	// Push label (one-hot encoded vector)
	label = vector<int>(labelArray, labelArray + sizeof(labelArray) / sizeof(int));

	// Concatenate a 1 to the end of the matrix
	if (appendOne)
	{
		Mat one = Mat::ones(Size(1, 1), CV_8UC1);
		vconcat(image, one, image);
	}

	// Convert to floats
	image.convertTo(image, CV_32FC1);

	if (noMemoryConservation)
		imageArray = new float*[image.rows];
	for (int i = 0; i < image.rows; i++)
	{
		image.at<float>(i, 0) = image.at<float>(i, 0) >= 1 ? 1.0 : 0.0; // Used to be 0.5 : -0.5;
		if (noMemoryConservation)
			imageArray[i] = &(image.at<float>(i, 0));
	}
}

InputImage::InputImage(string path, bool testImage, bool appendOne, bool noMemoryConservation)
{
	imageArray = NULL;

	// Load image
	image = imread(path, CV_LOAD_IMAGE_GRAYSCALE);

	if (noMemoryConservation)
		image.copyTo(originalImage);

	if (testImage)
	{
		cropImage();
		//imshow("Cropped", image);
		//waitKey(0);
	}

	// Check data validity
	if (image.empty())
	{
		valid = false;
		return;
	}

	if (image.rows != 28 || image.cols != 28)
	{
		resize(image, image, Size(28, 28));
	}

	// Make binary (1 channel with a 0 or 255 value)
	threshold(image, image, 128.0, 255.0, THRESH_BINARY);

	// Reshape image into 400 rows and 1 column
	image = image.reshape(0, DATA_SIZE);

	valid = true;

	// Get label character
	int charIndex = path.find_last_of('\\');
	char character = path.at(charIndex + 1);
	charLabel = character;

	// Get label
	labelIndex = charToOneHotIndex(character);
	// Push label (one-hot encoded vector)
	label = vector<int>(LABEL_SIZE, 0);
	label[labelIndex] = 1;

	// Concatenate a 1 to the end of the matrix
	if (appendOne)
	{
		Mat one = Mat::ones(Size(1, 1), CV_8UC1);
		vconcat(image, one, image);
	}

	// Convert to floats
	image.convertTo(image, CV_32FC1);

	if (noMemoryConservation)
		imageArray = new float*[image.rows];
	for (int i = 0; i < image.rows; i++)
	{
		image.at<float>(i, 0) = image.at<float>(i, 0) >= 1 ? 1.0 : 0.0; // Used to be 0.5 : -0.5;
		if (noMemoryConservation)
			imageArray[i] = &(image.at<float>(i, 0));
	}
}

void InputImage::cropImage()
{
	// Convert colours
	threshold(image, image, 40.0, 255, CV_THRESH_BINARY);

	// Get bounding boxes
	int largestRow = 0, smallestRow = INFINITY*2, largestCol = 0, smallestCol = INFINITY*2;
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

	// Resize
	resize(image, image, Size(28, 28));
}

int InputImage::getLabelIndex()
{
	return this->labelIndex;
}

char InputImage::getCharLabel()
{
	return this->charLabel;
}

bool InputImage::isValid()
{
	return this->valid;
}

// Returns a pointer to the matrix
Mat* InputImage::getImage()
{
	return &(this->image);
}

vector<int>* InputImage::getLabelVector()
{
	return &(this->label);
}

// Returns a vector version of the image
vector<float> InputImage::getVectorizedImage()
{
	vector<float> vectorized;

	for (int i = 0; i < image.rows; i++)
	{
		vectorized.push_back(image.at<float>(i, 0));
	}

	return vectorized;
}

int InputImage::charToOneHotIndex(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else if (c >= 'a' && c <= 'z')
	{
		return c - 'a' + 10;
	}
	else if (c >= 'A' && c <= 'Z')
	{
		return c - 'A' + 36;
	}
}

Mat* InputImage::getOriginalimage()
{
	return &(this->originalImage);
}

int InputImage::oneHotIndexToChar(int i)
{
	// Check cases
	if (i >= 0 && i <= 9)
	{
		// is a number
		return '0' + i;
	}
	else if (i >= 10 && i <= 35)
	{
		// is a lowercase letter
		return 'a' + i - 10;
	}
	else if (i >= 36 && i <= 61)
	{
		// is a uppercase letter
		return 'A' + i - 36;
	}
}