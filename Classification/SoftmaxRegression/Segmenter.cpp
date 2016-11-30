#include "stdafx.h"
#include "Segmenter.h"


Segmenter::Segmenter(std::string fileName, std::string outputFolder)
{
	params.imagePath = fileName;

	if (outputFolder == "")
		params.outputfolder = "C:\\uav-software\\segmenter_outputs\\";
	else
		params.outputfolder = outputFolder;
}

Segmenter::~Segmenter()
{
}

std::vector<cv::Mat> Segmenter::segment()
{
	cout << params.imagePath << endl;
	return segment(params.imagePath);
}

std::vector<cv::Mat> Segmenter::segment(string path)
{
	try
	{
		cv::Mat test_im = cv::imread(path, CV_LOAD_IMAGE_COLOR);
		return segment(test_im);
	}
	catch (cv::Exception &e)
	{
		cout << "Invalid path error: " << e.what() << endl;
	}

	return std::vector<cv::Mat>();
}


std::vector<cv::Mat> Segmenter::segment(cv::Mat test_im)
{
	cv::Mat luv_im;

	// if the image is too small, up-scale by 2
	if (min(test_im.cols, test_im.rows) <= params.min_image_sidelength)
		cv::resize(test_im, test_im, cv::Size(0, 0), 2.0, 2.0, 2);

	// blur, then convert to luv
	cv::medianBlur(test_im, test_im, 3);
	cv::cvtColor(test_im, luv_im, COLOR_BGR2Luv);

	cv::Mat luv_im_res = luv_im.reshape(1, test_im.rows * test_im.cols);
	luv_im_res.convertTo(luv_im_res, CV_32FC2);

	// run kmeans clustering, analyze results
	cv::Mat mylabels;
	cv::kmeans(luv_im_res, params.max_clusters, mylabels, params.kmeans_terminator, params.max_num_kmeans_iter, cv::KMEANS_RANDOM_CENTERS);

	// Look at initial labels, for testing purposes
	std::vector<cv::Mat> mylabels_type;
	for (int i = 0; i < 3; ++i)
	{
		//multiply by 255 as all values from isolateLabel range from 0 - 1
		mylabels_type.push_back(255 * isolateLabel(mylabels, test_im.rows, i));
		cv::imwrite(params.outputfolder + "initial_label" + to_string(i) + ".jpg", mylabels_type[i]);
	}

	bool try_again = true;
	int attempts = 0;
	std::pair<cv::Mat, cv::Mat> results;

	while (try_again && attempts < 2)
	{
		try_again = false;

		// analyze mylabels, look for the shape and letter.
		results = analyzeLabels(mylabels, test_im.rows);

		if (results.first.empty())
			return std::vector<cv::Mat>();

		// if the letter was not found, mask the original image with just the shape and try again
		if (results.second.empty())
		{
			//cout << "attempting one more time to retrieve letter ..." << endl;
			cv::Mat masked = luv_im.clone(), masked_res;
			for (int i = 0; i < masked.rows; ++i)
			{
				for (int j = 0; j < masked.cols; ++j)
				if (results.first.at<int>(i, j) != 1)
					masked.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			}

			cv::imwrite(params.outputfolder + "masked_im.jpg", masked);

			masked_res = masked.reshape(1, test_im.rows * test_im.cols);
			masked_res.convertTo(masked_res, CV_32FC2);

			cv::kmeans(masked_res, params.max_clusters, mylabels, params.kmeans_terminator, params.max_num_kmeans_iter, cv::KMEANS_RANDOM_CENTERS);
			try_again = true;
			++attempts;
		}
	}

	cv::imwrite(params.outputfolder + "luv_im.jpg", luv_im);
	cv::imwrite(params.outputfolder + "retrieved_shape.jpg", 255 * results.first);
	cv::imwrite(params.outputfolder + "retrieved_letter.jpg", 255 * results.second);

	cv::Mat mylabels_res = mylabels.reshape(0, test_im.rows);

	return mylabels_type;
}


std::pair<cv::Mat, cv::Mat> Segmenter::analyzeLabels(cv::Mat labels, int num_rows)
{
	cv::Mat shape, letter;

	// look for just the shape in each label, it must:
	// 1) have area > shape_area_threshold % of the image
	// 2) touch < border_threshold % of the border pixels
	// if more than 1 label has a candidate, take the one with largest area (the "winner")
	int max_area = 0;
	int win_label = -1, win_id = -1;
	std::vector<cv::Mat> analyzed_labels(3);

	for (int label = 0; label < 3; ++label)
	{
		int numblobs = 0;
		analyzed_labels[label] = isolateLabel(labels, num_rows, label);
		for (int i = 0; i < analyzed_labels[label].rows; ++i)
		{
			for (int j = 0; j < analyzed_labels[label].cols; ++j)
			{
				int rows = analyzed_labels[label].rows;
				int cols = analyzed_labels[label].cols;
				std::pair<int, int> info = floodFill(analyzed_labels[label], i, j);
				float area_ratio = float(info.first) / (rows * cols);
				float border_ratio = float(info.second) / (2 * (rows + cols) - 4);

				if (area_ratio < params.shape_area_threshold || border_ratio > params.border_threshold)
					continue;

				if (info.first > max_area)
				{
					max_area = info.first;
					win_label = label;
					win_id = 2 + i*cols + j;
				}
			}
		}
	}

	// nothing legitimate found, discard as false positive
	if (win_label == -1)
		return std::make_pair(cv::Mat(), cv::Mat());

	shape = analyzed_labels[win_label].clone();

	// retrieve the shape, set shape pixels 1
	// any other pixels associated with this label will be set to 2
	for (int i = 0; i < shape.rows; ++i)
	{
		for (int j = 0; j < shape.cols; ++j)
		{
			if (shape.at<int>(i, j) == win_id)
				shape.at<int>(i, j) = 1;
			else if (shape.at<int>(i, j) != 0)
				shape.at<int>(i, j) = 2;
		}
	}

	std::pair<cv::Mat, bool> letter_info = retrieveLetter(shape.clone(), max_area);

	if (letter_info.second)
		return std::make_pair(cv::Mat(), cv::Mat());

	letter = letter_info.first;

	// fill in the shape with this new info
	for (int i = 0; i < letter.rows; ++i)
	{
		for (int j = 0; j < letter.cols; ++j)
			if (letter.at<int>(i, j) == 1)
				shape.at<int>(i, j) = 1;
	}
	
	removeNoise(letter);
    removeNoise(shape);
	return std::make_pair(shape, letter);
}


std::pair<cv::Mat, bool> Segmenter::retrieveLetter(cv::Mat& shape, int shape_area)
{
	// for convenience, invert the whole thing : label regions will be 0, all others 1
	for (int i = 0; i < shape.rows; ++i)
	{
		for (int j = 0; j < shape.cols; ++j)
		{
			if (shape.at<int>(i, j) != 0)
				shape.at<int>(i, j) = 0;
			else
				shape.at<int>(i, j) = 1;
		}
	}

	// flood in surroundings, with fill_id = -1
	int area, edge_touches;
	for (int i = 0; i < shape.rows; ++i)
	{
		floodFill(shape, area, edge_touches, i, 0, -1);
		floodFill(shape, area, edge_touches, i, shape.cols - 1, -1);
	}

	for (int i = 0; i < shape.cols; ++i)
	{
		floodFill(shape, area, edge_touches, 0, i, -1);
		floodFill(shape, area, edge_touches, shape.rows - 1, i, -1);
	}

	// look for letter to flood 
	float noise_ratio;
	int noise_area;
	int numblobs = 0, total_area = 0;
	int max_area = 0, win_id = -1;
	for (int i = 0; i < shape.rows; ++i)
	{
		for (int j = 0; j < shape.cols; ++j)
		{
			area = 0, edge_touches = 0;
			int fill_id = 2 + i*shape.rows + j;
			floodFill(shape, area, edge_touches, i, j, fill_id);
			float area_ratio = float(area) / shape_area;
			total_area += area;

			if (area > 0)
				++numblobs;

			if (area_ratio > params.letter_area_threshold && area > max_area)
			{
				max_area = area;
				win_id = fill_id;
			}
		}
	}

	noise_area = total_area - max_area;
	noise_ratio = float(noise_area) / shape_area;

	if (numblobs > params.max_blobs_allowed && noise_ratio > params.letter_area_threshold)
		return std::make_pair(cv::Mat(), true);

	if (win_id == -1)
		return std::make_pair(cv::Mat(), false);


	// retrieve the letter
	cv::Mat letter = shape.clone();
	for (int i = 0; i < letter.rows; ++i)
	{
		for (int j = 0; j < letter.cols; ++j)
		{
			if (letter.at<int>(i, j) == win_id)
				letter.at<int>(i, j) = 1;
			else
				letter.at<int>(i, j) = 0;
		}
	}

	return std::make_pair(letter, false);
}

std::pair<int, int> Segmenter::floodFill(cv::Mat& canvas, int i, int j)
{
	int area = 0, edge_touches = 0;

	// fill_ids must be unique for each fill and != 0 and != 1
	if (canvas.at<int>(i, j) == 1)
	{
		int fill_id = 2 + i*canvas.cols + j;
		floodFill(canvas, area, edge_touches, i, j, fill_id);
	}
	
	return std::make_pair(area, edge_touches);
}


void Segmenter::floodFill(cv::Mat& canvas, int& area, int& edge_touches, int i_seed, int j_seed, int fill_id)
{
	if (canvas.at<int>(i_seed, j_seed) != 1)
		return;

	canvas.at<int>(i_seed, j_seed) = fill_id;

	// use BFS
	std::queue<int> to_visit;
	to_visit.push(i_seed*canvas.cols + j_seed);

	while (!to_visit.empty())
	{
		int index = to_visit.front();
		int i = floor(index / canvas.cols);
		int j = index % canvas.cols;
		to_visit.pop();

		++area;
		if (i == 0 || i == canvas.rows - 1 || j == 0 || j == canvas.cols - 1)
			++edge_touches;

		for (int dir = 0; dir < 4; ++dir)
		{
			int newi = i + cos(dir*PI/2);
			int newj = j + sin(dir*PI/2);
			if (newi < 0 || newi >= canvas.rows || newj < 0 || newj >= canvas.cols)
				continue;
			if (canvas.at<int>(newi, newj) == 1)
			{
				canvas.at<int>(newi, newj) = fill_id;
				to_visit.push(newi*canvas.cols + newj);
			}	
		}
	}
}


void Segmenter::removeNoise(cv::Mat& label)
{
	// turn all non-zero entries into 1
	for (int i = 0; i < label.rows; ++i)
	{
		for (int j = 0; j < label.cols; ++j)
		{
			if (label.at<int>(i, j) != 0)
				label.at<int>(i, j) = 1;
		}
	}

	// find largest piece
	int max_area = 0, win_id = -1;
	for (int i = 0; i < label.rows; ++i)
	{
		for (int j = 0; j < label.cols; ++j)
		{
			int area = 0, edge_touches = 0;
			int fill_id = 2 + i*label.cols + j;
			floodFill(label, area, edge_touches, i, j, fill_id);

			if (area > max_area)
			{
				win_id = fill_id;
				max_area = area;
			}
		}
	}

	// clean noise based on results
	for (int i = 0; i < label.rows; ++i)
	{
		for (int j = 0; j < label.cols; ++j)
		{
			if (label.at<int>(i, j) == win_id)
				label.at<int>(i, j) = 1;
			else
				label.at<int>(i, j) = 0;
		}
	}
}


cv::Mat Segmenter::isolateLabel(cv::Mat mylabel, int num_rows, int label_num)
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

