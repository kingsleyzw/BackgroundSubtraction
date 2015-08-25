
// minimalistic foreground-background segmentation sample, based off OpenCV's bgfg_segm sample
#include "stdafx.h"
#include "BackgroundSubtractorSuBSENSE.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>

#include <string>
#include <stdio.h>
#include<vector>
#include <iomanip>
#include <io.h>
#include <fstream>
#include <iostream>
#include "cv.h"
#include "highgui.h"
using namespace std;
using namespace cv;


static void help() {
    printf("\nMinimalistic example of foreground-background segmentation in a video sequence using\n"
            "OpenCV's BackgroundSubtractor interface; will analyze frames from the default camera\n"
            "or from a specified file.\n\n"
            "Usage: \n"
            "  ./bgfg_segm [--camera]=<use camera, true/false>, [--file]=<path to file> \n\n");
}

const char* keys = {
    "{c  |camera   |true     | use camera or not}"
    "{f  |file     |tree.avi | movie file path  }"
};

//查找dir路径下所有文件(dir包括配置符，如c:\*.txt)
//ishavedir表示返回的文件名是否包含全路径，true为返回全路径的文件名，false只返回文件名
vector<string> FindAllFile(const char* dir,bool ishavedir=false)
{
	_finddata_t file;
	vector<string> file_list;
	long lf;
	if((lf = _findfirst(dir, &file))==-1l) { //_findfirst返回的是long型; long __cdecl _findfirst(const char *, struct _finddata_t *)
		return file_list;
	} else {
		do {
			if (ishavedir) {
				string tmppath=dir;
				int index=tmppath.find_last_of("*.")-1;
				tmppath=tmppath.substr(0,index).append(file.name);
				file_list.push_back(tmppath);
				//cout<<file.name<<endl;
			} else {

				file_list.push_back(file.name);
			}
		} while (_findnext( lf, &file ) == 0);//int _findnext(long, struct _finddata_t *);如果找到下个文件的名字成功的话就返回0,否则返回-1
		_findclose(lf);
		return file_list;
	}
}
//注意：当字符串为空时，也会返回一个空字符串
void split(std::string& s, std::string& delim,std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index=s.find_first_of(delim,last);
	while (index!=std::string::npos)
	{
		ret->push_back(s.substr(last,index-last));
		last=index+1;
		index=s.find_first_of(delim,last);
	}
	if (index-last>0)
	{
		ret->push_back(s.substr(last,index-last));
	}
}
int main(int argc, const char** argv) {
	/*if(argc<2)
		return 1;
	const char* file = argv[1];*/

	char file[] = "F:\\video\\CDnet2014\\baseline\\baseline\\PETS2006\\input\\*.jpg";
	vector<string> vec = FindAllFile(file,true);
	vector<string>::iterator iter = vec.begin();
	string strPicNameOutput = file;
	vector<string> strPicNameInputSplit;
	split(strPicNameOutput,string("\\"),&strPicNameInputSplit);
	strPicNameInputSplit.erase(strPicNameInputSplit.end() - 1);
	strPicNameInputSplit.erase(strPicNameInputSplit.end() - 1);
	strPicNameInputSplit[strPicNameInputSplit.size() - 2] = "results";
	strPicNameOutput="";
	for (vector<string>::const_iterator iter = strPicNameInputSplit.begin();
		iter != strPicNameInputSplit.end(); iter++){
			strPicNameOutput += *iter;
			strPicNameOutput += "\\";
	}
	cout<<strPicNameOutput<<endl;
	int num = 1;
	char picName[64] = {0};
	strPicNameOutput = strPicNameOutput.append("bin%06d.png");
	IplImage* frame = cvLoadImage((*iter).c_str(),0);
	if(frame == NULL)
		return 1;

	//********************************************************************

    help();
	
    cv::Mat  oCurrSegmMask, oCurrReconstrBGImg,oCurrResizeInputFrame;
   
    oCurrSegmMask.create(cvGetSize(frame),CV_8UC1);
    oCurrReconstrBGImg.create(cvGetSize(frame),CV_8UC1);
    cv::Mat oSequenceROI(cvGetSize(frame),CV_8UC1,cv::Scalar_<uchar>(255)); // for optimal results, pass a constrained ROI to the algorithm (ex: for CDnet, use ROI.bmp)
    BackgroundSubtractorSuBSENSE oBGSAlg;
	cv::Mat oCurrInputFrame(frame);
    oBGSAlg.initialize(oCurrInputFrame,oSequenceROI);
	IplImage* writeImageFg,*writeImageBg;

	//********************************************************************
	while(iter != vec.end()){
		sprintf(picName,strPicNameOutput.c_str(),num);
		cout<<num<<"  "<<picName<<endl;
		num++;
		frame = cvLoadImage((*iter++).c_str(),0);
		if(frame == NULL)
			break;
		//********************************************************************
        oBGSAlg(oCurrInputFrame,oCurrSegmMask,100.0/*,double(k<=100)*/); // lower rate in the early frames helps bootstrap the model when foreground is present
        oBGSAlg.getBackgroundImage(oCurrReconstrBGImg);
        imshow("input",oCurrInputFrame);
        imshow("segmentation mask",oCurrSegmMask);
        imshow("reconstructed background",oCurrReconstrBGImg);
		//writeImageFg = &oCurrSegmMask.operator IplImage();
		//********************************************************************
		//cvSaveImage(picName,writeImageFg);
		cvWaitKey(1);
	}
	if(frame)
		cvReleaseImage(&frame);
    return 0;
}

