#include "stdafx.h"
#include "ConvolutionalNeuralNetwork.h"


ConvolutionalNeuralNetwork::ConvolutionalNeuralNetwork(string folderPath, string _programPath)
{
	FileManager fileManager(folderPath + "\\");

	programPath = _programPath;

	vector<string>* files = fileManager.getFiles();

	initialized = true;

	// Gets required files and initializes corresponding layers
	int requiredFileCount = 0;
	for (vector<string>::iterator file = files->begin(); file != files->end(); ++file)
	{
		if (file->find("Wconv1.txt") != string::npos)
		{
			cout << "Reading Wconv1.txt..." << endl;
			requiredFileCount++;
			initConvTensor(*file, Wconv1, 5, 5, 1, 32);
		}
		else if (file->find("bconv1.txt") != string::npos)
		{
			cout << "Reading bconv1.txt..." << endl;
			requiredFileCount++;
			initBTensor(*file, bconv1);
		}
		else if (file->find("Wconv2.txt") != string::npos)
		{
			cout << "Reading Wconv2.txt..." << endl;
			requiredFileCount++;
			initConvTensor(*file, Wconv2, 5, 5, 32, 64);
		}
		else if (file->find("bconv2.txt") != string::npos)
		{
			cout << "Reading bconv2.txt..." << endl;
			requiredFileCount++;
			initBTensor(*file, bconv2);
		}
		else if (file->find("Wfc1.txt") != string::npos)
		{
			
			cout << "Reading Wfc1.txt..." << endl;
			requiredFileCount++;
			initWTensor(*file, Wfc1, 7 * 7 * 64, 1024);
		}
		else if (file->find("bfc1.txt") != string::npos)
		{
			cout << "Reading bfc1.txt..." << endl;
			requiredFileCount++;
			initBTensor(*file, bfc1);
		}
		else if (file->find("Wfc2.txt") != string::npos)
		{
			cout << "Reading Wfc2.txt..." << endl;
			requiredFileCount++;
			initWTensor(*file, Wfc2, 1024, 62);
		}
		else if (file->find("bfc2.txt") != string::npos)
		{
			cout << "Reading bfc2.txt..." << endl;
			requiredFileCount++;
			initBTensor(*file, bfc2);
		}
	}

	// Checks number of files present
	if (requiredFileCount != 8)
	{
		cout << "Required files not present" << endl;
		initialized = false;
		return;
	}
	else
	{
		cout << "Files read and imported" << endl;
	}
}

ConvolutionalNeuralNetwork::~ConvolutionalNeuralNetwork()
{
}

void ConvolutionalNeuralNetwork::initWTensor(string serializedW, vector<vector<float>> &w, int dim1, int dim2)
{
	ifstream file;
	file.open(serializedW);

	w = vector<vector<float>>(dim1);
	for (int i = 0; i < dim1; ++i)
	{
		w[i] = vector<float>(dim2);
	}

	int iCount = 0;
	string line;
	while (!file.eof())
	{
		getline(file, line);

		if (!file.eof())
		{
			stringstream ss(line);
			int jCount = 0;
			while (!ss.eof())
			{
				float parameter;
				ss >> parameter;
				if (!ss.fail())
					w[iCount][jCount] = parameter;
				jCount++;
			}
		}

		iCount++;
	}

	file.close();
}

void ConvolutionalNeuralNetwork::initBTensor(string serializedB, vector<float> &b)
{
	// Open file
	ifstream file;
	file.open(serializedB);

	while (!file.eof())
	{
		float parameter;
		file >> parameter;

		if (!file.fail())
			b.push_back(parameter);
	}

	file.close();
}

void ConvolutionalNeuralNetwork::initConvTensor(string serializedConv, vector<vector<vector<vector<float>>>> &convLayer, int dim1, int dim2, int dim3, int dim4)
{
	// Open file
	ifstream file;
	file.open(serializedConv);

	// Initialize layer to specified dimensions
	convLayer = vector<vector<vector<vector<float>>>>(dim1);
	for (int i = 0; i < convLayer.size(); ++i)
	{
		convLayer[i] = vector<vector<vector<float>>>(dim2);
		for (int j = 0; j < convLayer[i].size(); ++j)
		{
			convLayer[i][j] = vector<vector<float>>(dim3);
			for (int k = 0; k < convLayer[i][j].size(); ++k)
			{
				convLayer[i][j][k] = vector<float>(dim4);
			}
		}
	}

	// Get values
	int ijCount = 0;
	while (!file.eof())
	{
		string line;
		getline(file, line);

		int kCount = 0;
		while (line.find("#") == string::npos && !file.eof())
		{
			// Get parameters
			stringstream ss(line);
			int zCount = 0;
			while (!ss.eof())
			{
				// Parse into float
				float parameter;
				ss >> parameter;

				// Enter it into the data structure
				if (!ss.fail())
					convLayer[ijCount / dim2][ijCount % dim2][kCount][zCount] = parameter;

				zCount++;
			}

			// Gets new line
			getline(file, line);

			kCount++;
		}

		ijCount++;
	}

	file.close();
}

char ConvolutionalNeuralNetwork::classify(InputImage *image, float &confidence)
{
	Mat *imageMat = image->getImage();
	vector<Mat> imageCopy(1);
	imageMat->copyTo(imageCopy[0]);

	// Removes redundant 1
	imageCopy[0].pop_back();

	// Reshape to 28x28
	imageCopy[0] = imageCopy[0].reshape(0, 28);

	// Convolve first layer
	vector<Mat> convolutions1(32);
	for (int i = 0; i < convolutions1.size(); ++i)
		convolutions1[i] = Mat(28, 28, CV_32FC1);
	convolve(28, imageCopy, convolutions1, Wconv1, bconv1);

	// Max pool
	vector<Mat> convolutions1MP(32);
	for (int i = 0; i < convolutions1.size(); ++i)
	{
		convolutions1MP[i] = Mat(14, 14, CV_32FC1);
		maxPool2x2(convolutions1[i], convolutions1MP[i]);
	}

	// Convolve second layer
	vector<Mat> convolutions2(64);
	for (int i = 0; i < convolutions2.size(); ++i)
		convolutions2[i] = Mat(14, 14, CV_32FC1);
	convolve(14, convolutions1MP, convolutions2, Wconv2, bconv2);

	// Max pool
	vector<Mat> convolutions2MP(64);
	for (int i = 0; i < convolutions2.size(); ++i)
	{
		convolutions2MP[i] = Mat(7, 7, CV_32FC1);
		maxPool2x2(convolutions2[i], convolutions2MP[i]);
	}

	// Flatten data
	Mat flattened(7 * 7 * 64, 1, CV_32FC1);
	int count = 0;
	for (int row = 0; row < convolutions2MP[0].rows; ++row)
	{
		for (int col = 0; col < convolutions2MP[0].cols; ++col)
		{
			for (int i = 0; i < convolutions2MP.size(); ++i)
			{
				flattened.at<float>(count, 0) = convolutions2MP[i].at<float>(row, col);
				count++;
			}
		}
	}

	// Densely connected layer
	Mat denselyConnected(1024, 1, CV_32FC1);
	matMul(flattened, denselyConnected, Wfc1, bfc1, true);

	// Readout layer
	Mat readOut(62, 1, CV_32FC1);
	matMul(denselyConnected, readOut, Wfc2, bfc2, false);

	// Softmax
	vector<float> softmaxOutput(62);
	softMax(readOut, softmaxOutput);

	// Use Prediction
	// Find max for prediction
	float max = 0;
	int maxInd = 0;
	int predCount = 0;
	for (vector<float>::iterator it = softmaxOutput.begin(); it != softmaxOutput.end(); ++it)
	{
		if (*it > max)
		{
			max = *it;
			maxInd = predCount;
		}
		predCount++;
	}

	// Set confidence level
	confidence = max;

	// Return predicted character
	return InputImage::oneHotIndexToChar(maxInd);
}

void ConvolutionalNeuralNetwork::testBatch(TrainingSet &data)
{
	vector<InputImage*> *testingData = data.getData();

	cout << "Testing " << testingData->size() << " images..." << endl;

	int numCorrect = 0;

	#pragma omp parallel for reduction(+:numCorrect)
	for (int i = 0; i < testingData->size(); ++i)
	{
		float confidence;
		char classifiedChar = classify((*testingData)[i], confidence);
		//cout << "Predicted: " << classifiedChar << " | Actual: " << (*testingData)[i]->getCharLabel() << endl;

		if (tolower(classifiedChar) == tolower((*testingData)[i]->getCharLabel()))
		{
			numCorrect++;
		}
	}

	float percentCorrect = ((float)numCorrect / (float)testingData->size()) * 100;
	cout << "Percent correct: " << (int)percentCorrect << "%" << endl;
}

void ConvolutionalNeuralNetwork::convolve(int imageSize, vector<Mat> &images, vector<Mat> &outputImages, vector<vector<vector<vector<float>>>> &w, vector<float> &b)
{
	for (int feature = 0; feature < outputImages.size(); ++feature)
	{
		int numRows = imageSize;
		int numCols = imageSize;
		for (int i = 0; i < numRows; ++i)
		{
			for (int j = 0; j < numCols; ++j)
			{
				// [i][j] is the anchor location
				// Center of 5x5 matrix is at [2][2]
				float sum = 0;
				#pragma omp parallel for reduction(+:sum)
				for (int x = 0; x < 5; ++x)
				{
					for (int y = 0; y < 5; ++y)
					{
						// [x][y] correspond to index of 5x5 convolution matrix
						int rowToAccess = i + (x - 2);
						int colToAccess = j + (y - 2);

						bool isInvalidLocation = rowToAccess < 0 || colToAccess < 0 || rowToAccess >= numRows || colToAccess >= numCols;

						for (int inputFeature = 0; inputFeature < images.size(); ++inputFeature)
						{
							float pixel;

							if (isInvalidLocation)
							{
								pixel = 0; // ????
							}
							else
							{
								pixel = images[inputFeature].at<float>(rowToAccess, colToAccess);
							}

							sum += w[x][y][inputFeature][feature] * pixel;
						}
					}
				}

				// Add bias and set pixel
				outputImages[feature].at<float>(i, j) = sum + b[feature];

				// Apply relu
				if (outputImages[feature].at<float>(i, j) < 0)
					outputImages[feature].at<float>(i, j) = 0;
				//outputImages[feature].at<float>(i, j) = softPlus(outputImages[feature].at<float>(i, j));
			}
		}
	}
}

void ConvolutionalNeuralNetwork::maxPool2x2(Mat &image, Mat &outputImage)
{
	#pragma omp parallel for
	for (int i = 0; i < outputImage.rows; ++i)
	{
		for (int j = 0; j < outputImage.cols; ++j)
		{
			float topLeft = image.at<float>(i * 2, j * 2);
			float bottomLeft = image.at<float>(i * 2 + 1, j * 2);
			float topRight = image.at<float>(i * 2, j * 2 + 1);
			float bottomRight = image.at<float>(i * 2 + 1, j * 2 + 1);
			float max = max(topLeft, max(bottomLeft, max(topRight, bottomRight)));

			outputImage.at<float>(i, j) = max;
		}
	}
}

void ConvolutionalNeuralNetwork::matMul(Mat &input, Mat &output, vector<vector<float>> &w, vector<float> &b, bool rectify)
{
	for (int outputInd = 0; outputInd < output.rows; ++outputInd)
	{
		float sum = 0;
		#pragma omp parallel for reduction(+:sum)
		for (int j = 0; j < input.rows; ++j)
		{
			sum += input.at<float>(j, 0) * w[j][outputInd];
		}

		// Add bias
		output.at<float>(outputInd, 0) = sum + b[outputInd];

		// Apply relu
		if (rectify && output.at<float>(outputInd, 0) < 0)
			output.at<float>(outputInd, 0) = 0;
		//if (rectify)
		//	output.at<float>(outputInd, 0) = softPlus(output.at<float>(outputInd, 0));
	}
}

void ConvolutionalNeuralNetwork::softMax(Mat &input, vector<float> &output)
{
	vector<float> exponentials(LABEL_SIZE);
	float exponentialSum = 0;

	// Compute softmax values
	#pragma omp parallel for reduction(+:exponentialSum)
	for (int i = 0; i < LABEL_SIZE; i++)
	{
		exponentials[i] = expf(input.at<float>(i, 0));
		if (isinf(exponentials[i]))
			exponentials[i] = INFINITY;
		exponentialSum += exponentials[i];
	}

	if (exponentialSum == 0)
	{
		exponentialSum = 0.0000001;
	}

	// Compute predictions
	for (int i = 0; i < LABEL_SIZE; i++)
	{
		output[i] = exponentials[i] / exponentialSum;
		if (isinf(output[i]) || output[i] > 1)
			output[i] = 1;
		else if (output[i] < 0)
			output[i] = 0;
	}
}

void ConvolutionalNeuralNetwork::stitch(vector<Mat> &images)
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

float ConvolutionalNeuralNetwork::softPlus(float num)
{
	return logf(1 + expf(num));
}

char ConvolutionalNeuralNetwork::classifyWithRotation(InputImage *image, float &confidence)
{
	// Get original image
	Mat* originalImage = image->getOriginalimage();

	int rotationIntervals = 5;

	vector <std::pair<char, float>> results (360/ rotationIntervals);

	// Rotates in 5 degree intervals
	#pragma omp parallel for
	for (int angle = 0; angle < 360; angle += rotationIntervals)
	{
		// Do rotation
		Mat rotatedImage;
		originalImage->copyTo(rotatedImage);
		InputImage::rotateImage(rotatedImage, angle);

		// Do classification
		InputImage rotatedInputImage(&rotatedImage, image->getCharLabel(), true);
		float tempConfidence;
		char tempClassifiedChar = classify(&rotatedInputImage, tempConfidence);

		// Print for debugging
		//cout << tempConfidence << " ";

		results[angle / rotationIntervals].first = tempClassifiedChar;
		results[angle / rotationIntervals].second = tempConfidence;
	}

	
	float maxConfidence = 0;
	char classifiedChar;
	int rotatedAngle = 0;
	// Gets most confident result
	for (int i = 0; i < results.size(); ++i)
	{
		if (results[i].second > maxConfidence)
		{
			maxConfidence = results[i].second;
			rotatedAngle = i * rotationIntervals;
			classifiedChar = results[i].first;
		}
	}
	

	// Perform a count-sort algorithm
	/*
	char classifiedChar;
	vector<float> weights(62, 0);
	for (int i = 0; i < results.size(); ++i)
	{
		// Count-sorts each result by its confidence
		int index = InputImage::charToOneHotIndex(results[i].first);
		weights[index] += expf(results[i].second); // adds confidence of this count
	}
	float maxWeight = 0;
	for (int i = 0; i < weights.size(); ++i)
	{
		// Finds the maximum weight
		if (weights[i] > maxWeight)
		{
			classifiedChar = InputImage::oneHotIndexToChar(i);
			maxWeight = weights[i];
		}
	}
	*/

	//cout << endl;
	//cout << "Rotated " << rotatedAngle << " degrees" << endl;
	confidence = maxConfidence;
	//confidence = maxWeight;
	return classifiedChar;
}

void ConvolutionalNeuralNetwork::startCMDInput()
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
				TrainingSet testingSet(batchPath, true);
				testBatch(testingSet);
			}
			
			catch(cv::Exception& e)
			{
				cout << "Something went wrong" << endl;
			}

			cout << endl;
		}
		else if (command == "classifyWithRotation")
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
				float confidence;
				char c;
				try 
				{
					InputImage inputImage(savePath + "\\label.jpg", true);
					c = classifyWithRotation(&inputImage, confidence);
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
				float confidence;
				char c;
				try
				{
					InputImage inputImage(savePath + "\\label.jpg", true);
					c = classify(&inputImage, confidence);
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
			cout << "      classifyWithRotation <imagePath>" << endl;
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

bool ConvolutionalNeuralNetwork::isInitialized()
{
	return this->initialized;
}

void ConvolutionalNeuralNetwork::trimString(std::string &str)
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