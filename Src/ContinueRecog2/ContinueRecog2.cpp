//
// chrome tab moving capture and text recognition
//
#include "pch.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "../WebCrawl/webcrawl.h"

#pragma comment(lib, "opencv_core340d.lib")
#pragma comment(lib, "opencv_imgproc340d.lib")
#pragma comment(lib, "opencv_imgcodecs340d.lib")

using namespace std;
using namespace cv;

webcrawl::cCapture g_capture;
webcrawl::cTesseract g_tess;

struct sCrawlData {
	string name;
	string data;
};

void DisplayConfig(webcrawl::cConfig &config);
HWND GetChromeWindowHandle();
void ChangeChromeUrl(const HWND hwnd, const string &url);
bool FocusChromeWindow(const HWND hwnd);
std::pair<string, string> RecogText(const HWND hwnd, const RECT &nameR, const RECT &stockR
	, const int offsetY);
bool Filtering(INOUT string &name, INOUT string &stock);
bool WriteExcel(const char *fileName, const vector<sCrawlData> &datas);


int main()
{
	CoInitialize(NULL);

	webcrawl::cConfig config;
	if (!config.Read("config.json"))
	{
		cout << "Error Read config.json file" << endl;
		cout << config.m_err << endl;
		return 0;
	}

	DisplayConfig(config);

	if (!g_tess.Create())
	{
		cout << "Error Read Tesseract Resource" << endl;
		return 0;
	}

	cout << "Configuration..." << endl;
	Sleep(100);
	Sleep(100);
	Sleep(3000);

	const RECT nameR = config.m_nameRect;
	const RECT stockR = config.m_stockRect;
	const int offsetY = config.m_offsetY;
	
	vector<sCrawlData> crawlDatas;

	int cnt = 0;
	for (auto &site : config.m_sites)
	{
		HWND hwnd = GetChromeWindowHandle();

		// Goto URL
		cout << "Goto " << site.url << endl;
		string url = webcrawl::GetCurrentChromeTabURL(hwnd);
		while (url != site.url)
		{
			hwnd = GetChromeWindowHandle();
			ChangeChromeUrl(hwnd, site.url);
			Sleep(config.m_waitSiteLoadingSeconds * 1000);
			url = webcrawl::GetCurrentChromeTabURL(hwnd);
		}
		//

		auto result = RecogText(hwnd, nameR, stockR, 0);
		if (result.first.find(site.name) == string::npos)
		{
			cout << "\t ** Offset Recognition " << endl;
			result = RecogText(hwnd, nameR, stockR, config.m_offsetY);
		}

		++cnt;
		if (Filtering(result.first, result.second))
		{
			sCrawlData data;
			data.name = result.first;
			data.data = result.second;
			crawlDatas.push_back(data);

			cout << "Recognition (" << cnt << "/" << config.m_sites.size() << ")" 
				<< " >>" << endl;
			cout << "\t URL = " << url << endl;
			cout << "\t Name = " << result.first << endl;
			cout << "\t Stock = " << result.second << endl;
			cout << endl;
		}
		else
		{
			cout << "Error!! Crawling Name = " << site.name << ", URL = " << site.url << endl;
		}
	}

	const string fileName = webcrawl::GetCurrentDateTime() + ".txt";
	if (!WriteExcel(fileName.c_str(), crawlDatas))
	{
		cout << "Error Write Excel File" << endl;
	}

	CoUninitialize();
	return 0;
}


void DisplayConfig(webcrawl::cConfig &config)
{
	// display configuration 
	cout << "Configuration Information >>" << endl;
	cout << "\t* crawl-interval-seconds = " << config.m_crawlInteralSeconds << endl;
	cout << "\t* name-rect = " << config.m_nameRect.left << " " << config.m_nameRect.top << " "
		<< config.m_nameRect.right << " " << config.m_nameRect.bottom << endl;
	cout << "\t* stock-rect = " << config.m_stockRect.left << " " << config.m_stockRect.top << " "
		<< config.m_stockRect.right << " " << config.m_stockRect.bottom << endl;
	cout << "\t* offset-y = " << config.m_offsetY << endl;
	cout << "\t* wait-site-loading-seconds = " << config.m_waitSiteLoadingSeconds << endl;
	cout << endl << endl;
}


HWND GetChromeWindowHandle()
{
	HWND hwnd = NULL;
	while (!hwnd)
	{
		while (1)
		{
			hwnd = FindWindowEx(0, hwnd, L"Chrome_WidgetWin_1", NULL);
			if (!hwnd)
				break;
			if (!IsWindowVisible(hwnd))
				continue;
			break;
		}
		if (!hwnd)
		{
			ShellExecute(0, 0, L"www.google.com", 0, 0, SW_SHOW);
			Sleep(1000);
			Sleep(1000);
			Sleep(3000);
		}
	}

	return hwnd;
}


void ChangeChromeUrl(const HWND hwnd, const string &url)
{
	webcrawl::cKeyBot bot;
	const int delayTime = 200;

	if (!FocusChromeWindow(hwnd))
		return;

	// Set Cursor To Chrome Address Input Windowd
	bot.KeyDown(VK_MENU); // ALT key
	Sleep(delayTime);
	bot.KeyClick('D');
	bot.KeyUp(VK_MENU);

	// Copyt URL string to ClipBoard
	OpenClipboard(hwnd);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, url.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), url.c_str(), url.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
	//

	// Paste URL String to Chrome Address Input Window
	bot.KeyDown(VK_CONTROL);
	Sleep(delayTime);
	bot.KeyClick('V');
	bot.KeyUp(VK_CONTROL);

	// Goto URL Link
	bot.KeyClick(VK_RETURN);
}


// Focus window to send key event
bool FocusChromeWindow(const HWND hwnd)
{
	const int oldT = GetTickCount();
	while (GetForegroundWindow() != hwnd)
	{
		BringWindowToTop(hwnd);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		Sleep(300);

		// too much waste time in this code, return
		if (abs((int)GetTickCount() - oldT) > 10000)
			return false;
	}
	return true;
}


// return Text Recognition Result name, stock text
std::pair<string, string> RecogText(const HWND hwnd, const RECT &nameR, const RECT &stockR
	, const int offsetY)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);
	const int w = rc.right - rc.left;
	const int h = rc.bottom - rc.top;

	const string namePath = "name.bmp";
	g_capture.Capture(rc.left + nameR.left
		, rc.top + nameR.top + offsetY
		, (nameR.right - nameR.left)
		, (nameR.bottom - nameR.top)
		, namePath.c_str());

	const string stockPath = "stock.bmp";
	g_capture.Capture(rc.left + stockR.left
		, rc.top + stockR.top + offsetY
		, (stockR.right - stockR.left)
		, (stockR.bottom - stockR.top)
		, stockPath.c_str());

	Mat imgName = cv::imread(namePath, IMREAD_COLOR);
	const string recogName = g_tess.GetText(imgName.data, imgName.cols, imgName.rows, 3, imgName.step);
	
	Mat imgStock = cv::imread(stockPath, IMREAD_COLOR);
	const string recogStock = g_tess.GetText(imgStock.data, imgStock.cols, imgStock.rows, 3, imgStock.step);

	return { recogName, recogStock };
}


// stock 문자열에서 (1.0%) 안의 1.0 만 가져온다.
bool Filtering(INOUT string &name, INOUT string &stock)
{
	string out;
	int i = 0;
	int state = 0;
	while (i < (int)stock.size())
	{
		if (0 == state)
		{
			if ('(' == stock[i])
				state = 1;
		}
		else if (1 == state)
		{
			if ((')' == stock[i]) || ('%' == stock[i]))
			{
				break;
			}
			else
			{
				out += stock[i];
			}
		}
		++i;
	}

	if (out.empty())
		return false;

	webcrawl::trim(name);
	webcrawl::trim(out);

	stock = out;
	return true;
}


// Write Excel Format file
bool WriteExcel(const char *fileName, const vector<sCrawlData> &datas)
{
	ofstream ofs(fileName);
	if (!ofs.is_open())
		return false;

	for (auto &data : datas)
		ofs << data.name << "\t" << data.data << endl;

	return true;
}
