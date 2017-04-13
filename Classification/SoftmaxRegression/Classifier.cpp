#include "stdafx.h"
#include "Classifier.h"

#include "Segmenter.h"
#include "Utils.h"

Classifier::Classifier(const string &folderPath, const string &programPath)
	: folderPath(folderPath), programPath(programPath)
{
	session = NULL;

	if (folderPath[folderPath.size() - 1] != '/' && folderPath[folderPath.size() - 1] != '\\')
	{
		this->folderPath.push_back('/');
	}
}

Classifier::~Classifier()
{
	session->Close();
	delete session;
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
	// call segmenter first
	// segmentedImages[0] is shape, segmentedImages[1] is letter
	Segmenter segm(imagePath);
	vector<cv::Mat> segmentedImages = segm.segment();

	string savePath = programPath.substr(0, programPath.find_last_of('\\'));
	bool falsePositiveNoted = false;

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

		cv::imwrite(savePath + "\\label.jpg", segmentedImages[i]);
		double confidence = 0;
		char c = '-';

		// Default return values
		results.character = '-';
		results.characterConfidence = 0;

		// Do classification
		try
		{
			// classify D:\Workspace\UAV\Test Flights\Flight3_output\im0027roi0.jpg
			cv::Mat image = cv::imread(savePath + "\\label.jpg", cv::IMREAD_GRAYSCALE);
			classifyCharacter(image, c, confidence);
		}
		catch (cv::Exception &e)
		{
			continue;
		}

		if (confidence < 0.8)
		{
			cout << "Invalid image. Confidence of " << confidence << " for a " << c << " is too low. Likely a false positive ..." << endl;
		}
		else
		{
			results.character = c;
			results.characterConfidence = confidence;

			cout << "Classified as " << c << " with " << confidence << " confidence" << endl;
		}
	}

	return -1;
}

void Classifier::classifyCharacter(const Mat &image, char &c, double &confidence)
{
	int numRots = 3;
	confidence = 0;
	for (int rotIdx = 0; rotIdx < numRots; ++rotIdx)
	{
		double angle = (rotIdx - (int)(numRots / 2)) * 5;
		Mat rotated = Utils::rotateImage(image, angle);
		processImage(rotated);

		char cTemp = '-';
		double confidenceTemp = 0;
		classifyCharacterHelper(rotated, cTemp, confidenceTemp);

		if (confidenceTemp >= confidence)
		{
			confidence = confidenceTemp;
			c = cTemp;
		}
	}
}

/*
 * Requires a 40x40 black and white binary image
 */
void Classifier::classifyCharacterHelper(const Mat &image, char &c, double &confidence)
{
	// Convert the image to tensor
	Tensor inputTensor(tensorflow::DT_FLOAT, TensorShape({ 1, image.rows * image.cols }));
	auto inputTensorMapped = inputTensor.tensor<float, 2>();
	int j = 0;
	for (int r = 0; r < image.rows; ++r)
	{
		for (int c = 0; c < image.cols; ++c, ++j)
		{
			float value = (float)image.at<unsigned char>(r, c);
			inputTensorMapped(0, j) = value;
		}
	}

	// Set keep prob to 1
	Tensor keepProb(tensorflow::DT_FLOAT, TensorShape());
	keepProb.scalar<float>()() = 1.0;

	std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
		{ "input", inputTensor },
		{ "keep_prob", keepProb }
	};

	// Get the output
	std::vector<Tensor> outputs;
	TF_CHECK_OK(session->Run(inputs, { "out_node" }, {}, &outputs));

	assert(outputs.size() == 1);
	auto outputMapped = outputs[0].tensor<float, 2>();

	int numClasses = outputs[0].shape().dim_sizes().at(1);

	// Get the max and confidence
	int id = 0;
	double max = outputMapped(0, 0);
	double denominator = 0;
	double numerator = 0;
	for (int idx = 0; idx < numClasses; ++idx)
	{
		double value = outputMapped(0, idx);
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

	confidence = numerator / denominator;
	c = getCharFromIdx(id);
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
