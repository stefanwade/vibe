#include <iostream>
#include <opencv2/opencv.hpp>

#include  "vibe.h"
#include <cstddef>
#include <stdio.h>


#define WIDTH 320
#define HEIGHT 240

using namespace std;
using namespace cv;


int main(){
	/*
	VideoCapture video("");

	if (!video.isOpened())
	{
		cout << "open video error" << endl;
		return 1;
	}

	Mat frame;
	video.read(frame);
	*/
	IplImage *tempImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	IplImage *pImage = cvCreateImageHeader(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	IplImage *mask = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	bool vibeEnable = true;

	FILE *f;
	f = fopen("1.avi", "rb");
	if (!f)
	{
		cout << "file open error" << endl;
	}
	fseek(f, 0, SEEK_END);
	int frameNumber = 0;
	frameNumber = (int)((int)ftell(f) / (WIDTH*HEIGHT * 2));

	const int videoSize = WIDTH*HEIGHT*frameNumber * 2;
	unsigned char *img = new unsigned char[videoSize];

	fseek(f, 0, SEEK_SET);
	fread(img, 1, WIDTH*HEIGHT*frameNumber * 2, f);

	ViBe_BGS vibe_bgs;

	for (int frame_index = 0; frame_index < frameNumber; frame_index++)
	{
		int offset = WIDTH*HEIGHT * 2 * frame_index;
		unsigned char *src = img + offset;
		IplImage *pImage = cvCreateImageHeader(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
		pImage->imageData = (char*)src;
		tempImage = (IplImage*)cvClone(pImage);
		cvReleaseImage(&pImage);

		if (vibeEnable)
		{
			if (frame_index == 0)
			{
				//long t = cvGetTickCount();
				vibe_bgs.init(tempImage);
				vibe_bgs.processFirstFrame(tempImage);
				//t = cvGetTickCount() - t;
				//float costTime = t/cvGetTickFrequency()/1000;
				cout << "Training VIBE complete!" << endl;
				//cout << "cost " << costTime << " ms;" <<endl;
				//cvWaitKey(-1);
			}
			else
			{
				//long t = cvGetTickCount();
				vibe_bgs.testAndUpdate(tempImage);
				//t = cvGetTickCount() - t;
				//float costTime = t/cvGetTickFrequency()/1000;
				//cout << "cost " << costTime << " ms;" <<endl;
				//cvWaitKey(-1);
				mask = vibe_bgs.getMask();
				if (frame_index <= 5)
				{
					continue;
				}
				cvNamedWindow("test");
				cvShowImage("test", mask);
				cvWaitKey(1);

			}
		}
	}

	return 0;
}