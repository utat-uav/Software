#include "stdafx.h"
#include "InputImage.h"
#include "TrainingSet.h"
#include "NeuralNetwork.h"
#include "FileManager.h"
#include "ConvolutionalNeuralNetwork.h"
#include "BarcodeReader.h"
#include "Identifier.h"

// Guide: 
// -t "D:\Downloads\Images\Reformatted"  "D:\Downloads\training2\allinone" for training
// -r "D:\Downloads\training2\allInOne" "D:\Workspace\UAV\cv16-classification\SoftmaxRegression\SoftmaxRegression\savednnBAK2.txt" for testing

//--------------------------------------------------------------------
// PROTOTYPES
//--------------------------------------------------------------------

void credits();
void resaveImages(string outputFolder, FileManager& files);
void fixOliversShit(string outputFolder, FileManager& files);
void train(string path, string testPath);
void run(string path, string serializedNN);
void runCNN(string path, string programPath);
void runZBar(string path);
void saveProcessedDataset(string path);


//--------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------

int main(int argc, char** argv)
{
	// Test identifier with:
	// -identify "D:\Workspace\UAV\AUVSI2015Pictures\test1.jpg" "D:\Workspace\UAV\AUVSI2015Pictures\uav_gps.log" "D:\Workspace\UAV\AUVSI2015Pictures\Test Output"

	// Test cnn with:
	// -cnn "D:\Workspace\UAV\Software\Classification\Release\TrainedCNN"

	/*
	// Serialize for training/testing
	//saveProcessedDataset("D:\\Workspace\\UAV\\classification-training\\training2\\allinone\\");
	saveProcessedDataset("D:\\Workspace\\UAV\\classification-training\\Images\\Reformatted\\");
	return 0;

	// Fix oliver's shit
	FileManager fileManager("D:\\Workspace\\UAV\\classification-training\\Images\\JPEG\\");
	fixOliversShit("D:\\Workspace\\UAV\\classification-training\\Images\\Reformatted\\", fileManager);
	return 0;
	*/

	credits();

	if (argc < 2)
	{
		cout << "Insufficient arguments. Exiting program..." << endl;
		return -1;
	}

	string mode = argv[1];

	// Command inputs
	if (mode == "-t")
	{
		if (argc < 4)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// COMMENCE TRAINING" << endl << endl;
		string path = argv[2];
		string testPath = argv[3];

		train(path, testPath);
	}
	else if (mode == "-r")
	{
		if (argc < 4)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// COMMENCE RUNNING" << endl << endl;
		string path = argv[2];
		string serializedPath = argv[3];

		run(path, serializedPath);
	}
	else if (mode == "-s")
	{
		cout << "// COMMENCE DATA SAVING" << endl << endl;
		string path = argv[2];

		saveProcessedDataset(path);
	}
	else if (mode == "-cnn")
	{
		if (argc < 3)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// COMMENCE CONVOLUTIONAL NEURAL NETWORK" << endl << endl;
		string path = argv[2];
		string programPath = argv[0];

		runCNN(path, programPath);
	}
	else if (mode == "-zbar")
	{
		if (argc < 3)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// STARTING ZBAR..." << endl << endl;
		string path = argv[2];

		runZBar(path);
	}
	else if (mode == "-identify")
	{
		if (argc < 5)
		{
			cout << "Insufficient arguments. Exiting program..." << endl;
			return -1;
		}

		cout << "// STARTING IDENTIFIER..." << endl << endl;
		string fileName = argv[2];
		string gpsLog = argv[3];
		string outputFolder = argv[4];

		string results;
		Identifier identifier(fileName, gpsLog, outputFolder, &results);
		identifier.analyze();
	}
	else if (mode == "-help")
	{
		cout << "Press enter to continue..." << endl;
		cin.ignore();
	}

	//InputImage img("D:\\Workspace\\UAV\\classification-training\\Testingset\\test.png", true);

	//int test = InputImage::charToOneHotIndex('Z');
	//char test2 = InputImage::oneHotIndexToChar(test);

	return 0;
}


//--------------------------------------------------------------------
// FUNCTIONS
//--------------------------------------------------------------------

/*
* Train
*/
void train(string path, string testPath)
{
	// Initialize
	cout << "Importing and formatting training set..." << endl;
	TrainingSet trainingSet(path);
	TrainingSet testSet(testPath);
	NeuralNetwork neuralNetwork;

	// Train
	cout << "Training commencing..." << endl;
	neuralNetwork.train(trainingSet);
	cout << "Training complete" << endl;

	cout << "Testing..." << endl;
	neuralNetwork.test(testSet);
}

/*
* Classify
*/
void run(string path, string serializedNN)
{
	cout << "Importing test images..." << endl;
	TrainingSet testSet(path, true);
	cout << "Initializing NN..." << endl;
	NeuralNetwork neuralNetwork(serializedNN);

	cout << "Testing..." << endl;
	neuralNetwork.test(testSet);
}

/*
* Serialized processed dataset
*/
void saveProcessedDataset(string path)
{
	cout << "Loading images..." << endl;
	TrainingSet testSet(path, false, false); // Conserve memory

	cout << "Writing data..." << endl;
	ofstream savedDataSet;
	savedDataSet.open("serializedData.txt");

	// Get data
	vector<InputImage *> *data = testSet.getData();
	for (vector<InputImage *>::iterator inputImage = data->begin(); inputImage != data->end(); ++inputImage)
	{
		Mat *image = (*inputImage)->getImage();
		vector<int> *code = (*inputImage)->getLabelVector();

		// Pixel info
		for (int i = 0; i < image->rows - 1; ++i)
		{
			savedDataSet << image->at<float>(i, 0) << "\t";
		}
		savedDataSet << endl;

		// One-hot code
		for (int i = 0; i < code->size(); ++i)
		{
			savedDataSet << code->at(i) << "\t";
		}
		savedDataSet << endl;
	}

	savedDataSet << endl;
	savedDataSet.close();
}

/*
* Program initialization
*/
void credits()
{
	cout << "------------------------------------------------" << endl;
	cout << "// UTAT-UAV COMPUTER VISION CLASSIFICATION V2 \\\\" << endl;
	cout << "------------------------------------------------" << endl;
	cout << endl;
	cout << "Author(s): Davis Wu" << endl;
	cout << "Creation Date: Dec 30, 2015" << endl;
	cout << "Latest Modification Date: April 28, 2016" << endl;
	cout << "Info: attempts to classify characters" << endl;
	cout << endl;
	cout << "Command line info: [-t <trainingset> <testset>] to train program" << endl;
	cout << "                   [-r <testset> <serializedNN>] to run pre-trained program" << endl;
	cout << "                   [-cnn <serializedNN>] to run pre-trained program" << endl;
	cout << "                   [-s <dataset>] to serialize dataset" << endl;
	cout << "                   [-zbar <imagepath>] to read barcode" << endl;
	cout << "                   [-help me] for help" << endl;
	cout << endl;
}

/*
* Fixes Oliver's shit
*/
void fixOliversShit(string outputFolder, FileManager& files)
{
	cout << "Fixing Oliver's shit..." << endl;

	vector<string>* filePaths = files.getFiles();

	int count = 0;
	for (vector<string>::iterator file = filePaths->begin(); file != filePaths->end(); ++file)
	{
		// Process string
		int slashLocation = file->find_last_of('\\');

		// Get file name
		string fileName = file->substr(slashLocation+1);

		// Get corresponding number
		string imgNumberStr = fileName.substr(3, 3);
		int imgNumber = stoi(imgNumberStr);

		char label;

		// Check cases
		if (imgNumber >= 1 && imgNumber <= 10)
		{
			// Image is a number
			label = '0' + imgNumber - 1;
		}
		else if (imgNumber >= 11 && imgNumber <= 36)
		{
			// Image is a capital letter
			label = 'A' + imgNumber - 11;
		}
		else if (imgNumber >= 37 && imgNumber <= 62)
		{
			// Image is a lowercase letter
			label = 'a' + imgNumber - 37;
		}
		else
		{
			label = ' ';
		}

		//#pragma omp parallel for
		for (int rotIndex = 0; rotIndex < 3; rotIndex++) {
			// Calculate rotation from index
			int rot = -12 + rotIndex * 12;

			// Open file
			Mat image;
			image = imread(*file, IMREAD_COLOR); // Read the file

			// Rotate
			InputImage::rotateImage(image, rot);

			// Convert colours
			cvtColor(image, image, CV_BGR2GRAY);
			threshold(image, image, 128, 255, CV_THRESH_BINARY);
			Mat imageCpy;
			image.copyTo(imageCpy);

			// Get bounding boxes
			int largestRow = 0, smallestRow = INFINITY * 2, largestCol = 0, smallestCol = INFINITY * 2;
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

			string finalFileName = outputFolder + label + "_" + to_string(count) + "_rotated" + to_string(rot) + ".png";
			imwrite(finalFileName, image);
		}

		count++;
	}
}


/*
* Does some reformatting
*/
void resaveImages(string outputFolder, FileManager& files)
{
	cout << "Resaving images..." << endl;

	for (unordered_map<string, vector<string>>::iterator folder = files.organizedFolders.begin(); folder != files.organizedFolders.end(); ++folder)
	{
		int count = 500;
		for (vector<string>::iterator file = folder->second.begin(); file != folder->second.end(); ++file)
		{
			if ((file->at(file->length() - 1) == 'g' && file->at(file->length() - 2) == 'n' && file->at(file->length() - 3) == 'p') ||
				(file->at(file->length() - 1) == 'g' && file->at(file->length() - 2) == 'p' && file->at(file->length() - 3) == 'j'))
			{
				Mat image;
				image = imread(*file, IMREAD_COLOR); // Read the file

				// Resave folder
				string finalFileName = outputFolder + folder->first.at(folder->first.length() - 1) + "_" + to_string(count) + ".png";
				imwrite(finalFileName, image);
				count++;
			}
		}
	}

	cout << "Done reformatting images" << endl;
}

/*
* Does barcode scan
*/
void runZBar(string path)
{
	BarcodeReader barcodeReader;
	barcodeReader.scanImage(path);
}

/*
* Classifies with CNN
*/
void runCNN(string path, string programPath)
{
	cout << "Initializing convolutional neural network..." << endl;
	cout << "Program path is " << programPath << " and the cnn path is " << path << endl;
	ConvolutionalNeuralNetwork cnn(path, programPath);

	if (!cnn.isInitialized())
	{
		cout << "Initialization failed. Exiting program..." << endl;
		return;
	}

	// Commence command inputs
	cnn.startCMDInput();
}
