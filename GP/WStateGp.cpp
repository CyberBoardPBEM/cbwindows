// WStateGp.cpp - classes used to manage player program window state.
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "Board.h"
#include    "PBoard.h"
#include    "VwPbrd.h"
#include    "VwPrjgam.h"
#include    "WStateGp.h"

enum { gpFrmProject = 0, gpFrmPlayBoard = 1 };

CWnd& CGpWinStateMgr::OnGetFrameForWinStateElement(const CWinStateElement& pWse)
{
    wxASSERT(pWse.m_wWinCode == wincodeViewFrame);
    CGamDoc& pDoc = GetDocument();

#if 0
    if (pWse.m_wUserCode1 == gpFrmProject)
    {
        CWnd& pWnd = pDoc.FindProjectView();
        return pWnd;
    }
    else if (pWse.m_wUserCode1 == gpFrmPlayBoard)
    {
        // The second user code is the board's serial number.
        CPlayBoard& pPBoard = CheckedDeref(pDoc.GetPBoardManager().GetPBoardBySerial(pWse.m_boardID));
        CPlayBoardView* pView = pDoc.FindPBoardView(pPBoard);
        if (pView == NULL)
        {
            // No frame open for board. Create it.
            pDoc.CreateNewFrame(
                pPBoard.GetBoard()->GetName(), pPBoard);
            // Try to locate it again
            pView = pDoc.FindPBoardView(pPBoard);
            wxASSERT(pView);
        }
        return CheckedDeref(pView->GetParentFrame());
    }
    else
        AfxThrowInvalidArgException();
#else
    AfxThrowNotSupportedException();
#endif
}

void CGpWinStateMgr::OnAnnotateWinStateElement(CWinStateElement& pWse, const CWnd& pWnd)
{
#if 0
    if (pWnd.IsKindOf(RUNTIME_CLASS(CProjFrame)))
        pWse.m_wUserCode1 = gpFrmProject;
    else if (pWnd.IsKindOf(RUNTIME_CLASS(CPlayBoardPanelContainer)))
    {
        const CPlayBoardPanelContainer& pPanelContainer = static_cast<const CPlayBoardPanelContainer&>(pWnd);
        const CPlayBoardPanel& pPanel = pPanelContainer.GetChild();
        pWse.m_wUserCode1 = gpFrmPlayBoard;
        pWse.m_boardID = pPanel.m_pPBoard->GetSerialNumber();
    }
#else
    AfxThrowNotSupportedException();
#endif
}

