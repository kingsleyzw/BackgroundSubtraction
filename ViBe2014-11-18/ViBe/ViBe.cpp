// ViBe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
// ViBe.cpp : Defines the entry point for the console application.
//

#include"bgf_ViBe.h"
#include"iostream"
#include"highgui.h"
#include"cv.h"
#include<string.h>

using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	

	bgf_ViBe* viBe = new bgf_ViBe();
    int nframesToLearnBG = 300;
    //string filename = "E:\\video\\Invasion014_1.avi";
	string filename = "F:\\video\\CDnet2014\\baseline\\baseline\\highway.avi";
	//string filename = "E:\\video\\警戒区检测.avi";
	//string filename = "E:\\video\\hall.avi";

    IplImage* rawImage = 0; //yuvImage is for codebook method
    IplImage *Imask = 0;
    CvCapture* capture = 0;

    int c, n, nframes = 0;

    capture = cvCreateFileCapture( filename.c_str() );
 

    if( !capture )
    {
        printf( "Can not initialize video capturing\n\n" );
        return -1;
    }
	 rawImage = cvQueryFrame( capture );
	 //IplImage* src = cvCreateImage(cvSize(352,288),rawImage->depth,rawImage->nChannels);
	 IplImage* src = cvCreateImage(cvGetSize(rawImage),rawImage->depth,rawImage->nChannels);
	double fps = /*cvGetCaptureProperty(pCapture,CV_CAP_PROP_FPS)*/25;
	CvVideoWriter *writer =cvCreateVideoWriter(
		"F:\\video\\snow-VIBE.avi",
		CV_FOURCC('M','J','P','G'),
		fps,
		cvGetSize(src),
		0); 
    //MAIN PROCESSING LOOP:
	clock_t start1, stop1;
	double time1=0.0;
    for(;rawImage;)
    {
      
       
		
		//cvResize(rawImage,src,CV_INTER_LINEAR);//将视频大小转换成352*288标准大小.
		src = cvCloneImage(rawImage);
		cvShowImage("src",src);
		cvCvtColor(src,src,CV_RGB2YUV);//与RGB相比，转化为YUV后会明显减少噪点并且减少阴影
        ++nframes;
		
        //First time:
        if( nframes == 1 && src )
        {
			viBe->initialize(src);
            Imask= cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
            //cvSet(Imask,cvScalar(255));

            cvNamedWindow( "src", 1 );
            cvNamedWindow( "Foreground",1);
        }

        // If we've got an rawImage and are good to go:
        if( src )
        {
			start1 = clock();
			viBe->update(src,Imask);
			stop1 = clock();
			time1 += (double)(stop1 - start1)/CLOCKS_PER_SEC*1000;
           
        }
		cvWriteFrame(writer,Imask);
		cvShowImage("Foreground",Imask);
		 rawImage = cvQueryFrame( capture );
        // User input:
        c = cvWaitKey(1);
   
	}
	cvReleaseVideoWriter(&writer);
    cvReleaseCapture( &capture );
    cvDestroyWindow( "src" );
    cvDestroyWindow( "Foreground");
 
	cout<<nframes<<"  "<<time1/nframes<<endl;
	/*for(int i = 0;i < 100;i++)
		cout<<bgf_ViBe::GetRandom(-1, 1)<<endl;*/

	return 0;
}



