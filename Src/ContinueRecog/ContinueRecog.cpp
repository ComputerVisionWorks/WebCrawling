
#include "pch.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include "../WebCrawl/webcrawl.h"

#pragma comment(lib, "opencv_core340d.lib")
#pragma comment(lib, "opencv_imgproc340d.lib")
#pragma comment(lib, "opencv_imgcodecs340d.lib")

using namespace std;
using namespace cv;


int main()
{
	//const RECT offsetR = { 10, 362, 277, 419 };
	const RECT offsetR = { 58, 485, 439, 575 };
	//const RECT offsetR = { 58, 680, 439, 782};
	//const RECT offsetR = { 10, 560, 439, 623 };

	webcrawl::cTesseract tess;
	if (!tess.Create())
	{
		cout << "error tesseract" << endl;
		return 0;
	}

	int count = 0;
	HWND hwnd = NULL;
	webcrawl::cCapture capture;
	while (count < 10)
	{
		hwnd = FindWindowEx(0, hwnd, L"Chrome_WidgetWin_1", NULL);
		if (!hwnd)
			break;
		if (!IsWindowVisible(hwnd))
			continue;

		RECT rc;
		GetWindowRect(hwnd, &rc);
		const int w = rc.right - rc.left;
		const int h = rc.bottom - rc.top;

		string imPath = "test.bmp";
		capture.Capture(rc.left + offsetR.left
			, rc.top + offsetR.top
			, (offsetR.right - offsetR.left)
			, (offsetR.bottom - offsetR.top)
			, imPath.c_str());

		Mat im = cv::imread(imPath, IMREAD_COLOR);
		string outText = tess.GetText(im.data, im.cols, im.rows, 3, im.step);
		cout << "Recognition >> " << endl;
		cout << outText << endl;
		Sleep(1000);

		++count;
		hwnd = NULL;
	}
}
