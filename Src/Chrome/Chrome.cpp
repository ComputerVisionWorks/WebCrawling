
// get chrom tab url
// https://stackoverflow.com/questions/48504300/get-active-tab-url-in-chrome-with-c?noredirect=1&lq=1

#include "pch.h"
#include <Windows.h>
#include <AtlBase.h>
#include <AtlCom.h>
#include <UIAutomation.h>
//#include <iostream>
using namespace std;

int chrome_ui();
int chrome_tab();


class KeyBot
{
private:
	INPUT _buffer[1];

public:
	KeyBot();
	void KeyUp(char key);
	void KeyDown(char key);
	void KeyClick(char key);
};

KeyBot::KeyBot()
{
	_buffer->type = INPUT_KEYBOARD;
	_buffer->ki.wScan = 0;
	_buffer->ki.time = 0;
	_buffer->ki.dwExtraInfo = 0;
}

void KeyBot::KeyUp(char key)
{
	_buffer->ki.wVk = key;
	_buffer->ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, _buffer, sizeof(INPUT));
}

void KeyBot::KeyDown(char key)
{
	_buffer->ki.wVk = key;
	_buffer->ki.dwFlags = 0;
	SendInput(1, _buffer, sizeof(INPUT));
}

void KeyBot::KeyClick(char key)
{
	KeyDown(key);
	Sleep(10);
	KeyUp(key);
}


int main()
{
	//ShellExecute(0, 0, L"www.investing.com/equities/amazon-com-inc", 0, 0, SW_SHOW);

	//chrome_ui();

	chrome_tab();
}


int chrome_tab()
{
	HWND hwnd = NULL;
	while (true)
	{
		hwnd = FindWindowEx(0, hwnd, L"Chrome_WidgetWin_1", NULL);
		if (!hwnd)
			break;
		if (!IsWindowVisible(hwnd))
			continue;

		BringWindowToTop(hwnd);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		
		KeyBot bot;
		bot.KeyDown(VK_CONTROL);
		bot.KeyClick(VK_TAB);
		bot.KeyUp(VK_CONTROL);

		break;
	}
	return 0;
}


// https://stackoverflow.com/questions/48504300/get-active-tab-url-in-chrome-with-c?noredirect=1&lq=1
int chrome_ui()
{
	CoInitialize(NULL);
	HWND hwnd = NULL;
	while (true)
	{
		hwnd = FindWindowEx(0, hwnd, L"Chrome_WidgetWin_1", NULL);
		if (!hwnd)
			break;
		if (!IsWindowVisible(hwnd))
			continue;

		CComQIPtr<IUIAutomation> uia;
		if (FAILED(uia.CoCreateInstance(CLSID_CUIAutomation)) || !uia)
			break;

		CComPtr<IUIAutomationElement> root;
		if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root)
			break;

		CComPtr<IUIAutomationCondition> condition;

		//URL's id is 0xC354, or use UIA_EditControlTypeId for 1st edit box
		uia->CreatePropertyCondition(UIA_ControlTypePropertyId,
			CComVariant(0xC354), &condition);

		//or use edit control's name instead
		//uia->CreatePropertyCondition(UIA_NamePropertyId, 
		//      CComVariant(L"Address and search bar"), &condition);

		CComPtr<IUIAutomationElement> edit;
		if (FAILED(root->FindFirst(TreeScope_Descendants, condition, &edit))
			|| !edit)
			continue; //maybe we don't have the right tab, continue...

		CComVariant url;
		edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
		MessageBox(0, url.bstrVal, 0, 0);
		break;
	}
	CoUninitialize();
	return 0;
}