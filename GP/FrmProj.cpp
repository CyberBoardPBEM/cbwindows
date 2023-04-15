// FrmProg.cpp
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "FrmProj.h"
#include    "VwPrjgsn.h"
#include    "VwPrjgam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjFrame

IMPLEMENT_DYNCREATE(CProjFrame, CMDIChildWndEx)

CProjFrame::CProjFrame()
{
}

CProjFrame::~CProjFrame()
{
}

BEGIN_MESSAGE_MAP(CProjFrame, CMDIChildWndEx)
    //{{AFX_MSG_MAP(CProjFrame)
    ON_WM_SYSCOMMAND()
    //}}AFX_MSG_MAP
    ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjFrame message handlers

BOOL CProjFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIChildWndEx::PreCreateWindow(cs))
        return FALSE;

    cs.style |= WS_CLIPCHILDREN;
    cs.style &= ~(DWORD)FWS_ADDTOTITLE;
    return TRUE;
}

void CProjFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    CGamDoc* pDoc = (CGamDoc*)GetActiveDocument();
    CB::string str = pDoc->GetTitle();
    str += " - ";
    CB::string strType;
    if (pDoc->IsScenario())
        strType = CB::string::LoadString(IDS_PROJTYPE_SCENARIO);
    else
        strType = CB::string::LoadString(IDS_PROJTYPE_GAME);

    str += strType;
    SetWindowText(str);
}

void CProjFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == SC_CLOSE)
    {
        CView *pView = GetActiveView();
        if (pView)
        {
            if (pView->IsKindOf(RUNTIME_CLASS(CGsnProjView)) ||
                pView->IsKindOf(RUNTIME_CLASS(CGamProjView)))
            {
                ((CGamDoc*)GetActiveDocument())->OnFileClose();
                return;
            }
        }
    }
    CMDIChildWndEx::OnSysCommand(nID, lParam);
}

void CProjFrame::OnClose()
{
    // Close the document when the main document window is closed.
    ((CGamDoc*)GetActiveDocument())->OnFileClose();
}
