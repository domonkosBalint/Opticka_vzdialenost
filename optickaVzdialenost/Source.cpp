#include "opencv2/opencv.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv\cv.h>
#include <windows.h>
#include <math.h>


using namespace std;
using namespace cv;

const static int SENSITIVITY_VALUE = 20;

int main(int argc, char** argv) {
	//Create a black image with the size as the camera output
	Mat imgLines;
	Mat grayImage1, grayImage2;
	Mat differenceImage;
	Mat imgOriginal;
	Mat thresholdImage;
	Mat imgHSV;
	Mat imgThresholded;
	Mat imgTmp;
	Mat imgTmp2;

	Moments oMoments;

	bool bSuccess;

	double dM01;
	double dM10;
	double dArea;
	double vzdialenost;

	int posX;
	int posY;
	int iLastX = -1;
	int iLastY = -1;
	double pomer = 3.4;

	VideoCapture cap(0); //capture the video from webcam

	if (!cap.isOpened()) {   // if not success, exit program 
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	/*
	//red color settings
	int iLowH = 150;
	int iHighH = 179;

	int iLowS = 143;
	int iHighS = 255;

	int iLowV = 5;
	int iHighV = 255;
	*/


	//blue color detection settings
	int iLowH = 80;
	int iHighH = 130;

	int iLowS = 150;
	int iHighS = 255;

	int iLowV = 60;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	cap.read(imgTmp);	//temporary image from camera
	imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);

	vector<Vec3f> circles;
	int radius =0;
	Point center;

	while (true) {
		bSuccess = cap.read(imgOriginal); // read a new frame from video

		if (!bSuccess) { //if not success, break loop      
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		cvtColor(imgOriginal, grayImage1, COLOR_BGR2GRAY);
		/*
		cap.read(imgTmp2);
		cvtColor(imgTmp2,grayImage2,COLOR_BGR2GRAY);

		absdiff(grayImage1,grayImage2,differenceImage);
		threshold(differenceImage,thresholdImage,SENSITIVITY_VALUE,255,THRESH_BINARY);
		*/
		//imshow("Difference Image",differenceImage);
		//imshow("Threshold Image", thresholdImage)

		GaussianBlur(grayImage1, grayImage1, Size(9, 9), 2, 2);

		/// Apply the Hough Transform to find the circles
		HoughCircles(grayImage1, circles, CV_HOUGH_GRADIENT, 2, 20, 100, 155, 20, 300);
		waitKey(100);
		/// Draw the circles detected
		for (size_t i = 0; i < circles.size(); i++) {
			//center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			center.x = cvRound(circles[i][0]);
			center.y = cvRound(circles[i][1]);

			radius = cvRound(circles[i][2]);

			// circle center
			circle(imgOriginal, center, 3, Scalar(0, 255, 0), -1, 8, 0);
			// circle outline
			circle(imgOriginal, center, radius, Scalar(0, 0, 255), 3, 8, 0);
		}

		imshow("Hough circles", imgOriginal);

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

		//morphological opening (removes small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//morphological closing (removes small holes from the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//Calculate the moments of the thresholded image
		oMoments = moments(imgThresholded);

		dM01 = oMoments.m01;
		dM10 = oMoments.m10;
		dArea = oMoments.m00;

		// if the area <= 10000, I consider that there are no object in the image and it's because of the noise, the area is not zero 
		if (dArea > 10000) {
			//calculate the position of the ball
			posX = dM10 / dArea;
			posY = dM01 / dArea;

			if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
				//Draw a red line from the previous point to the current point
				line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0, 0, 255), 2);
				//std::cout << "X: " << (int)(iLastX-posX) << "     Y: " << (int)(iLastY-posY) << endl;   //pocitam si vzdialenost
				vzdialenost = sqrt((double)((iLastX - posX)*(iLastX - posX)) + (double)((iLastY - posY)*(iLastY - posY)));
				system("CLS");
				cout << "Vzdialenost-" << vzdialenost << endl;
				printf("X os - %f ;", posX / 20.5);
				printf("Y os - %f ;", 40.5 / posY * 100);
				printf("Z os - %f", pomer / radius * 1000);
				//Sleep(2000);
			}

			iLastX = posX;
			iLastY = posY;
		}

		imshow("Thresholded Image", imgThresholded); //show the thresholded image

		imgOriginal = imgOriginal + imgLines;
		imshow("Original", imgOriginal); //show the original image

		if (waitKey(30) == 27) {         //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop     
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;
}