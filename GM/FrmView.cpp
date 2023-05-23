// FrmView.cpp
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include    "stdafx.h"
#include    "Gm.h"
#include    "FrmView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewFrame

IMPLEMENT_DYNCREATE(CViewFrame, CMDIChildWndEx)

CViewFrame::CViewFrame()
{
}

CViewFrame::~CViewFrame()
{
}

BEGIN_MESSAGE_MAP(CViewFrame, CMDIChildWndEx)
    //{{AFX_MSG_MAP(CViewFrame)
    ON_WM_CLOSE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CViewFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIChildWndEx::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(LTGRAY_BRUSH),
        AfxGetApp()->LoadIconW(IDR_BOARDVIEW));

    cs.style |= WS_CLIPCHILDREN;
    cs.style &= ~(DWORD)FWS_ADDTOTITLE;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CViewFrame message handlers

void CViewFrame::OnClose()
{
    CWnd::OnClose();            // Short circuit frame's doc close code
}
