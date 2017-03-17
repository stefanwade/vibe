#include <opencv2/opencv.hpp>
#include <iostream>
#include "vibe.h"

using namespace std;
using namespace cv;

int c_xoff[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};    //x的邻居点
int c_yoff[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};    //y的邻居点

ViBe_BGS::ViBe_BGS(void)
{

}

ViBe_BGS::~ViBe_BGS(void)
{

}

//************************ Assign space and init *******************************
void ViBe_BGS::init(const IplImage* _image)
{
	for (int i=0; i<NUM_SAMPLES; i++)
	{
		m_samples[i] = cvCreateImage(cvGetSize(_image), IPL_DEPTH_8U, 1);
		cvZero(m_samples[i]);
	}

	m_segmentationMask = cvCreateImage(cvGetSize(_image),IPL_DEPTH_8U,1);
	m_foregroundMatchCount = cvCreateImage(cvGetSize(_image),IPL_DEPTH_8U,1);
}

//************************ Init model from first frame *************************
void ViBe_BGS::processFirstFrame(const IplImage* _image)
{
	int row, col;

	for(int i=0; i<_image->height; i++)
	{
		for (int j=0; j<_image->width; j++)
		{	

			for (int k=0; k<NUM_SAMPLES; k++)
			{
				//Random pick up NUM_SAMPLES pixel in the neighborhood to construct the model
				int random = getRandom(0, 8);

				row = i + c_yoff[random];
				if (row < 0)
				{
					row = 0;
				}
				if (row >= _image->height)
				{
					row = _image->height - 1;
				}

				col = j + c_xoff[random];
				if (col < 0)
				{
					col = 0;
				}
				if (col >= _image->width)
				{
					col = _image->width - 1;
				}
				m_samples[k]->imageData[i*m_samples[k]->widthStep+j] = _image->imageData[row*_image->widthStep+col];
			}
		}
	}
}

//************************ Test a new frame and update model ***************************
void ViBe_BGS::testAndUpdate(const IplImage* _image)
{
	int level,step;
	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq *first_seq = NULL, *blob_seq = NULL, *prev_seq = NULL, *hole_seq = NULL, *child_seq;

	for (int i=0; i<_image->height; i++)
	{
		for (int j=0; j<_image->width; j++)
		{
			int matches(0), count(0);
			float dist;

			while (matches < MIN_MATCHES && count < NUM_SAMPLES )
			{
				dist = abs((uchar)m_samples[count]->imageData[i*m_samples[count]->widthStep+j] - (uchar)_image->imageData[i*_image->widthStep+j]);
				if (dist < RADIUS)
				{
					matches++;
				}
				count++;
			}

			if (matches >= MIN_MATCHES)
			{
				m_foregroundMatchCount->imageData[i*m_foregroundMatchCount->widthStep+j] = 0;
				m_segmentationMask->imageData[i*m_segmentationMask->widthStep+j] = 0;

				//如果一个像素是背景点， 那么它1 / defaultSubsamplingFactor的概率去更新自己的模型样本值
				int random = getRandom(0, SUBSAMPLE_FACTOR-1);
				if(random == 0)
				{
					random = getRandom(0, NUM_SAMPLES-1);
					m_samples[random]->imageData[i*m_samples[random]->widthStep+j] = _image->imageData[i*_image->widthStep+j];
				}

				//同时也有1 / defaultSubsamplingFactor的概率更新它的邻居点的模型样本值
				random = getRandom(0, SUBSAMPLE_FACTOR-1);
				if (random == 0)
				{
					int row, col;
					random = getRandom(0, 8);
					row = i + c_yoff[random];
					if (row < 0)
					{
						row = 0;
					}
					if (row >= _image->height)
					{
						row = _image->height - 1;
					}

					col = j + c_xoff[random];
					if (col < 0)
					{
						col = 0;
					}
					if (col >= _image->width)
					{
						col = _image->width - 1;
					}

					random = getRandom(0, NUM_SAMPLES-1);
					m_samples[random]->imageData[row*m_samples[random]->widthStep+col] = _image->imageData[i*_image->widthStep+j];
				}
			}
			else
			{
				m_segmentationMask->imageData[i*m_segmentationMask->widthStep+j] = 255;
				m_foregroundMatchCount->imageData[i*m_foregroundMatchCount->widthStep+j]++;
				//如果某个像素点连续N次被检测为前景，则认为一块静止区域被误判为运动，将其更新为背景点
				if (m_foregroundMatchCount->imageData[i*m_foregroundMatchCount->widthStep+j] > 50)
				{
					int random = getRandom(0, SUBSAMPLE_FACTOR-1);
					if (random == 0)
					{
						random = getRandom(0, NUM_SAMPLES-1);
						m_samples[random]->imageData[i*m_samples[random]->widthStep+j] = _image->imageData[i*_image->widthStep+j];
					}
				}
			}
		}
	}

	IplImage *tempImage = (IplImage*)cvClone(m_segmentationMask);
	IplConvKernel *p = cvCreateStructuringElementEx(2, 2, 1, 1, CV_SHAPE_RECT);
	IplConvKernel *q = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_CROSS);
	cvErode(tempImage, tempImage, p);
	cvDilate(tempImage, tempImage, q, 2);
	cvReleaseImage(&m_segmentationMask);
	m_segmentationMask = (IplImage*)cvClone(tempImage);
	cvReleaseImage(&tempImage);
}



int ViBe_BGS::getRandom(int a,int b)                   //产生随机数函数 
{
	int i;  
	i=rand()%(b-a+1)+a ;                     //产生a--a+b的随机数。
	return i;
}
