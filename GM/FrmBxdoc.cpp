// FrmBxdoc.cpp
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
#include    "GmDoc.h"
#include    "FrmBxdoc.h"
#include    "VwPrjgbx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDocFrame

IMPLEMENT_DYNCREATE(CDocFrame, CMDIChildWndEx)

CDocFrame::CDocFrame()
{
}

CDocFrame::~CDocFrame()
{
}

BEGIN_MESSAGE_MAP(CDocFrame, CMDIChildWndEx)
    //{{AFX_MSG_MAP(CDocFrame)
    //}}AFX_MSG_MAP
    ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDocFrame message handlers

BOOL CDocFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIChildWndEx::PreCreateWindow(cs))
        return FALSE;

    cs.style |= WS_CLIPCHILDREN;
    cs.style &= ~(DWORD)FWS_ADDTOTITLE;
    return TRUE;
}

void CDocFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    CDocument* pDoc = GetActiveDocument();
    CB::string str = pDoc->GetTitle();
    str += " - ";
    CB::string strType = CB::string::LoadString(IDS_PROJTYPE_GAMEBOX);
    str += strType;
    SetWindowText(str);
}

void CDocFrame::OnClose()
{
    // Close the document when the main document window is closed.
    GetActiveDocument()->OnCmdMsg(ID_FILE_CLOSE, CN_COMMAND, nullptr, nullptr);
}
