// WStateGp.cpp - classes used to manage player program window state.
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
#include    "FrmPbrd.h"
#include    "FrmProj.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "WStateGp.h"

enum { gpFrmProject = 0, gpFrmPlayBoard = 1 };

CWnd* CGpWinStateMgr::OnGetFrameForWinStateElement(CWinStateElement* pWse)
{
    ASSERT(pWse->m_wWinCode == wincodeViewFrame);
    CGamDoc* pDoc = GetDocument();

    if (pWse->m_wUserCode1 == gpFrmProject)
    {
        CWnd* pWnd = GetDocumentFrameHavingRuntimeClass(RUNTIME_CLASS(CProjFrame));
        ASSERT(pWnd != NULL);           // Must be open.
        return pWnd;
    }
    else if (pWse->m_wUserCode1 == gpFrmPlayBoard)
    {
        // The second user code is the board's serial number.
        CPlayBoard* pPBoard = pDoc->GetPBoardManager()->GetPBoardBySerial(pWse->m_wUserCode2);
        CView* pView = pDoc->FindPBoardView(pPBoard);
        if (pView == NULL)
        {
            // No frame open for board. Create it.
            pDoc->CreateNewFrame(GetApp()->m_pBrdViewTmpl,
                pPBoard->GetBoard()->GetName(), pPBoard);
            // Try to locate it again
            pView = pDoc->FindPBoardView(pPBoard);
        }
        return pView->GetParentFrame();
    }
    else
        return NULL;
}

void CGpWinStateMgr::OnAnnotateWinStateElement(CWinStateElement *pWse, CWnd *pWnd)
{
    if (pWnd->IsKindOf(RUNTIME_CLASS(CProjFrame)))
        pWse->m_wUserCode1 = gpFrmProject;
    else if (pWnd->IsKindOf(RUNTIME_CLASS(CPlayBoardFrame)))
    {
        CPlayBoardFrame* pFrame = (CPlayBoardFrame*)pWnd;
        pWse->m_wUserCode1 = gpFrmPlayBoard;
        pWse->m_wUserCode2 = pFrame->m_pPBoard->GetSerialNumber();
    }
}

