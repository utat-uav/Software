#include "stdafx.h"
#include "Classifier.h"

#include <chrono>
#include <ctime>

#include "Segmenter.h"
#include "Utils.h"
#include "Color.h"


Classifier::Classifier(const string &folderPath, const string &programPath)
	: folderPath(folderPath), programPath(programPath)
{
	session = NULL;
	knownColors = Color::getAUVSIColors();

	if (folderPath[folderPath.size() - 1] != '/' && folderPath[folderPath.size() - 1] != '\\')
	{
		this->folderPath.push_back('/');
	}
}

Classifier::~Classifier()
{
	if (session)
	{
		session->Close();
		delete session;
	}
}

bool Classifier::initialize()
{
	TF_CHECK_OK(NewSession(SessionOptions(), &session));

	GraphDef graph_def;
	TF_CHECK_OK(ReadBinaryProto(Env::Default(), folderPath + "frozen_model.pb", &graph_def));

	// Add the graph to the session
	TF_CHECK_OK(session->Create(graph_def));

	return true;
}

int Classifier::classify(const string &imagePath, Results &results)
{
	cv::Mat imageMat = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);
	return classify(imageMat, results);
}

/*
 * Result returns with confidence == 0 if no suitable character found
 */
int Classifier::classify(const Mat &imageMat, Results &results)
{
	// call segmenter first
	// segmentedImages[0] is shape, segmentedImages[1] is letter
	Segmenter segm;
	vector<cv::Mat> segmentedImages = segm.segment(imageMat);

	string savePath = programPath.substr(0, programPath.find_last_of('\\'));
	bool falsePositiveNoted = false;

	results.characterConfidence = 0;

	//for (unsigned i = 0; i < segmentedImages.size(); ++i)
	for (unsigned i = 1; i < segmentedImages.size(); ++i) // Second image is always the character
	{
		if (segmentedImages[i].empty())
		{
			if (!falsePositiveNoted)
			{
				falsePositiveNoted = true;
				cout << "Invalid image. Likely a false positive ..." << endl;
			}
			continue;
		}

		double confidence = 0;
		char c = '-';

		// Default return values
		results.character = '-';
		results.characterConfidence = 0;

		// Do classification
		try
		{
			// classify D:\Workspace\UAV\Test Flights\Flight3_output\im0027roi0.jpg
			classifyCharacter(segmentedImages[i], c, confidence);
		}
		catch (cv::Exception &e)
		{
			continue;
		}

		// Anything below 98% is uncertainty that we don't need
		if (confidence < params.confidenceThresh)
		{
			cout << "Invalid image. Confidence of " << confidence << " for a " << c << " is too low. Likely a false positive ..." << endl;
		}
		else
		{
			results.character = c;
			results.characterConfidence = confidence;

			std::stringstream ss;
			ss << "Classified as " << c << " with " << confidence << " confidence";
			results.description = ss.str();
			cout << results.description << endl;
		}
	}

	if (!falsePositiveNoted)
	{
		classifyColors(imageMat, segmentedImages, results.shapeColor, results.characterColor);
		cout << "Classified alphanumeric color as " << results.characterColor << endl;
		cout << "Classified shape color as " << results.shapeColor << endl;
	}

	return -1;
}


void Classifier::classifyColors(const Mat &image, vector<Mat> segmentedImages, string& shapeColorStr, string& letterColorStr)
{
	Mat mask = image.clone();

	// resize segmentedImages
	resize(segmentedImages[0], segmentedImages[0], Size(image.cols, image.rows), 1);
	resize(segmentedImages[1], segmentedImages[1], Size(image.cols, image.rows), 1);

	// take the average color of shape and letter
	// avgColor is in RGB 
	Vec3f avgColorShape(0.0, 0.0, 0.0), avgColorLetter(0.0, 0.0, 0.0);
	int shapePixels = 0, letterPixels = 0;
	for (int i = 0; i < image.rows; ++i)
		for (int j = 0; j < image.cols; ++j)
		{
			Vec3b bgrColor = image.at<Vec3b>(i, j);
			if (segmentedImages[1].at<unsigned char>(i, j) >= 255)
			{
				avgColorLetter[0] += bgrColor[2];
				avgColorLetter[1] += bgrColor[1];
				avgColorLetter[2] += bgrColor[0];
				++letterPixels;
			}
			else
			{
				mask.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
				if (segmentedImages[0].at<unsigned char>(i, j) >= 255)
				{
					avgColorShape[0] += bgrColor[2];
					avgColorShape[1] += bgrColor[1];
					avgColorShape[2] += bgrColor[0];
					++shapePixels;
				}
			}	
		}

	avgColorShape /= (shapePixels * 255.0);
	avgColorLetter /= (letterPixels * 255.0);
	Color shapeColor(avgColorShape), letterColor(avgColorLetter);
	Color closest;

	// for testing
	/*
	imshow("mask", mask);
	waitKey(0);
	cout << "saturation: " << shapeColor.hls[2] << " vs " << letterColor.hls[2] << endl;
	cout << "lightness: " << shapeColor.hls[1] << " vs " << letterColor.hls[1] << endl;
	cout << "color var: " << shapeColor.colorVariance() << " vs " << letterColor.colorVariance() << endl; 
	shapeColor.visualize(false);
	knownColors["brown"].visualize(false);
	knownColors["orange"].visualize(false);
	waitKey(0);
	*/

	classifyColorsHelper(shapeColor, closest);
	shapeColorStr = closest.name;

	classifyColorsHelper(letterColor, closest);
	letterColorStr = closest.name;
}


void Classifier::classifyColorsHelper(const Color& color, Color& _closest)
{
	Color closest;
	double minDist = numeric_limits<double>::max();

	// if it seems to be a grayscale color then we only compare lightness values
	if (color.colorVariance() < params.varThresh || color.hls[1] > params.lThresh || color.hls[1] < 1.0 - params.lThresh)
	{
		vector<Color> grayScale = { knownColors["black"], knownColors["gray"], knownColors["white"] };
		for (int i = 0; i < grayScale.size(); ++i)
		{
			double dist = pow(color.hls[1] - grayScale[i].hls[1], 2.0);
			if (dist < minDist)
			{
				minDist = dist;
				closest = grayScale[i];
			}
		}
		_closest = closest;
		return;
	}

	for (auto it = knownColors.begin(); it != knownColors.end(); ++it)
	{
		// don't compare with grayscale colors
		string name = it->first;
		if (name == "black" || name == "gray" || name == "white")
			continue;

		double dist = color.distanceFrom(it->second);
		if (dist < minDist)
		{
			minDist = dist;
			closest = it->second;
		}
	}

	// brown and orange are distinguished best by saturation values
	if (closest.name == "brown" || closest.name == "orange")
	{
		double satDistOrange = pow(color.hls[2] - knownColors["orange"].hls[2], 2.0);
		double satDistBrown = pow(color.hls[2] - knownColors["brown"].hls[2], 2.0);
		if (satDistOrange < satDistBrown)
			closest = knownColors["orange"];
		else
			closest = knownColors["brown"];
	}

	_closest = closest;
}


void Classifier::classifyCharacter(const Mat &image, char &c, double &confidence)
{
	confidence = 0;
	vector<Mat> images;
	for (int rotIdx = 0; rotIdx < params.numRots; ++rotIdx)
	{
		double angle = (rotIdx - (int)(params.numRots / 2)) * 20;
		Mat rotated = Utils::rotateImage(image, angle);
		processImage(rotated);
		images.push_back(rotated);
	}

	vector<char> chars;
	vector<double> confidences;
	classifyCharacterHelper(images, chars, confidences);

	// Get the best confidence
	confidence = confidences[0];
	c = chars[0];
	for (int i = 1; i < confidences.size(); ++i)
	{
		if (confidences[i] >= confidence)
		{
			confidence = confidences[i];
			c = chars[i];
		}
	}
}

/*
 * Requires a 40x40 black and white binary image
 * This function can take in multiple inputs (as a batch) and outputs multiple results
 */
void Classifier::classifyCharacterHelper(const vector<Mat> &images, // Inputs
										 vector<char> &chars, vector<double> &confidences) // Outputs
{
	assert(images.size() > 0);
	assert(chars.size() == 0);
	assert(confidences.size() == 0);

	// Timing parameters
	std::chrono::time_point<std::chrono::system_clock> start, end;

	// Init input tensor dimensions
	Tensor inputTensor(tensorflow::DT_FLOAT,
					   TensorShape({ (long long)images.size(), images[0].rows * images[0].cols }));
	auto inputTensorMapped = inputTensor.tensor<float, 2>();

	// Copy images to tensor
	for (int imgIdx = 0; imgIdx < images.size(); ++imgIdx)
	{
		int j = 0;
		for (int r = 0; r < images[imgIdx].rows; ++r)
		{
			for (int c = 0; c < images[imgIdx].cols; ++c, ++j)
			{
				float value = (float)images[imgIdx].at<unsigned char>(r, c);
				inputTensorMapped(imgIdx, j) = value;
			}
		}
	}

	// Set keep_prob to 1
	Tensor keepProb(tensorflow::DT_FLOAT, TensorShape());
	keepProb.scalar<float>()() = 1.0;

	std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
		{ "input", inputTensor },
		{ "keep_prob", keepProb }
	};

	std::vector<Tensor> outputs;
	
	// Get the output
	start = std::chrono::system_clock::now();
	TF_CHECK_OK(session->Run(inputs, { "out_node" }, {}, &outputs));
	end = std::chrono::system_clock::now();

	// Calculate time
	std::chrono::duration<double> elapsed_seconds = end - start;
	//std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds" << std::endl;

	assert(outputs.size() == 1);
	auto outputMapped = outputs[0].tensor<float, 2>();

	assert(outputs[0].shape().dim_sizes().at(0) == images.size());
	int numClasses = outputs[0].shape().dim_sizes().at(1);

	// Loop through the results and get the highest one
	for (int imgIdx = 0; imgIdx < images.size(); ++imgIdx)
	{
		// Get the max and confidence of the current image
		int id = 0;
		double max = outputMapped(imgIdx, 0);
		double denominator = 0;
		double numerator = 0;
		for (int idx = 0; idx < numClasses; ++idx)
		{
			double value = outputMapped(imgIdx, idx);

			// Compute the exponent
			double exponent = std::exp(value);
			if (std::isnan(exponent))
			{
				exponent = 10000000;
			}

			if (value >= max)
			{
				numerator = exponent;
				max = value;
				id = idx;
			}

			denominator += exponent;
		}

		char c = getCharFromIdx(id);
		double confidence = numerator / denominator;
		chars.push_back(c);
		confidences.push_back(confidence);

		//std::cout << "DEBUG: " << c << " | " << confidence << std::endl;
	}
}

/*
 * Resizes it appropriately and makes it binary
 */
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

	cv::threshold(image, image, 128, 1, cv::THRESH_BINARY);
}

char Classifier::getCharFromIdx(int idx)
{
	char c = '-';
	if (idx <= 9)
	{
		c = '0' + idx;
	}
	else
	{
		idx -= 10;
		c = 'A' + idx;
	}

	return c;
}
