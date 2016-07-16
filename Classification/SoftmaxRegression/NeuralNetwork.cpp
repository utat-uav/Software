#include "stdafx.h"
#include "NeuralNetwork.h"


NeuralNetwork::NeuralNetwork()
{
	// ROWS X COLUMNS initialization
	// DATA_SIZE + 1 for bias parameters
	parameters = Mat(LABEL_SIZE, DATA_SIZE + 1, DataType<float>::type);

	// Create rand number generator seed based on time
	srand(time(NULL));

	// (EXPERIMENTAL) Initalize all parameters to somewhat random number
	for (int i = 0; i < parameters.rows; i++)
	{
		for (int j = 0; j < parameters.cols; j++)
		{
			int randNumber = rand() % 100;
			parameters.at<float>(i, j) = (float)randNumber / (float)1000000;
		}
	}

	// Allocate pointers
	parameterPointers = new float*[(LABEL_SIZE)*(DATA_SIZE + 1)];

	// Assign pointers for fast access
	for (int i = 0; i < parameters.rows; i++)
	{
		for (int j = 0; j < parameters.cols; j++)
		{
			parameterPointers[i*(DATA_SIZE + 1) + j] = &(parameters.at<float>(i, j));
		}
	}
}

// Initialize it with a pre-trained NN
NeuralNetwork::NeuralNetwork(string serializedPath)
{
	parameters = Mat(LABEL_SIZE, DATA_SIZE + 1, DataType<float>::type);

	ifstream serializedNN;
	serializedNN.open(serializedPath);

	// Loads NN parameters from file
	for (int i = 0; i < parameters.rows; ++i)
	{
		for (int j = 0; j < parameters.cols; ++j)
		{
			float val;
			serializedNN >> val;
			parameters.at<float>(i, j) = val;
		}
	}

	serializedNN.close();

	// Allocate pointers
	parameterPointers = new float*[(LABEL_SIZE)*(DATA_SIZE + 1)];

	// Assign pointers for fast access
	for (int i = 0; i < parameters.rows; i++)
	{
		for (int j = 0; j < parameters.cols; j++)
		{
			parameterPointers[i*(DATA_SIZE + 1) + j] = &(parameters.at<float>(i, j));
		}
	}
}

NeuralNetwork::~NeuralNetwork()
{
	delete[] parameterPointers;
}

void NeuralNetwork::predictHelper(Mat &V, vector<float> &predictions)
{
	vector<float> exponentials(LABEL_SIZE);
	float exponentialSum = 0;

	// Compute softmax values
	//#pragma omp parallel for reduction(+:exponentialSum)
	for (int i = 0; i < LABEL_SIZE; i++)
	{
		exponentials[i] = expf(V.at<float>(i, 0));
		if (isinf(exponentials[i]))
			exponentials[i] = INFINITY;
		exponentialSum += exponentials[i];
	}

	if (exponentialSum == 0)
	{
		exponentialSum = 0.0000001;
	}

	// Compute predictions
	//#pragma omp parallel for
	for (int i = 0; i < LABEL_SIZE; i++)
	{
		//predictions.push_back(exponentials[i] / exponentialSum);
		predictions[i] = exponentials[i] / exponentialSum;
		if (isinf(predictions[i]) || predictions[i] > 1)
			predictions[i] = 1;
		else if (predictions[i] < 0)
			predictions[i] = 0;
	}
}

void NeuralNetwork::train(TrainingSet &trainingSet)
{
	vector<InputImage *>* data = trainingSet.getData();

	vector<float> G;

	// Repeat until convergence
	bool hasConverged = false;
	int count = 0;
	float avgCrossEntropy = 100;
	time_t timer;
	time(&timer);
	int k = 0;
	while (!hasConverged)
	{
		if (count > MIN_TRAIN_TIME)
		{
			hasConverged = true;
			break;
		}
		count++;

		if (count % 5 == 0)
		{
			cout << count << "th cycle with " << avgCrossEntropy << " avg cross entropy" << endl;
			cout << difftime(time(0), timer) << " seconds elapsed" << endl;
		}

		// Reset average crossentropy
		avgCrossEntropy = 0;

		// Get predictions
		vector<vector<float>> allPredictions;
		vector<InputImage *> inputImages;
		for (int m = k; m < k + BATCH_SIZE; ++m)
		{
			int ind = m % data->size();

			Mat *trainingImageMat = data->at(ind)->getImage();
			vector<int> *actualLabel = data->at(ind)->getLabelVector();

			// Get V
			Mat V = parameters * (*trainingImageMat);

			// Compute prediction
			vector<float> predictions(LABEL_SIZE);
			predictHelper(V, predictions);

			avgCrossEntropy -= (logf(predictions[data->at(ind)->getLabelIndex()]));

			allPredictions.push_back(predictions);
			inputImages.push_back(data->at(ind));
		}

		// Update parameters
		for (int i = 0; i < parameters.rows; ++i)
		{
			for (int j = 0; j < parameters.cols; ++j)
			{
				float grad = 0;
#pragma omp parallel for reduction(+:grad)
				for (int p = 0; p < BATCH_SIZE; p++)
				{
					grad += inputImages.at(p)->getImage()->at<float>(j, 0) * (inputImages.at(p)->getLabelVector()->at(i) - allPredictions[p][i]);
				}

				parameters.at<float>(i, j) += TRAINING_STEP * grad;
			}
		}

		// Average the cross entropy
		avgCrossEntropy /= BATCH_SIZE;

		k += BATCH_SIZE;
	}

	// Save to file
	ofstream nnsave;
	nnsave.open("savednn.txt");
	for (int i = 0; i < parameters.rows; ++i)
	{
		for (int j = 0; j < parameters.cols; ++j)
		{
			nnsave << parameters.at<float>(i, j) << "\t";
		}
		nnsave << endl;
	}
	nnsave << endl;
	nnsave.close();

	//cout << parameters << endl;
}

void NeuralNetwork::test(TrainingSet &testSet)
{
	vector<InputImage *>* data = testSet.getData();

	int numCorrect = 0;
	for (vector<InputImage *>::iterator testImage = data->begin(); testImage != data->end(); ++testImage)
	{
		Mat *trainingImageMat = (*testImage)->getImage();
		vector<int> *actualLabel = (*testImage)->getLabelVector();

		// Get V
		Mat V = parameters * (*trainingImageMat);

		// Compute prediction
		vector<float> predictions(LABEL_SIZE);
		predictHelper(V, predictions);

		// Find max for prediction
		float max = 0;
		int maxInd = 0;
		int count = 0;
		for (vector<float>::iterator it = predictions.begin(); it != predictions.end(); ++it)
		{
			if (*it > max)
			{
				max = *it;
				maxInd = count;
			}
			count++;
		}

		char predictedChar = InputImage::oneHotIndexToChar(maxInd);
		cout << "Predicted: " << predictedChar << " | Actual: " << (*testImage)->getCharLabel() << endl;
		if (tolower(predictedChar) == tolower((*testImage)->getCharLabel()))
		{
			numCorrect++;
		}
	}

	float percentCorrect = ((float)numCorrect / (float)data->size()) * 100;
	cout << "Percent correct: " << (int)percentCorrect << "%" << endl;
}
