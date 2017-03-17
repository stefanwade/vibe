#include <stdio.h>
#include <iostream>
#include <inc/processFrame.h>
#include <opencv2\opencv.hpp>
#include <inc\vibe.h>
#include <cstddef>

#define WIDTH 320       //352
#define HEIGHT 240      //288
#define HUMAN_RATIO 0.9   //(height/width)
#define MAX_HUMAN_RATIO 2.5
#define RESIZE_SCALE 1.7
#define OBJECT_AREA 230
#define STAY_ERROR_FRAME_NUM 20
//#pragma comment(lib,"D:\project\va_hule\trunk\emcv\Debug\emcv.lib")
trace_result* test_result;
pos* newBB;
CvPoint leftTopPoint = cvPoint(0,0);
CvPoint rightBottomPoint = cvPoint(0,0);
bool isCenterPointExist = false;

using namespace std;

//单独调试TLD算法时加入的鼠标回调事件函数
void onmouse(int event, int x, int y, int flags, void *param)
{
	IplImage *tempImage = NULL;
	CvPoint ptr1 = cvPoint(0,0);
	static bool moveControl = false;

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		ptr1 = cvPoint(x,y);
		leftTopPoint = ptr1;
		moveControl = true;
	}
	if (event == CV_EVENT_MOUSEMOVE && moveControl)
	{
		tempImage = (IplImage*)cvClone(param);
		CvPoint ptr2 = cvPoint(x,y);
		cvRectangle(tempImage, leftTopPoint, ptr2, CV_RGB(255, 255, 255));
		cvShowImage("test", tempImage);
		cvReleaseImage(&tempImage);
	}
	if (event == CV_EVENT_RBUTTONDOWN)
	{
		tempImage = (IplImage*)cvClone(param);
		CvPoint ptr2 = cvPoint(x,y);
		cvRectangle(tempImage, leftTopPoint, ptr2, CV_RGB(255, 255, 255));
		cvShowImage("test", tempImage);
		cvReleaseImage(&tempImage);
		moveControl = false;
		rightBottomPoint = ptr2;
	}
}

//检查VIBE算法的输出图像中是否有满足TLD跟踪条件的前景区域
bool isMaskExist(IplImage *mask)
{
	int centerX, centerY;
	vector<Rect> BBox;

	IplImage *tempMask = NULL;
	tempMask = (IplImage *)cvClone(mask);
	CvScalar sum = cvSum(tempMask);
	if (sum.val[0] == 0)
	{
		cvReleaseImage(&tempMask);
		return false;
	}

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = NULL;
	cvFindContours(tempMask, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
	for (; contour != NULL; contour = contour->h_next)
	{
		Rect tempRect = cvBoundingRect(contour, 0);
		if (tempRect.height/(float)tempRect.width >= HUMAN_RATIO && tempRect.area()>= OBJECT_AREA)
		{
			BBox.push_back(tempRect);
		}
		else continue;
	}

	//if (BBox.size() >= 1)
	for(int i = 0; i<BBox.size(); i++)
	{
		//cout << "object exist" << endl;
		//cout << "x: " << BBox[i].x << " y: " << BBox[i].y << " width: " << BBox[i].width << " height: " << BBox[i].height << endl;
		if ((BBox[i].x - 5 <= 0) || (BBox[i].y - 5 <= 0) ||
			(tempMask->width - BBox[i].x - BBox[i].width <= 5) ||
			(tempMask->height - BBox[i].y - BBox[i].height <= 5))
		{
			continue;
		}
		else
		{
			centerX = BBox[i].x + (BBox[i].width/2);
			centerY = BBox[i].y + (BBox[i].height/2);
			int newWidth = (int)(BBox[i].width / RESIZE_SCALE);
			int newHeight = (int)(BBox[i].height / RESIZE_SCALE);
			BBox[i].x = centerX - newWidth/2;
			BBox[i].y = centerY - newHeight/2;
			BBox[i].width = newWidth;
			BBox[i].height = newHeight;
			leftTopPoint = cvPoint(BBox[i].x, BBox[i].y);
			rightBottomPoint = cvPoint(BBox[i].x+BBox[i].width, BBox[i].y+BBox[i].height);
			cvReleaseMemStorage(&storage);
			cvReleaseImage(&tempMask);
			return true;
		}
	}

	cvReleaseMemStorage(&storage);
	cvReleaseImage(&tempMask);
	return false;
}

//对于每一帧的跟踪结果进行判断是否跟踪出错
bool errorDetect(trace_result *test_result)
{
	static int stayCount = 0;
	static CvPoint tempCenterPoint;
	CvPoint centerPoint;
	centerPoint.x     = test_result->object_pos[0].x+test_result->object_pos[0].width/2;
	centerPoint.y     = test_result->object_pos[0].y+test_result->object_pos[0].height/2;

	if (centerPoint.x > 0 && centerPoint.y > 0)
	{
		if ((float)test_result->object_pos[0].height/(float)test_result->object_pos[0].width >= MAX_HUMAN_RATIO)
		{
			//ratioError = true;
			cout << "object ratio error!!!" << endl;
			return true;
			//status = -1;
		}

		if (!isCenterPointExist)
		{
			tempCenterPoint = centerPoint;
			isCenterPointExist = true;
		}
		else
		{
			if (centerPoint.x == tempCenterPoint.x && centerPoint.y == tempCenterPoint.y)
			{
				stayCount++;
			}
			else
			{
				stayCount = 0;
				tempCenterPoint = centerPoint;
			}

			if (stayCount == STAY_ERROR_FRAME_NUM)
			{
				//stayError = true;
				cout << "stay long time error!!!" << endl;
				cout << "stayCount: " << stayCount << endl;
				stayCount = 0;
				return true;
			}
		}

		if ((test_result->object_pos[0].x - 5 <= 0) || (test_result->object_pos[0].y - 5 <= 0) ||
			(WIDTH - test_result->object_pos[0].x - test_result->object_pos[0].width <= 5) ||
			(HEIGHT - test_result->object_pos[0].y - test_result->object_pos[0].height <= 5))
		{
			cout << "object reach edge error!!! from function" << endl;
			cout << test_result->object_pos[0].x << "  " << test_result->object_pos[0].y << "  " << test_result->object_pos[0].width << "  " << test_result->object_pos[0].height << endl;
			//cvWaitKey(-1);
			return true;
			//edgeError = true;
		}
	}
	return false;
}

int main()
{
	int status = 3;
	int stayCount = 0;
	bool vibeEnable  = true;
	bool tldEnable   = false;
	bool tldInitFlag = true;
	bool isTldInit   = false;
	//bool stayError   = false;
	//bool edgeError   = false;
	//bool ratioError  = false;
	bool trackError  = false;
	CvPoint tempCenterPoint;
	FILE *f;
	IplImage *pImage = cvCreateImageHeader(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	IplImage *mask   = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	IplImage *tempImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	//CvFont font;
	//f = fopen("C:\\Users\\luyi\\Desktop\\bottle.yuv", "rb");E:\视频序列
	f = fopen("F:\\Research\\Dataset\\sequence\\test.yuv", "rb");
	if (!f)
	{
		cout << "file open error!" << endl;
	}

	fseek(f, 0, SEEK_END);
	int frameNumber = 0;
	frameNumber = (int)((int)ftell(f)/(WIDTH*HEIGHT*2));
	cout << "frame number is " << frameNumber << endl;

	const int videoSize = WIDTH*HEIGHT*frameNumber*2;
	unsigned char *img = new unsigned char[videoSize];

	fseek(f, 0, SEEK_SET);
	fread(img, 1, WIDTH*HEIGHT*frameNumber*2, f);

	test_result = new trace_result;
	newBB = new pos;
	ViBe_BGS vibe_bgs;

	if(test_result == NULL)
	{
		printf("tld--->debug:no enough memory! malloc fail!\n");
		return -1;
	}
	TLD_Init(WIDTH,HEIGHT);
	int frame_index;
	for(frame_index = 0; frame_index <frameNumber; frame_index ++)
	{
		int offset = WIDTH*HEIGHT*2*frame_index;
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
				cout<<"Training VIBE complete!"<<endl;
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
				tldEnable = isMaskExist(mask);
				if (tldEnable)
				{
					cout<<"begin tld!"<<endl;
					vibeEnable = false;
				}
			}
		}

		if (tldEnable)
		{
			if (tldInitFlag)
			{
				//long t = cvGetTickCount();
				//TLD_Init(WIDTH,HEIGHT);
				//t = cvGetTickCount() - t;
				//float costTime = t/cvGetTickFrequency()/1000;
				//cout << "cost " << costTime << " ms;" <<endl;
				//cvWaitKey(-1);
				//test_result = new trace_result;
				isTldInit = true;
				newBB->x = leftTopPoint.x;
				newBB->y = leftTopPoint.y;
				newBB->width = rightBottomPoint.x-leftTopPoint.x;
				newBB->height = rightBottomPoint.y-leftTopPoint.y;
				tldInitFlag = false;
				//cvWaitKey(0);
			}
			else
			{
				newBB->width = 0;
			}
			//long t = cvGetTickCount();
			status = TLD_Process(src,newBB,test_result );
			//t = cvGetTickCount() - t;
			//float costTime = t/cvGetTickFrequency()/1000;
			//cout << "cost " << costTime << " ms;" <<endl;
			//cvWaitKey(-1);
			if (status == -1)
			{
				trackError = true;
			}
			printf("TLD status is : %d ...!!!\n", status);
			printf("Startx: %d, Starty: %d, Width: %d, Height: %d\n", test_result->object_pos[0].x, test_result->object_pos[0].y,test_result->object_pos[0].width, test_result->object_pos[0].height);
		}

		//还原跟踪后的方框大小
		CvRect  tempRect;
		CvPoint centerPoint;
		tempRect.width    = (int)(test_result->object_pos[0].width*RESIZE_SCALE);
		tempRect.height   = (int)(test_result->object_pos[0].height*RESIZE_SCALE);
		centerPoint.x     = test_result->object_pos[0].x+test_result->object_pos[0].width/2;
		centerPoint.y     = test_result->object_pos[0].y+test_result->object_pos[0].height/2;
		tempRect.x        = centerPoint.x - tempRect.width/2;
		tempRect.y        = centerPoint.y - tempRect.height/2;
		//cout << "width1: " << test_result->object_pos[0].width << " height1: " << test_result->object_pos[0].height <<endl;
		//cout << "width2: " << tempRect.width << " height2: " << tempRect.height <<endl;
		//cvWaitKey(-1);

		//对跟踪结果画框标注
		CvFont font;
		cvInitFont( &font, CV_FONT_HERSHEY_SIMPLEX, .5f, .5f, 0, 1, 8);
		IplImage *tempFrame = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
		tempFrame = (IplImage*)cvClone(tempImage);
		cvPutText(tempFrame, "object 1", cvPoint(tempRect.x-30, tempRect.y-10), &font, CV_RGB(255,255,255));
		cvRectangle(tempFrame, cvPoint(tempRect.x,tempRect.y), 
			cvPoint(tempRect.x+tempRect.width,tempRect.y+tempRect.height),
			CV_RGB(255,255,255));
		//cvRectangle(tempFrame, cvPoint(test_result->object_pos[0].x,test_result->object_pos[0].y), 
		//	cvPoint(test_result->object_pos[0].x+test_result->object_pos[0].width,test_result->object_pos[0].y+test_result->object_pos[0].height),
		//	CV_RGB(255,255,255));
		cvNamedWindow("result");
		char keyWord = cvWaitKey(10);
		switch (keyWord)
		{
		case 'q':
			exit(0);
		case 's':
			cvWaitKey(0);
			break;
		default:
			break;
		}
		cvShowImage("result", tempFrame);
		cvReleaseImage(&tempFrame);
		cvReleaseImage(&tempImage);

		//错误判断流程
		//if (centerPoint.x > 0 && centerPoint.y > 0)
		//{
		//	if ((float)test_result->object_pos[0].height/(float)test_result->object_pos[0].width >= MAX_HUMAN_RATIO)
		//	{
		//		ratioError = true;
		//		//status = -1;
		//	}

		//	if (!isCenterPointExist)
		//	{
		//		tempCenterPoint = centerPoint;
		//		isCenterPointExist = true;
		//	}
		//	else
		//	{
		//		if (centerPoint.x == tempCenterPoint.x && centerPoint.y == tempCenterPoint.y)
		//		{
		//			stayCount++;
		//		}
		//		else
		//		{
		//			stayCount = 0;
		//			tempCenterPoint = centerPoint;
		//		}

		//		if (stayCount == STAY_ERROR_FRAME_NUM)
		//		{
		//			stayError = true;
		//			stayCount = 0;
		//		}
		//	}

		//	if ((test_result->object_pos[0].x - 5 <= 0) || (test_result->object_pos[0].y - 5 <= 0) ||
		//		(WIDTH - test_result->object_pos[0].x - test_result->object_pos[0].width <= 5) ||
		//		(HEIGHT - test_result->object_pos[0].y - test_result->object_pos[0].height <= 5))
		//	{
		//		edgeError = true;
		//	}
		//}

		//if (ratioError || stayError || edgeError || trackError)
		if (errorDetect(test_result) || trackError)
		{
			vibeEnable = true;
			tldEnable = false;
			tldInitFlag = true;
			if (isTldInit)
			{
				//long t = cvGetTickCount();
				//TLD_Delete();
				//t = cvGetTickCount() - t;
				//float costTime = t/cvGetTickFrequency()/1000;
				//cout << "cost " << costTime << " ms;" <<endl;
				//cvWaitKey(-1);
				//delete test_result;
				isTldInit = false;
				test_result->object_pos->height = 0;
				test_result->object_pos->width  = 0;
				test_result->object_pos->x      = 0;
				test_result->object_pos->y      = 0;
				//stayError = false;
				//edgeError = false;
				//ratioError = false;
				trackError = false;
				cvWaitKey(-1);
			}
		}
		cout<<"No."<< frame_index << " frame end!" << endl;
	}
	//cvWaitKey(-1);
	fclose(f);
	TLD_Delete();
	cvDestroyWindow("test");
	//cvReleaseImage(&pImage);
	delete []img;
	delete test_result;
	delete newBB;

	return 0;
}

