#include "stdafx.h"
#include "Identifier.h"

#include "Utils.h"

/*
 * All in one mode means it will perform all of classification and barcode reading
 */
Identifier::Identifier(const std::string &imagePath, const std::string &gpsLog, const std::string &outputFolder, ::string *results,
	double groundLevel, string programPath, string cnnPath, bool removeFalsePositives)
  : geolocater(75.0, groundLevel), results(results), programPath(programPath),
	cnnPath(cnnPath), removeFalsePositives(removeFalsePositives)
{
	params.imagePath = imagePath;
	params.gpsLog = gpsLog;
	params.outputFolder = outputFolder;

	allInOne = programPath != "" && cnnPath != "";

	// Get the name of the file from the file path
	int indexOfSlash = imagePath.find_last_of("\\");
	if (indexOfSlash == std::string::npos) indexOfSlash = imagePath.find_last_of("/");
	std::string name = imagePath.substr(indexOfSlash + 1);

	params.imageName = name;

	// Make sure the output folder ends in a slash of some sort
	char lastChar = params.outputFolder[params.outputFolder.size() - 1];
	if (lastChar != '\\' && lastChar != '/')
	{
		bool forwardSlash = params.outputFolder.find('/') != std::string::npos;
		if (forwardSlash)
		{
			params.outputFolder.push_back('/');
		}
		else
		{
			params.outputFolder.push_back('\\');
		}
	}
}


Identifier::~Identifier()
{
}


void Identifier::analyze()
{
	// Read from gps log
	readGPSLog();

	// Get image
	cv::Mat image = imread(params.imagePath, CV_LOAD_IMAGE_COLOR);   // Read the file;
	saveCompressedImage(image);

	// Get image info
	int width, height, channels;
	width = image.cols;
	height = image.rows;
	channels = image.channels();

	// update geolocation params
	geolocater.updateParams(width, height, params.heading, params.altitude, LatLon(params.latitude, params.longitude));
	writeAnalysisParameters(width, height);

	// Get hsv, apply blur
	cv::Mat hsvImage;
	cv::cvtColor(image, hsvImage, cv::COLOR_RGB2HSV);
	cv::blur(hsvImage, hsvImage, cv::Size(6, 6));

	// Decide on a constant size
	const int constSize = 1500;
	double rowScale = (double)image.rows / constSize;
	double colScale = (double)image.cols / constSize;

	// Create MSER
	/*
	CV_WRAP static Ptr<MSER> create( int _delta=5,
									int _min_area=60,
									int _max_area=14400,
									double _max_variation=0.25, double _min_diversity=.2,
									int _max_evolution=200, double _area_threshold=1.01,
									double _min_margin=0.003, int _edge_blur_size=5 );
	*/

	cv::Ptr<cv::MSER> mser = cv::MSER::create(6,
		(double)params.minArea / (double)2738 * constSize,
		(double)params.maxArea / (double)2738 * constSize,
		0.099, 0.65, 200, 1.01, 0.003, 5);


	// Split hsv channels
	cv::Mat hsv[3];
	cv::split(hsvImage, hsv);

	// Run MSER on each channel
	std::vector<cv::Rect> mserResults;
	for (int i = 0; i < 3; ++i)
	{
		std::vector<std::vector<cv::Point>> msers;
		std::vector<cv::Rect> bboxes;
		Mat resized(hsv[i].rows, hsv[i].cols, hsv[i].depth());
		cv::resize(hsv[i], resized, cv::Size(constSize, constSize), 0, 0, cv::INTER_CUBIC);

		mser->detectRegions(resized, msers, bboxes);
		mserResults.insert(mserResults.end(), bboxes.begin(), bboxes.end());
	}

	// Rescale the bounding boxes
	for (int i = 0; i < mserResults.size(); ++i)
	{
		mserResults[i].x *= colScale;
		mserResults[i].width *= colScale;

		mserResults[i].y *= rowScale;
		mserResults[i].height *= rowScale;
	}

	removeDuplicates(mserResults, image);

	Classifier *classifier = NULL;
	if (allInOne)
	{
		classifier = new Classifier(cnnPath, programPath);
		classifier->initialize();
	}

	// Process mserResults
	std::vector<CropResult> cropResults;
	for (int i = 0; i < mserResults.size(); ++i)
	{
		CropResult cropResult;

		double maxDim = mserResults[i].width > mserResults[i].height ?
			mserResults[i].width : mserResults[i].height;

		// Get cropped region
		double centerX = mserResults[i].x + mserResults[i].width / 2;
		double centerY = mserResults[i].y + mserResults[i].height / 2;
		double x1 = centerX - maxDim / 2 - params.cropPadding;
		double y1 = centerY - maxDim / 2 - params.cropPadding;
		double x2 = centerX + maxDim / 2 + params.cropPadding;
		double y2 = centerY + maxDim / 2 + params.cropPadding;
		if (x1 < 0) x1 = 0;
		if (y1 < 0) y1 = 0;
		if (x2 < 0) x2 = 0;
		if (y2 < 0) y2 = 0;
		if (x1 >= image.cols) x1 = image.cols - 1;
		if (y1 >= image.rows) y1 = image.rows - 1;
		if (x2 >= image.cols) x2 = image.cols - 1;
		if (y2 >= image.rows) y2 = image.rows - 1;
		cv::Rect roi(x1, y1, x2 - x1, y2 - y1);

		cropResult.x = centerX;
		cropResult.y = centerY;
		cropResult.size = roi.width > roi.height ? roi.width : roi.height;

		// No area of 0 please
		if (roi.area() == 0) continue;

		// Do crop
		cv::Mat crop = image(roi);

		// Check color
		cv::Scalar meanColor = cv::mean(crop);
		double mag = std::sqrt(meanColor[0] * meanColor[0] + meanColor[1] * meanColor[1] + meanColor[2] * meanColor[2]);
		if (mag < 35)
		{
			continue;
		}

		// Check if valid by doing all the classification tests
		if (allInOne)
		{
			if (!Utils::allInOneClassify(crop, classifier, cropResult.classifierResults) && removeFalsePositives)
			{
				continue;
			}
		}

		// Write image
		// Constructor ensures that params.outputFolder ends in a slash
		cropResult.imageName = getCropName(params.imageName, i);
		cv::imwrite(params.outputFolder + cropResult.imageName, crop);

		// Get LatLon of the crop center
		cropResult.coords = geolocater.targLatLon(centerY, centerX);
		cropResults.push_back(cropResult);
	}

	if (allInOne)
	{
		delete classifier;
	}

	// Write results to output file
	writeCropResults(cropResults);

	// Draw the mserResults
	for (int i = 0; i < mserResults.size(); ++i)
	{
		cv::rectangle(image, mserResults[i], CV_RGB(0, 255, 0));
	}

	// Save file
	std::string imageNameNoExtension = params.imageName.substr(0, params.imageName.find_last_of('.'));
	std::ofstream out(params.outputFolder + imageNameNoExtension + " results.ini");
	out << *results;
	out.close();

	// Show result
	//namedWindow("Debug", WINDOW_AUTOSIZE);
	//imshow("Debug", image);
	//waitKey(0);
}


void Identifier::removeDuplicates(std::vector<cv::Rect>& mserResults, const Mat &image)
{
	// get the average bbox sidelength
	double total = 0, avg = 0;
	for (int i = 0; i < mserResults.size(); ++i)
		total += sqrt(mserResults[i].area());

	avg = total / mserResults.size();

	// filter mserResults, e.g boxes within boxes and other redundant bounds
	double gridLength = min(3 * avg, image.cols / 5);
	int gridsPerrow = ceil(image.cols / gridLength);
	int gridsPerCol = ceil(image.rows / gridLength);
	std::unordered_map<int, std::pair<int, cv::Rect>> hashTable;
	std::vector<int> occurenceCount(mserResults.size(), 0);

	// use 4 differing grid offsets, we will only take elements that appear in all 4
	for (int s = 0; s < 4; ++s)
	{
		// binary counter pattern for offsets, 00, 01, 10, 11
		double xoffset = gridLength * 0.5 * (s % 2);
		double yoffset = gridLength * 0.5 * (s >= 2);

		for (int i = 0; i < mserResults.size(); ++i)
		{
			double centerX = mserResults[i].x + mserResults[i].width / 2;
			double centerY = mserResults[i].y + mserResults[i].height / 2;

			// min x , y = 0
			// max x , y = image.rows
			// use extra precaution not to hash out of bounds
			int xHash = min(floor((centerX + xoffset) / gridLength), gridsPerrow - 1);
			int yHash = min(floor((centerY + yoffset) / gridLength), gridsPerCol - 1);
			xHash = max(xHash, 0);
			yHash = max(yHash, 0);
			int hashIndex = yHash*gridsPerrow + xHash;
			auto p = hashTable.find(hashIndex);

			if (p == hashTable.end())
			{
				std::pair<int, cv::Rect> info = std::make_pair(i, mserResults[i]);
				hashTable.insert(std::make_pair(hashIndex, info));
			}
			else if (p->second.second.area() < mserResults[i].area())
				p->second = std::make_pair(i, mserResults[i]);
		}

		// increase counts, move on to next grid offset
		for (auto it = hashTable.begin(); it != hashTable.end(); ++it)
			++occurenceCount[it->second.first];
		hashTable.clear();
	}

	// delete rois that don't remain in all 4 offset grid
	int j = 0;
	for (int i = 0; i < mserResults.size(); ++i)
	{
		if (occurenceCount[j++] < 4)
		{
			mserResults.erase(mserResults.begin() + i);
			--i;
		}
	}
}

void Identifier::readGPSLog()
{
	// Parameters to fill
	std::string latitudeString, longitudeString, altitudeString, headingString, headingEnglishString;
	std::string date;

	results->append("[Position]\n");
	
	std::string line;
	std::ifstream file(params.gpsLog);
	
	bool firstLine = true;

	if (file.is_open())
	{
		while (getline(file, line))
		{
			// Get past the header
			if (line.find("Latitude") != std::string::npos)
			{
				firstLine = false;
			}
			if (firstLine)
			{
				continue;
			}

			// Split string
			std::vector<std::string> splitLine;
			std::stringstream ss(line);
			std::string token;
			while (std::getline(ss, token, ','))
			{
				splitLine.push_back(token);
			}

			// There are 5 items
			// Image name, latitude, longitude, altitude, heading
			if (splitLine.size() >= 5 && splitLine[0] == params.imageName)
			{
				params.latitude = std::stod(splitLine[1]);
				latitudeString = splitLine[1];

				params.longitude = std::stod(splitLine[2]);
				longitudeString = splitLine[2];

				params.altitude = std::stod(splitLine[3]);
				altitudeString = splitLine[3];

				params.heading = std::stod(splitLine[4]);
				headingString = splitLine[4];
				headingEnglishString = headingToEnglish(params.heading);

				if (splitLine.size() >= 6)
				{
					date = splitLine[5];
				}
			}
		}

		file.close();
	}

	results->append("heading=" + headingEnglishString + "\n");
	results->append("headingDegrees=" + headingString + "\n");
	results->append("latitude=" + latitudeString + "\n");
	results->append("longitude=" + longitudeString + "\n");
	results->append("altitude=" + altitudeString + "\n");
}

std::string Identifier::headingToEnglish(double headingDegrees)
{
	if (headingDegrees >= 45 && headingDegrees < 135)
		return "East";
	if (headingDegrees >= 135 && headingDegrees < 225)
		return "South";
	if (headingDegrees >= 225 && headingDegrees < 315)
		return "West";
	if (headingDegrees < 45 || headingDegrees >= 315)
		return "North";

	return "";
}

std::string Identifier::getCropName(std::string imageName, int index)
{
	imageName = imageName.substr(0, imageName.find_last_of('.'));
	return imageName + "roi" + std::to_string(index) + ".jpg";
}

void Identifier::writeCropResults(const std::vector<CropResult> &cropResults)
{
	results->append("\n[Crop Info]\n");
	results->append("Number of Crops=" + std::to_string(cropResults.size()) + "\n");
	for (int i = 0; i < cropResults.size(); ++i)
	{
		results->append("\n[Crop " + std::to_string(i + 1) + "]\n");
		results->append("Image Name=" + cropResults[i].imageName + "\n");
		results->append("X=" + std::to_string(cropResults[i].x) + "\n");
		results->append("Y=" + std::to_string(cropResults[i].y) + "\n");
		results->append("Size=" + std::to_string(cropResults[i].size) + "\n");

		LatLon coords = cropResults[i].coords;
		results->append("latitude=" + std::to_string(coords.lat) + "\n");
		results->append("longitude=" + std::to_string(coords.lon) + "\n");
		results->append("Background Color=" + cropResults[i].classifierResults.shapeColor + "\n");
		results->append("Alphanumeric Color=" + cropResults[i].classifierResults.characterColor + "\n");
		results->append("Shape=" + cropResults[i].classifierResults.shape + "\n");
		results->append("Alphanumeric=" + string(1, cropResults[i].classifierResults.character) + "\n");
		results->append("Description=" + cropResults[i].classifierResults.description + "\n");
	}
}

void Identifier::writeAnalysisParameters(int width, int height)
{
	std::string tempGPSLog = params.gpsLog;
	std::replace(tempGPSLog.begin(), tempGPSLog.end(), '\\', '/');

	std::string tempImagePath = params.outputFolder + params.imageName;
	std::replace(tempImagePath.begin(), tempImagePath.end(), '\\', '/');

	results->append("\n[Analysis Parameters]\n");
	results->append("Width=" + std::to_string(width) + "\n");
	results->append("Height=" + std::to_string(height) + "\n");
	results->append("GPS_LOG=" + tempGPSLog + "\n");
	results->append("IMAGE=" + tempImagePath + "\n");
}

void Identifier::saveCompressedImage(const Mat &image)
{
	Mat resized;
	cv::resize(image, resized, cv::Size(700, (double)700 / image.cols*image.rows));

	cv::imwrite(params.outputFolder + params.imageName, resized);
}
