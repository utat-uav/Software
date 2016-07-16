#include "stdafx.h"
#include "Segmenter.h"


Segmenter::Segmenter()
{
}

Segmenter::~Segmenter()
{
}

//copy and pasted code
std::vector<cv::Mat> Segmenter::segment(string path)
{
	const int MAX_CLUSTERS = 3;
	const int MAX_NUM_KMEANS_ITER = 50;
	const double MAX_KMEANS_EPSILON = 1.0;

	cv::Mat labim;
	cv::TermCriteria kmeans_terminator = cv::TermCriteria(
		cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
		MAX_NUM_KMEANS_ITER,
		MAX_KMEANS_EPSILON
		);

	//std::cout << "Converting to Lab..." << std::endl;
	cv::Mat test_im = cv::imread(path, CV_LOAD_IMAGE_COLOR);

	//blur, then convert to HSV
	cv::medianBlur(test_im, test_im, 3);
	//cv::blur(test_im, test_im, cv::Size(3, 3));
	cv::cvtColor(test_im, labim, COLOR_BGR2HSV);

	// Taking relevant channels
	cv::Mat lab_chan[3];
	cv::Mat abim = cv::Mat(test_im.rows, test_im.cols, CV_8UC2);

	//std::cout << "Rows: " << test_im.rows << " Cols: " << test_im.cols
	//	<< " Channels: " << test_im.channels() << std::endl;

	// Take only ab channels (here we actually take all the channels...)
	cv::split(labim, lab_chan);
	std::vector<cv::Mat> ab_chan;

	// Load the vector
	ab_chan.push_back(lab_chan[0]);
	ab_chan.push_back(lab_chan[1]);
	ab_chan.push_back(lab_chan[2]);

	// MERGE!!
	cv::merge(ab_chan, abim);

	//std::cout << "abim :: Rows: " << abim.rows << " Cols: " << abim.cols
	//	<< " Channels: " << abim.channels() << std::endl;

	//std::cout << "Reshaping..." << std::endl;
	cv::Mat abim_res = abim.reshape(1, test_im.rows * test_im.cols);

	//std::cout << "# chan " << abim_res.channels() << std::endl;
	//std::cout << "Showing..." << std::endl;
	//cv::imwrite("abim.jpg", abim_res);
	abim_res.convertTo(abim_res, CV_32FC2);

	//std::cout << "Running KMeans..." << std::endl;
	cv::Mat mylabels;
	cv::kmeans(abim_res, MAX_CLUSTERS, mylabels, kmeans_terminator, MAX_NUM_KMEANS_ITER, cv::KMEANS_RANDOM_CENTERS);

	//std::cout << "Reshaping..." << std::endl;
	cv::Mat mylabels_res = mylabels.reshape(0, test_im.rows);

	//std::cout << "Saving labels..." << std::endl;
	//cv::imwrite("labels.jpg", mylabels_res * 70);

	//std::cout << "mylabels_res :: Rows: " << mylabels_res.rows << " Cols: " << mylabels_res.cols
	//	<< " Channels: " << mylabels_res.channels() << std::endl;

	// Isolate labels
	std::vector<cv::Mat> mylabels_type;
	for (int i = 0; i < 3; ++i)
	{
		//multiply by 255 as all values from isolateLabel range from 0 - 1
		mylabels_type.push_back(255*isolateLabel(mylabels, test_im.rows, i));
	}

	return mylabels_type;
}

cv::Mat isolateLabel(cv::Mat mylabel, int num_rows, int label_num)
{
	cv::Mat outmat = mylabel.clone();
	for (int i = 0; i < mylabel.rows; ++i)
	{
		if (mylabel.at<int>(i, 0) == label_num)
			outmat.at<int>(i, 0) = 1;
		else
			outmat.at<int>(i, 0) = 0;
	}

	return outmat.reshape(0, num_rows);
}