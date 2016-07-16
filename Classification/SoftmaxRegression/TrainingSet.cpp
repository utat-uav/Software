#include "stdafx.h"
#include "TrainingSet.h"

TrainingSet::TrainingSet(string path, bool testSet, bool noMemoryConserve)
{
	// Get file names
	vector<string> files;

	FILE* pipe = NULL;
	string pCmd = "dir /B /S " + string(path);
	char buf[256];

	// Attempt opening directory
	if (NULL == (pipe = _popen(pCmd.c_str(), "rt")))
	{
		cout << "Shit. Directory's broken." << endl;
		return;
	}

	// Get file names
	while (!feof(pipe))
	{
		if (fgets(buf, 256, pipe) != NULL)
		{
			files.push_back(string(buf));
		}
	}

	// Close
	_pclose(pipe);

	// Import data
	for (vector<string>::iterator file = files.begin(); file != files.end(); ++file)
	{
		// Remove \n character
		file->pop_back();

		InputImage *inputImage = new InputImage(*file, testSet, true, noMemoryConserve);

		// Validity check
		if (!inputImage->isValid())
		{
			delete inputImage;
			cout << path << " is not valid data" << endl;
			continue;
		}

		// Add to data
		data.push_back(inputImage);
	}

	// Shuffle vector
	srand(time(0));
	random_shuffle(data.begin(), data.end());

	// Testing
	//vector<int> vectorizedImage = data.at(5)->getVectorizedImage();
	//vector<int> *label = data.at(5)->getLabelVector();
	/*
	Mat *mat = data.at(20)->getImage();
	*mat = mat->reshape(0, 20);
	imwrite("test.jpg", *mat);
	*/
}

TrainingSet::~TrainingSet()
{
	// Deallocate data set
	for (vector<InputImage *>::iterator it = data.begin(); it != data.end(); ++it)
	{
		delete (*it);
	}
}

int TrainingSet::getDataSize()
{
	return this->data.size();
}

vector<InputImage *>* TrainingSet::getData()
{
	return &(this->data);
}

InputImage* TrainingSet::operator[](int i)
{
	return data[i];
}