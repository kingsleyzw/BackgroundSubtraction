#include "StdAfx.h"
#include "BgFg_Vibe.h"

namespace cv{

static const int defaultNbSamples = 20;//样本数太少噪点会增加
static const int defaultReqMatches = 2;
static const int defaultRadius = 20;
static const int defaultSubsamplingFactor = 32;  //可以控制遗失物的消融速度
static const int nChannels = 3;
static const int BACKGROUNG = 0;
static const int FOREGROUNG = 255;
static const int NEIGHBOR = 1;
uchar**** Samples;

int GetRandom(int istart,int iend)
{
	int val=istart+rand()%(iend-istart + 1);
	return val;
}

double euclidDist(int x0,int y0,int z0,int x1,int y1,int z1){
	return sqrtf((double)((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)+(z0-z1)*(z0-z1)));
}

struct	VibeInvoker : ParallelLoopBody{
	VibeInvoker(Mat* _image,Mat* _fgmask){
		src = _image;
		mask = _fgmask;
		nchannels = src->channels();
		cols = src->cols;
		rows = src->rows;
	}
	void operator()(const Range& range) const{ //重载操作符（）
		
		int x, y;
		for( y = range.start; y < range.end; y++ )
		{
			const uchar* p = src->datastart + cols*y*nchannels;
			uchar* m = mask->datastart + cols*y;

			for( x = 0; x < cols; x++, p += nchannels){
				int count = 0,index = 0,dist = 0;
				for(int index = 0;index<defaultNbSamples && count < defaultReqMatches;index++) {
					dist = euclidDist(p[0],p[1],p[2],Samples[x][y][index][0],Samples[x][y][index][1],Samples[x][y][index][2]);
					if(dist <defaultRadius)
						count++;
				}

				if(count>=defaultReqMatches){
					m[x]=BACKGROUNG;
					int rand = GetRandom(0,defaultSubsamplingFactor - 1);
					if(rand == 0){
						rand =  GetRandom(0,defaultNbSamples - 1);
						Samples[x][y][rand][0] =  p[0];
						Samples[x][y][rand][1] =  p[1];
						Samples[x][y][rand][2] =  p[2];
					}

					rand = GetRandom(0,defaultSubsamplingFactor - 1);
					if(rand ==0){

						int dx = GetRandom(-NEIGHBOR,NEIGHBOR);
						int dy = GetRandom(-NEIGHBOR,NEIGHBOR);

						int neighborX = x + dx;
						neighborX = (0<=neighborX && neighborX<cols)? neighborX :x;
						int neighborY = y + dy;
						neighborY = (0<=neighborY && neighborY<rows)? neighborY :y;

						int randSample =  GetRandom(0,defaultNbSamples - 1);

						Samples[neighborX][neighborY][randSample][0] =  p[0];
						Samples[neighborX][neighborY][randSample][1] =  p[1];
						Samples[neighborX][neighborY][randSample][2] =  p[2];
					}
				} 
				else{
					m[x]=FOREGROUNG;
				}

				//cout<<endl;
			}
		}
	}
	
	Mat *src, *mask;
	int rows, cols, nchannels;

};//end struct VibeInvoker

void BgFg_Vibe::initialize(const CvArr* _image){
	CvMat src, *image = cvGetMat( _image, &src );
	Samples = new uchar***[image->width]();
	for(int i=0;i<image->width;i++){
		Samples[i] = new uchar**[image->height]();
	}
	for(int i=0;i<image->width;i++){
		for(int j=0;j<image->height;j++){
			Samples[i][j] = new uchar*[defaultNbSamples]();
		}
	}
	for(int i=0;i<image->width;i++){
		for(int j=0;j<image->height;j++)
			for(int k=0;k<defaultNbSamples;k++){
				Samples[i][j][k] = new uchar[nChannels]();
			}
	}
	for( int y = 0; y < src.height; y++ )
	{
		const uchar* p = image->data.ptr + image->step*y ;

		for( int x = 0; x < src.width; x++ ,p+=nChannels)
		{	

			for(int k = 0;k<defaultNbSamples;k++) {

				int dx = GetRandom(-NEIGHBOR,NEIGHBOR);
				int dy = GetRandom(-NEIGHBOR,NEIGHBOR);

				int neighborX = x + dx;
				neighborX = (0<=neighborX && neighborX<src.width)? neighborX :x;
				int neighborY = y + dy;
				neighborY = (0<=neighborY && neighborY<src.height)? neighborY :y;

				//cout <<neighborX<<" "<<neighborY<<endl;
				const uchar* neighborP = image->data.ptr + image->step*neighborY ;
				neighborP+=neighborX*nChannels;

				Samples[x][y][k][0] = neighborP[0];
				Samples[x][y][k][1] = neighborP[1];
				Samples[x][y][k][2] = neighborP[2];
				//cout<<"---x="<<x<<" y="<<y<<" sample="<<k<<" b="<<0+p[0]<<" g="<<0+p[1]<<" r="<<0+p[2]<<endl;
				//cout<<"+++x="<<x<<" y="<<y<<" sample="<<k<<" b="<<0+*(sample+0)<<" g="<<0+*(sample+1)<<" r="<<0+*(sample+2)<<endl;
			}
		}
	}
}

void BgFg_Vibe::update(Mat* _image,Mat* _fgmask,int rows){
	int totalCols = rows;
	typedef VibeInvoker parallelTestBody;
	parallel_for_(Range(0, totalCols), parallelTestBody(_image,_fgmask));
}

}//end namespace ourVibe
