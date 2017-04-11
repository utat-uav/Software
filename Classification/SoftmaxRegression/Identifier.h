#pragma once

#include "stdafx.h"
#include "Geolocation.h"

class Identifier
{
public:
	Identifier(const std::string &imagePath, const std::string &gpsLog, const std::string &outputFolder, std::string *results);
	virtual ~Identifier();

	struct Params
	{
		Params()
		{
			sizeOfROI = 300;
			minPointsInCluster = 5;
			maxArea = 300000;
			minArea = 2650;
			cropPadding = 3;
		}

		std::string imagePath;
		std::string imageName;
		std::string gpsLog;
		std::string outputFolder; 
		unsigned sizeOfROI;
		unsigned minPointsInCluster;
		unsigned maxArea;
		unsigned minArea;
		unsigned cropPadding;

		// updated everytime image is read
		double latitude, longitude, altitude, heading;
	};

	struct CropResult
	{
		std::string imageName;

		// in pixels, x and y specify the center of the crop
		int x;
		int y;
		int size;
		LatLon coords = LatLon(0,0);
	};

	void analyze();

private:
	Params params;
	Geolocation geolocater = Geolocation(75.0);
	std::string *results;

	void readGPSLog();
	std::string headingToEnglish(double headingDegrees);
	std::string getCropName(std::string imageName, int index);
	void writeCropResults(const std::vector<CropResult> &cropResults);
	void writeAnalysisParameters(int width, int height);
	void saveCompressedImage(const Mat &image);

	// raw mserResults will take many slightly diff. bounding boxes of the same target
	void removeDuplicates(std::vector<cv::Rect>& mserResults, const Mat &image);
};

