#pragma once
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/internal.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <vector>
using namespace cv;
class BackgroundSubtractorMOG2
{
public:
	void initialize(Size _frameSize, int _frameType);
	void operator()(InputArray _image, OutputArray _fgmask, double learningRate);
	void getBackgroundImage(OutputArray backgroundImage) const;
	BackgroundSubtractorMOG2();
	BackgroundSubtractorMOG2(int _history,  float _varThreshold, bool _bShadowDetection);
	~BackgroundSubtractorMOG2();


};

