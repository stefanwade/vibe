#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

#define NUM_SAMPLES 20                 //ÿ�����ص����������
#define MIN_MATCHES 2                  //#minָ��
#define RADIUS 10                      //Sqthere�뾶    10
#define SUBSAMPLE_FACTOR 5             //�Ӳ�������
#define SEGMENTATIONMASK_FGBLOB_MIN 10 //ǰ�������Сֵ 5
#define SEGMENTATIONMASK_HOLE_MIN 50   //�ն������Сֵ 50
#define UPDATINGMASK_HOLE_MIN 500        //�ն������Сֵ

class ViBe_BGS
{
public:
	ViBe_BGS(void);
	~ViBe_BGS(void);

	void init(const IplImage *_image);            //��ʼ��
	void processFirstFrame(const IplImage *_image);
	void testAndUpdate(const IplImage *_image);    //����
	//void computeGradient(const IplImage *_image);         //�����ݶ�
	IplImage* getMask(void){return m_segmentationMask;};
	int getRandom(int a,int b);

private:
	IplImage *m_samples[NUM_SAMPLES];
	//uchar ***m_samples;
	IplImage *m_foregroundMatchCount;
	IplImage *m_segmentationMask;
};