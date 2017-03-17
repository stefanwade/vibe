#include <iostream>
#include <opencv2/opencv.hpp>
#include "vibe.hpp"

using namespace std;
using namespace cv;
using namespace vibe;

int main(){
	VideoCapture video("1.avi");
	Mat frame;
	video.read(frame);
	Mat frame_gray;
	cvtColor(frame, frame_gray, CV_BGR2GRAY);

	VIBE vibe;
	vibe.init(frame_gray);

	while (video.read(frame)){
		cvtColor(frame, frame_gray, CV_BGR2GRAY);
		vibe.update(frame_gray);
		Mat mask = vibe.getMask();

		/*
		imshow("vibe", mask);
		int k = cvWaitKey(1);
		if (k == 27) break;
		*/

		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(mask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));

		//Mat contoursImage(frame.rows, frame.cols, CV_8U, Scalar(255));
		cout << contours.size() << endl;
		for (int i = 0; i<contours.size(); i++){
			if (hierarchy[i][3] != -1) drawContours(frame, contours, i, Scalar(0), 3);
		}
		imshow("mask", frame);
		int k = cvWaitKey(1);
		if (k == 27) break;
		cin.get();
	}
	
	return 0;
}