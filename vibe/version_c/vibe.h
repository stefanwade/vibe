#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

#define NUM_SAMPLES 20                 //每个像素点的样本个数
#define MIN_MATCHES 2                  //#min指数
#define RADIUS 10                      //Sqthere半径    10
#define SUBSAMPLE_FACTOR 5             //子采样概率
#define SEGMENTATIONMASK_FGBLOB_MIN 10 //前景面积最小值 5
#define SEGMENTATIONMASK_HOLE_MIN 50   //空洞面积最小值 50
#define UPDATINGMASK_HOLE_MIN 500        //空洞面积最小值

class ViBe_BGS
{
public:
	ViBe_BGS(void);
	~ViBe_BGS(void);

	void init(const IplImage *_image);            //初始化
	void processFirstFrame(const IplImage *_image);
	void testAndUpdate(const IplImage *_image);    //更新
	//void computeGradient(const IplImage *_image);         //计算梯度
	IplImage* getMask(void){return m_segmentationMask;};
	int getRandom(int a,int b);

private:
	IplImage *m_samples[NUM_SAMPLES];
	//uchar ***m_samples;
	IplImage *m_foregroundMatchCount;
	IplImage *m_segmentationMask;
};