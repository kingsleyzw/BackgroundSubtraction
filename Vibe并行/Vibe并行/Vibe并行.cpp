// Vibe并行.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <time.h>
#include "BgFg_Vibe.h"
#include<vector>
#include <iomanip>
#include <io.h>
#include <fstream>
using namespace cv;
using namespace std;
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
int main(){
	char file[] = "F:\\video\\CDnet2014\\baseline\\baseline\\highway\\input\\*.jpg";
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
	int frameNum = 1;
	char picName[64] = {0};
	strPicNameOutput = strPicNameOutput.append("bin%06d.png");
	IplImage* frame = NULL; 


	//********************************************************************
	
	BgFg_Vibe* viBe = new BgFg_Vibe();
	Mat Imask,Input;
	
	//********************************************************************
	Mat input;
	//clock_t start1, stop1;
	double time1=0.0;
	while(iter != vec.end()){
		sprintf(picName,strPicNameOutput.c_str(),frameNum);
		cout<<picName<<endl;
		frame = cvLoadImage((*iter++).c_str(),1);
		if(frame == NULL)
			break;
		//********************************************************************

		cvCvtColor(frame,frame,CV_RGB2YUV);//与RGB相比，转化为YUV后会明显减少噪点并且减少阴影
		if( frameNum == 1 ){
			viBe->initialize(frame);
			Imask= Mat(cvCreateImage( cvGetSize(frame), IPL_DEPTH_8U, 1 ));
		}
		Input = Mat(frame);
		//start1 = clock();
		viBe->update(&Input,&Imask,frame->height);
		//stop1 = clock();
		//time1 += (double)(stop1 - start1)/CLOCKS_PER_SEC*1000;
		imshow("Imask",Imask);
		cvWaitKey(1);
	    //********************************************************************
		//cvSaveImage(picName,frame);
		if(frame)
			cvReleaseImage(&frame);
		frameNum++;
	}
	//cout<<frameNum<<"  time="<<time1/frameNum<<endl;
	
	return 0;
}

