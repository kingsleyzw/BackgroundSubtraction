#include <stdafx.h>
#include <opencv2/opencv.hpp>
#include <time.h>
#include "test.hpp"
using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	Mat testInput = Mat::ones(40,400000, CV_32F);
	clock_t start, stop;

	start = clock();
	parallelTestWithFor(testInput);
	stop = clock();
	cout<<"Running time using \'for\':"<<(double)(stop - start)/CLOCKS_PER_SEC*1000<<"ms"<<endl;

	start = clock();
	parallelTestWithParallel_for(testInput);
	stop = clock();
	cout<<"Running time using \'parallel_for\':"<<(double)(stop - start)/CLOCKS_PER_SEC*1000<<"ms"<<endl;

	start = clock();
	parallelTestWithParallel_for_(testInput);
	stop = clock();
	cout<<"Running time using \'parallel_for_\':"<<(double)(stop - start)/CLOCKS_PER_SEC*1000<<"ms"<<endl;

	system("pause");
}