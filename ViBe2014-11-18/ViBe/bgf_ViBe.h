#pragma once
#include"highgui.h"
class bgf_ViBe
{
public:
	void initialize(const CvArr* _image);
	void update(const CvArr* _image,CvArr* _fgmask);
	int static bgf_ViBe::GetRandom(int istart,int iend);
	double euclidDist(int x0,int y0,int z0,int x1,int y1,int z1);

private:
	static const int defaultNbSamples = 20;//样本数太少噪点会增加
    static const int defaultReqMatches = 2;
    static const int defaultRadius = 20;
    static const int defaultSubsamplingFactor = 32;  //可以控制遗失物的消融速度
	static const int nChannels = 3;
	static const int BACKGROUNG = 0;
	static const int FOREGROUNG = 255;
	static const int NEIGHBOR = 1;
	uchar**** Samples;
};

