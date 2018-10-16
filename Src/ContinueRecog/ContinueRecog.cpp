
#include "pch.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include "../TessOcr/tessocr.h"

#pragma comment(lib, "opencv_core340d.lib")
#pragma comment(lib, "opencv_imgproc340d.lib")
#pragma comment(lib, "opencv_imgcodecs340d.lib")

using namespace std;
using namespace cv;


int main()
{
	tessocr::cTesseract tess;
	if (!tess.Create())
	{
		cout << "error tesseract" << endl;
		return 0;
	}

	int count = 0;
	HWND hwnd = NULL;
	tessocr::cCapture capture;
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
		capture.Capture(rc.left + 10
			, rc.top + 485
			, 451
			, 90
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
