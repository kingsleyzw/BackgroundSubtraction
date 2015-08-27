#include "StdAfx.h"
#include "Bgfg_subsense.h"

namespace cv{

struct BgDesc{
	ushort uIntreLBSP;
	BgDesc(){
		uIntreLBSP = 0;
	}
};
static const int defaultNbSamples = 20;//样本数太少噪点会增加
static const int defaultReqMatches = 2;
static const int defaultRadius = 20;
static const int defaultTLBSP = 20;
static const int defaultSubsamplingFactor = 32;  //可以控制遗失物的消融速度
static int nChannels;
static const int BACKGROUNG = 0;
static const int FOREGROUNG = 255;
static const int NEIGHBOR = 1;
static const size_t PATCH_SIZE = 5;
static const size_t DESC_SIZE = 2;
uchar**** Samples;
BgDesc *** BgModel;

int GetRandom(int istart,int iend)
{
	int val=istart+rand()%(iend-istart + 1);
	return val;
}

double euclidDist(int x0,int y0,int z0,int x1,int y1,int z1){
	return sqrtf((double)((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)+(z0-z1)*(z0-z1)));
}
int euclidDist(int x,int y){
	return fabsf(x-y);
}
template<typename T> static inline typename std::enable_if<std::is_integral<T>::value,size_t>::type L1dist(T a, T b) {
	return (size_t)fabsf((int)a-b);
}
static const uchar popcount_LUT8[256] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};
template<typename T> static inline size_t popcount(T x) {
	size_t nBytes = sizeof(T);
	size_t nResult = 0;
	for(size_t l=0; l<nBytes; ++l)
		nResult += popcount_LUT8[(uchar)(x>>l*8)];
	return nResult;
}
template<typename T> static inline T cmixdist(T oL1Distance, T oCDistortion) {
	return (oL1Distance/2+oCDistortion*4);
}
template<size_t nChannels, typename T> static inline typename std::enable_if<std::is_floating_point<T>::value,float>::type cmixdist(const T* curr, const T* bg) {
	return cmixdist(L1dist<nChannels>(curr,bg),cdist<nChannels>(curr,bg));
}
template<typename T> static inline size_t hdist(T a, T b) {
	return popcount(a^b);
}
inline static void computeGrayscaleDescriptor(const cv::Mat& oInputImg, const uchar _ref, const int _x, const int _y, const size_t _t, ushort& _res) {
	CV_Assert(!oInputImg.empty());
	CV_Assert(oInputImg.type()==CV_8UC1);
	CV_Assert(DESC_SIZE==2); // @@@ also relies on a constant desc size
	CV_Assert(_x>=(int)PATCH_SIZE/2 && _y>=(int)PATCH_SIZE/2);
	CV_Assert(_x<oInputImg.cols-(int)PATCH_SIZE/2 && _y<oInputImg.rows-(int)PATCH_SIZE/2);
	const size_t _step_row = oInputImg.step.p[0];
	const uchar* const _data = oInputImg.data;
#include "LBSP_16bits_dbcross_1ch.i"
}

struct	VibeInvoker : ParallelLoopBody{
	VibeInvoker(Mat* _image,Mat* _fgmask){
		src = _image;
		mask = _fgmask;
		cols = src->cols;
		rows = src->rows;
	}
	void operator()(const Range& range) const{ //重载操作符（）
		
		int x, y;
		ushort uCurrIntraLBSP,uCurrInterLBSP;
		if(nChannels == 1){
			for( y = range.start; y < range.end; y++ )
			{
				const uchar* p = src->datastart + cols*y*nChannels;
				uchar* m = mask->datastart + cols*y;
				for( x = 0; x < cols; x++, p += nChannels){
					int count = 0,index = 0,dist = 0;
					//****************start*******匹配规则******start*****************
					computeGrayscaleDescriptor(*src,p[0],x,y,defaultTLBSP,uCurrIntraLBSP);
					for(int index = 0;index<defaultNbSamples && count < defaultReqMatches;index++) {

						const ushort& uBGIntraLBSP = (ushort)BgModel[x][y][index].uIntreLBSP;  //前一帧的LBSP
						const size_t nIntraLBSPDist = hdist<ushort>(uCurrIntraLBSP,uBGIntraLBSP);
						computeGrayscaleDescriptor(*src,Samples[x][y][index][0],x,y,defaultTLBSP,uCurrInterLBSP);
						const size_t nInterLBSPDist = hdist(uCurrInterLBSP,uBGIntraLBSP);
						const size_t nLBSPDist = (nIntraLBSPDist+nInterLBSPDist)/2;    //(interLBSP + intraLBSP)/2

						dist = euclidDist(p[0],Samples[x][y][index][0]);
						if(dist <defaultRadius)
							count++;
					}
					//****************end*******匹配规则******end*****************

					//****************start*******更新背景样本******start*****************
					if(count>=defaultReqMatches){
						m[x]=BACKGROUNG;
						int rand = GetRandom(0,defaultSubsamplingFactor - 1);
						if(rand == 0){
							rand =  GetRandom(0,defaultNbSamples - 1);
							Samples[x][y][rand][0] =  p[0];
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
						}
						//****************end*******更新背景样本******end*****************
					} 
					else{
						m[x]=FOREGROUNG;
					}
					
					//cout<<endl;
				}//end x
			}//end y
		}//end nChannels 1
		else if(nChannels == 3){
			for( y = range.start; y < range.end; y++ )
			{
				const uchar* p = src->datastart + cols*y*nChannels;
				uchar* m = mask->datastart + cols*y;

				for( x = 0; x < cols; x++, p += nChannels){
					int count = 0,index = 0,dist = 0;
					//****************start*******匹配规则******start*****************
					for(int index = 0;index<defaultNbSamples && count < defaultReqMatches;index++) {
						dist = euclidDist(p[0],p[1],p[2],Samples[x][y][index][0],Samples[x][y][index][1],Samples[x][y][index][2]);
						if(dist <defaultRadius)
							count++;
					}
					//****************end*******匹配规则******end*****************
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
				}//end x
			}//end y
		}//end nChannels 3
	}//end operator
	Mat *src, *mask;
	int rows, cols;

};//end struct VibeInvoker

void BgFg_Vibe::initialize(const CvArr* _image, int _nchannels){
	CvMat src, *image = cvGetMat( _image, &src );
	nChannels = _nchannels;
	Samples = new uchar***[image->width]();
	BgModel = new BgDesc**[image->width]();
	for(int i=0;i<image->width;i++){
		Samples[i] = new uchar**[image->height]();
		BgModel[i] = new BgDesc*[image->height]();
	}
	for(int i=0;i<image->width;i++){
		for(int j=0;j<image->height;j++){
			Samples[i][j] = new uchar*[defaultNbSamples]();
			BgModel[i][j] = new BgDesc[defaultNbSamples]();
		}
	}
	for(int i=0;i<image->width;i++){
		for(int j=0;j<image->height;j++)
			for(int k=0;k<defaultNbSamples;k++){
				Samples[i][j][k] = new uchar[nChannels]();
				//BgModel[i][j][k] = new BgDesc();
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
				if(nChannels == 1){
					Samples[x][y][k][0] = *neighborP;
				}else if (nChannels == 3){
					Samples[x][y][k][0] = neighborP[0];
					Samples[x][y][k][1] = neighborP[1];
					Samples[x][y][k][2] = neighborP[2];
				}
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
