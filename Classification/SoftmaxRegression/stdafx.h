// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>
#include <zbar\Image.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
#include <cstdio>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <algorithm>
#include <amp.h>

#define LABEL_SIZE 62
#define DATA_SIZE 784
#define TRAINING_STEP 0.003
#define MIN_TRAIN_TIME 6000
#define TARGET_CROSS_ENTROPY 0.015
#define INFINITY 100000
#define BATCH_SIZE 401

using namespace concurrency;
using namespace cv;
using namespace std;
using namespace zbar;