#include "StdAfx.h"
#include "bgf_ViBe.h"
#include "iostream"
#include "math.h"
using namespace std;

void bgf_ViBe::initialize(const CvArr* _image){
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

void bgf_ViBe::update(const CvArr* _image,CvArr* _fgmask){

    CvMat src, *image = cvGetMat( _image, &src );
    CvMat mstub, *mask = cvGetMat( _fgmask, &mstub );
    int x, y;

    for( y = 0; y < src.height; y++ )
    {
        const uchar* p = image->data.ptr + image->step*y ;
        uchar* m = mask->data.ptr + mask->step*y;

        for( x = 0; x < src.width; x++, p += 3){
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
					neighborX = (0<=neighborX && neighborX<src.width)? neighborX :x;
					int neighborY = y + dy;
					neighborY = (0<=neighborY && neighborY<src.height)? neighborY :y;

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


int bgf_ViBe::GetRandom(int istart,int iend)
{
	int val=istart+rand()%(iend-istart + 1);
	return val;
}

double bgf_ViBe::euclidDist(int x0,int y0,int z0,int x1,int y1,int z1){
	return sqrtf((double)((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1)+(z0-z1)*(z0-z1)));
}


