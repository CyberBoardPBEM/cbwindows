// winstate.cpp - classes used to manage window state.
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
#include    "WinState.h"
#include    "Versions.h"

///////////////////////////////////////////////////////////////////////////
// Returns FALSE if no frame restoration data is supplied by frames.

BOOL CWinStateManager::GetStateOfOpenDocumentFrames()
{
    ASSERT(m_pDoc != NULL);
    SetUpListIfNeedTo();

    // First entry in the frame list is the main frame...

    OwnerPtr<CWinStateElement> pWse(GetWindowState(AfxGetMainWnd()));

    pWse->m_wWinCode = wincodeMainFrame;
    m_pList->push_back(std::move(pWse));

    // Then we need to build a list of MDI frames that are in Z
    // order so we can restore the proper visual order later.

    std::vector<CB::not_null<CFrameWnd*>> tblFrame;
    GetDocumentFrameList(tblFrame);         // Get's unordered list
    ArrangeFrameListInZOrder(tblFrame);     // Order 'em

    // Scan the list in reverse Z order and obtain serialized
    // data needed to later restore the window state.

    for (size_t i = tblFrame.size(); i > 0; i--)
    {
        CWnd& pWnd = *tblFrame.at(i - size_t(1));
        OwnerPtr<CWinStateElement> pWse(GetWindowState(&pWnd));
        pWse->m_wWinCode = wincodeViewFrame;
        OnAnnotateWinStateElement(*pWse, &pWnd);
        m_pList->push_back(std::move(pWse));
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

void CWinStateManager::RestoreStateOfDocumentFrames()
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pList != NULL);
    if (m_pList == NULL)
        return;
    // Processes only the main frame and the MDI child frames.
    // All other records are ignored.
    for (CWinStateList::iterator pos = m_pList->begin() ; pos != m_pList->end() ; ++pos)
    {
        CWinStateElement& pWse = **pos;
        if (pWse.m_wWinCode == wincodeMainFrame)
        {
            CWnd* pWnd = AfxGetMainWnd();
            ASSERT(pWnd != NULL);
            CRect rctVScreen;
            SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)rctVScreen, 0);
            int cxVScreen = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            if (cxVScreen > 0)
            {
                // Multimonitor metrics are are supported
                int cyVScreen = GetSystemMetrics(SM_CYVIRTUALSCREEN);
                int xVScreen = GetSystemMetrics(SM_XVIRTUALSCREEN);
                int yVScreen = GetSystemMetrics(SM_YVIRTUALSCREEN);
                rctVScreen.SetRect(xVScreen, yVScreen, xVScreen + cxVScreen, yVScreen + cyVScreen);
            }
            else
            {
                // Otherwise set the virtual screen rect to the primary
                // desktop dimensions.
                SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)rctVScreen, 0);
            }
            CRect rctDesktop;
            SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)rctDesktop, 0);

            // Clone the saved window position for easier calculations
            CRect rctSaved(pWse.m_wndState.rcNormalPosition);

            // Check if the Window's midpoint is visible. If so leave it's position alone.
            if (!rctVScreen.PtInRect(rctSaved.CenterPoint()))
            {
                // Force the window onto the primary desktop area.
                pWse.m_wndState.rcNormalPosition = rctDesktop;
            }
            pWnd->SetWindowPlacement(&pWse.m_wndState);
            RestoreWindowState(pWnd, pWse);
        }
        else if (pWse.m_wWinCode == wincodeViewFrame)
        {
            CWnd* pWnd = OnGetFrameForWinStateElement(pWse);
            ASSERT(pWnd != NULL);
            if (pWnd != NULL)
            {
                pWnd->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE);
                pWnd->SetWindowPlacement(&pWse.m_wndState);
                RestoreWindowState(pWnd, pWse);
            }
        }
    }
    (DYNAMIC_DOWNCAST(CMDIFrameWndEx, AfxGetMainWnd()))->RecalcLayout();
}

///////////////////////////////////////////////////////////////////////////

OwnerPtr<CWinStateManager::CWinStateElement> CWinStateManager::GetWindowState(CWnd* pWnd)
{
    ASSERT(m_pDoc != NULL);
    OwnerPtr<CWinStateElement> pWse = OnCreateWinStateElement();
    pWnd->GetWindowPlacement(&pWse->m_wndState);

    TRY
    {
        CMemFile file;
        CArchive ar(&file, CArchive::store);
        BOOL bOK = (BOOL)pWnd->SendMessage(WM_WINSTATE, (WPARAM)&ar, 0);
        ar.Close();
        if (bOK)
        {
            size_t len = value_preserving_cast<size_t>(file.GetLength());
            pWse->m_pWinStateBfr.Reset(file.Detach(), len);
        }
    }
    END_TRY

    return pWse;
}

///////////////////////////////////////////////////////////////////////////

BOOL CWinStateManager::RestoreWindowState(CWnd* pWnd, CWinStateElement& pWse)
{
    ASSERT(m_pDoc != NULL);
    if (pWse.m_pWinStateBfr == NULL)
        return TRUE;

    BOOL bOK = FALSE;

    TRY
    {
        CMemFile file(pWse.m_pWinStateBfr, value_preserving_cast<unsigned>(pWse.m_pWinStateBfr.GetSize()));
        CArchive ar(&file, CArchive::load);
        bOK = (BOOL)pWnd->SendMessage(WM_WINSTATE, (WPARAM)&ar, 1);
        ar.Close();
        file.Detach();
    }
    END_TRY
    return bOK;
}

/////////////////////////////////////////////////////////////////////////////

void CWinStateManager::GetDocumentFrameList(std::vector<CB::not_null<CFrameWnd*>>& tblFrames)
{
    tblFrames.clear();

    POSITION pos = m_pDoc->GetFirstViewPosition();
    while (pos != NULL)
    {
        CView* pView = m_pDoc->GetNextView(pos);
        CFrameWnd* pFrame = pView->GetParentFrame();
        ASSERT(pFrame != NULL);
        size_t i;
        for (i = 0; i < tblFrames.size(); i++)
        {
            if (pFrame == tblFrames.at(i))
                break;
        }
        if (i == tblFrames.size())
            tblFrames.push_back(pFrame);          // Add new frame
    }
}

/////////////////////////////////////////////////////////////////////////////

CWnd* CWinStateManager::GetDocumentFrameHavingRuntimeClass(CRuntimeClass* pClass)
{
    POSITION pos = m_pDoc->GetFirstViewPosition();
    while (pos != NULL)
    {
        CView* pView = m_pDoc->GetNextView(pos);
        CFrameWnd* pFrame = pView->GetParentFrame();
        if (pFrame->IsKindOf(pClass))
            return pFrame;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CWinStateManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (m_pList == NULL || m_pList->empty())
        {
            ar << (DWORD)0;
            return;
        }
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<DWORD>(m_pList->size());
        }
        else
        {
            CB::WriteCount(ar, m_pList->size());
        }
        for (CWinStateList::iterator pos = m_pList->begin() ; pos != m_pList->end() ; ++pos)
        {
            CWinStateElement& pWse = **pos;
            pWse.Serialize(ar);
        }
    }
    else
    {
        m_pList = nullptr;
        size_t dwCount;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            DWORD temp;
            ar >> temp;
            dwCount = temp;
        }
        else
        {
            dwCount = CB::ReadCount(ar);
        }
        if (dwCount == size_t(0))
            return;
        m_pList = MakeOwner<CWinStateList>();
        while (dwCount--)
        {
            OwnerPtr<CWinStateElement> pWse(OnCreateWinStateElement());
            pWse->Serialize(ar);
            m_pList->push_back(std::move(pWse));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CWinStateManager::ArrangeFrameListInZOrder(std::vector<CB::not_null<CFrameWnd*>>& tblFrames)
{
    std::vector<CFrameWnd*> tblZFrames;

    CMDIFrameWnd* pFrame = (CMDIFrameWnd*)AfxGetMainWnd();
    ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)));

    EnumChildWindows(pFrame->m_hWndMDIClient, EnumFrames, (LPARAM)&tblZFrames);

    // Null out any entries that aren't in the caller's table
    for (size_t i = 0; i < tblZFrames.size(); i++)
    {
        size_t j;
        for (j = 0; j < tblFrames.size(); j++)
        {
            if (tblFrames.at(j) == tblZFrames.at(i))
                break;
        }
        if (j == tblFrames.size())
            tblZFrames.at(i) = NULL;
    }
    // Now copy the remaining frame pointers into the caller's list
    // in Z order (top to bottom)
    tblFrames.clear();
    tblFrames.reserve(tblZFrames.size());
    for (size_t i = size_t(0); i < tblZFrames.size(); i++)
    {
        if (tblZFrames.at(i) != NULL)
            tblFrames.push_back(tblZFrames.at(i));
    }
}

BOOL CALLBACK CWinStateManager::EnumFrames(HWND hWnd, LPARAM dwTblFramePtr)
{
    std::vector<CFrameWnd*>& pTbl = CheckedDeref(reinterpret_cast<std::vector<CFrameWnd*>*>(dwTblFramePtr));
    CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
    // Only interested in certain kinds of windows.
    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
        pTbl.push_back(static_cast<CMDIChildWndEx*>(pWnd));
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

void CWinStateManager::SetUpListIfNeedTo()
{
    ASSERT(m_pDoc != NULL);
    if (m_pList == NULL)
        m_pList = MakeOwner<CWinStateList>();
}

///////////////////////////////////////////////////////////////////////////

CWinStateManager::CWinStateElement::CWinStateElement()
{
    m_wWinCode = 0;
    m_wUserCode1 = 0;
    m_boardID = nullBid;
}

void CWinStateManager::CWinStateElement::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wWinCode;
        ar << m_wUserCode1;
        ar << m_boardID;
        ar << m_wndState;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<DWORD>(m_pWinStateBfr.GetSize());
        }
        else
        {
            CB::WriteCount(ar, m_pWinStateBfr.GetSize());
        }
        if (m_pWinStateBfr.GetSize() > size_t(0))
            ar.Write(m_pWinStateBfr, value_preserving_cast<unsigned>(m_pWinStateBfr.GetSize()));
    }
    else
    {
        m_pWinStateBfr.Reset();

        ar >> m_wWinCode;
        ar >> m_wUserCode1;
        ar >> m_boardID;
        ar >> m_wndState;
        size_t size;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            DWORD dwSize;
            ar >> dwSize;
            size = dwSize;
        }
        else
        {
            size = CB::ReadCount(ar);
        }
        if (size > size_t(0))
        {
            m_pWinStateBfr.Reset(static_cast<BYTE*>(malloc(size)), size);
            ASSERT(m_pWinStateBfr != NULL);
            if (m_pWinStateBfr == NULL)
            {
                AfxThrowMemoryException();
            }
            ar.Read(m_pWinStateBfr, value_preserving_cast<unsigned>(m_pWinStateBfr.GetSize()));
        }
    }
}

///////////////////////////////////////////////////////////////////////////

CWinPlacement::CWinPlacement()
{
    memset(this, 0, sizeof(WINDOWPLACEMENT));
    length = sizeof(WINDOWPLACEMENT);
}

CArchive& AFXAPI operator<<(CArchive& ar, const CWinPlacement& wndPlace)
{
//    ar << (DWORD)wndPlace.length;
    ar << (DWORD)wndPlace.flags;
    ar << (DWORD)wndPlace.showCmd;
    ar << (DWORD)wndPlace.ptMinPosition.x;
    ar << (DWORD)wndPlace.ptMinPosition.y;
    ar << (DWORD)wndPlace.ptMaxPosition.x;
    ar << (DWORD)wndPlace.ptMaxPosition.y;
    ar << (DWORD)wndPlace.rcNormalPosition.top;
    ar << (DWORD)wndPlace.rcNormalPosition.bottom;
    ar << (DWORD)wndPlace.rcNormalPosition.left;
    ar << (DWORD)wndPlace.rcNormalPosition.right;

    return ar;
}

CArchive& AFXAPI operator>>(CArchive& ar, CWinPlacement& wndPlace)
{
    DWORD dwTmp;
//    ar >> dwTmp; wndPlace.length = (UINT)dwTmp;
    ar >> dwTmp; wndPlace.flags = (UINT)dwTmp;
    ar >> dwTmp; wndPlace.showCmd = (UINT)dwTmp;
    ar >> dwTmp; wndPlace.ptMinPosition.x = (LONG)dwTmp;
    ar >> dwTmp; wndPlace.ptMinPosition.y = (LONG)dwTmp;
    ar >> dwTmp; wndPlace.ptMaxPosition.x = (LONG)dwTmp;
    ar >> dwTmp; wndPlace.ptMaxPosition.y = (LONG)dwTmp;
    ar >> dwTmp; wndPlace.rcNormalPosition.top = (LONG)dwTmp;
    ar >> dwTmp; wndPlace.rcNormalPosition.bottom = (LONG)dwTmp;
    ar >> dwTmp; wndPlace.rcNormalPosition.left = (LONG)dwTmp;
    ar >> dwTmp; wndPlace.rcNormalPosition.right = (LONG)dwTmp;

    return ar;
}

