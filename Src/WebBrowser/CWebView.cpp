// CWebView.cpp : implementation file
//

#include "stdafx.h"
#include "WebBrowser.h"
#include "CWebView.h"


// CWebView

IMPLEMENT_DYNCREATE(CWebView, CHtmlView)

CWebView::CWebView()
{
}

CWebView::~CWebView()
{
}

void CWebView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWebView, CHtmlView)
END_MESSAGE_MAP()


// CWebView diagnostics

#ifdef _DEBUG
void CWebView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CWebView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CWebView message handlers
