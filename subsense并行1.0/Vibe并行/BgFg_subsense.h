#pragma once
#include"highgui.h"
#include <opencv2/core/internal.hpp>
namespace cv{
class BgFg_Vibe
{
public:
	void show(InputArray _src);
	void initialize(const CvArr* _image,int _nchannels);
	void update(Mat* _image,Mat* _fgmask,int rows);
	/*int static GetRandom(int istart,int iend);
	double static euclidDist(int x0,int y0,int z0,int x1,int y1,int z1);*/
	

};
}

