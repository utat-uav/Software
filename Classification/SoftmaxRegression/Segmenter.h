#pragma once
#include "stdafx.h"

class Segmenter
{
public:
	Segmenter();
	~Segmenter();
	std::vector<cv::Mat> segment(string path);
};

//used in imageSegment
cv::Mat isolateLabel(cv::Mat mylabel, int num_rows, int label_num);
