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

///////////////////////////////////////////////////////////////////////////

CWinStateManager::~CWinStateManager()
{
    DestroyList();
}

///////////////////////////////////////////////////////////////////////////
// Returns FALSE if no frame restoration data is supplied by frames.

BOOL CWinStateManager::GetStateOfOpenDocumentFrames()
{
    ASSERT(m_pDoc != NULL);
    SetUpListIfNeedTo();

    // First entry in the frame list is the main frame...

    CWinStateElement* pWse = GetWindowState(AfxGetMainWnd());
    if (pWse == NULL)
        return FALSE;

    pWse->m_wWinCode = wincodeMainFrame;
    m_pList->AddTail(pWse);

    // Then we need to build a list of MDI frames that are in Z
    // order so we can restore the proper visual order later.

    CPtrArray tblFrame;
    GetDocumentFrameList(tblFrame);         // Get's unordered list
    ArrangeFrameListInZOrder(tblFrame);     // Order 'em

    // Scan the list in reverse Z order and obtain serialized
    // data needed to later restore the window state.

    for (int i = tblFrame.GetSize() - 1; i >= 0; i--)
    {
        CWnd* pWnd = (CWnd*)tblFrame.GetAt(i);
        CWinStateElement* pWse = GetWindowState(pWnd);
        if (pWse != NULL)
        {
            pWse->m_wWinCode = wincodeViewFrame;
            OnAnnotateWinStateElement(pWse, pWnd);
            m_pList->AddTail(pWse);
        }
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
    POSITION pos = m_pList->GetHeadPosition();
    while (pos != NULL)
    {
        CWinStateElement* pWse = (CWinStateElement*)m_pList->GetNext(pos);
        if (pWse->m_wWinCode == wincodeMainFrame)
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
            CRect rctSaved(pWse->m_wndState.rcNormalPosition);

            // Check if the Window's midpoint is visible. If so leave it's position alone.
            if (!rctVScreen.PtInRect(rctSaved.CenterPoint()))
            {
                // Force the window onto the primary desktop area.
                pWse->m_wndState.rcNormalPosition = rctDesktop;
            }
            pWnd->SetWindowPlacement(&pWse->m_wndState);
            RestoreWindowState(pWnd, pWse);
        }
        else if (pWse->m_wWinCode == wincodeViewFrame)
        {
            CWnd* pWnd = OnGetFrameForWinStateElement(pWse);
            ASSERT(pWnd != NULL);
            if (pWnd != NULL)
            {
                pWnd->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE);
                pWnd->SetWindowPlacement(&pWse->m_wndState);
                RestoreWindowState(pWnd, pWse);
            }
        }
    }
    (DYNAMIC_DOWNCAST(CMDIFrameWndEx, AfxGetMainWnd()))->RecalcLayout();
}

///////////////////////////////////////////////////////////////////////////

CWinStateManager::CWinStateElement* CWinStateManager::GetWindowState(CWnd* pWnd)
{
    ASSERT(m_pDoc != NULL);
    CWinStateElement* pWse;

    pWse = OnCreateWinStateElement();
    ASSERT(pWse != NULL);
    pWnd->GetWindowPlacement(&pWse->m_wndState);

    TRY
    {
        CMemFile file;
        CArchive ar(&file, CArchive::store);
        BOOL bOK = (BOOL)pWnd->SendMessage(WM_WINSTATE, (WPARAM)&ar, 0);
        ar.Close();
        if (bOK)
        {
            pWse->m_dwWinStateBfrSize = (DWORD)file.GetLength();
            pWse->m_pWinStateBfr = file.Detach();
        }
    }
    END_TRY

    return pWse;
}

///////////////////////////////////////////////////////////////////////////

BOOL CWinStateManager::RestoreWindowState(CWnd* pWnd, CWinStateElement* pWse)
{
    ASSERT(m_pDoc != NULL);
    ASSERT(pWse != NULL);
    if (pWse->m_pWinStateBfr == NULL)
        return TRUE;

    BOOL bOK = FALSE;

    TRY
    {
        CMemFile file(pWse->m_pWinStateBfr, pWse->m_dwWinStateBfrSize);
        CArchive ar(&file, CArchive::load);
        bOK = (BOOL)pWnd->SendMessage(WM_WINSTATE, (WPARAM)&ar, 1);
        ar.Close();
        file.Detach();
    }
    END_TRY
    return bOK;
}

/////////////////////////////////////////////////////////////////////////////

void CWinStateManager::GetDocumentFrameList(CPtrArray& tblFrames)
{
    tblFrames.RemoveAll();

    POSITION pos = m_pDoc->GetFirstViewPosition();
    while (pos != NULL)
    {
        CView* pView = m_pDoc->GetNextView(pos);
        CFrameWnd* pFrame = pView->GetParentFrame();
        ASSERT(pFrame != NULL);
        int i;
        for (i = 0; i < tblFrames.GetSize(); i++)
        {
            if (pFrame == (CFrameWnd*)tblFrames.GetAt(i))
                break;
        }
        if (i == tblFrames.GetSize())
            tblFrames.Add(pFrame);          // Add new frame
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
        if (m_pList == NULL || m_pList->GetCount() == 0)
        {
            ar << (DWORD)0;
            return;
        }
        ar << (DWORD)m_pList->GetCount();
        POSITION pos = m_pList->GetHeadPosition();
        while (pos != NULL)
        {
            CWinStateElement* pWse = (CWinStateElement*)m_pList->GetNext(pos);
            pWse->Serialize(ar);
        }
    }
    else
    {
        if (m_pList != NULL)
            delete m_pList;
        m_pList = NULL;
        DWORD dwCount;
        ar >> dwCount;
        if (dwCount == 0)
            return;
        m_pList = new CPtrList;
        while (dwCount--)
        {
            CWinStateElement* pWse = OnCreateWinStateElement();
            pWse->Serialize(ar);
            m_pList->AddTail(pWse);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CWinStateManager::ArrangeFrameListInZOrder(CPtrArray& tblFrames)
{
    CPtrArray tblZFrames;

    CMDIFrameWnd* pFrame = (CMDIFrameWnd*)AfxGetMainWnd();
    ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)));

    EnumChildWindows(pFrame->m_hWndMDIClient, EnumFrames, (LPARAM)&tblZFrames);

    // Null out any entries that aren't in the caller's table
    for (int i = 0; i < tblZFrames.GetSize(); i++)
    {
        int j;
        for (j = 0; j < tblFrames.GetSize(); j++)
        {
            if (tblFrames.GetAt(j) == tblZFrames.GetAt(i))
                break;
        }
        if (j == tblFrames.GetSize())
            tblZFrames.SetAt(i, NULL);
    }
    // Now copy the remaining frame pointers into the caller's list
    // in Z order (top to bottom)
    tblFrames.RemoveAll();
    for (int i = 0; i < tblZFrames.GetSize(); i++)
    {
        if (tblZFrames.GetAt(i) != NULL)
            tblFrames.Add(tblZFrames.GetAt(i));
    }
}

BOOL CALLBACK CWinStateManager::EnumFrames(HWND hWnd, LPARAM dwTblFramePtr)
{
    CPtrArray* pTbl = (CPtrArray*)dwTblFramePtr;
    CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
    // Only interested in certain kinds of windows.
    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
        pTbl->Add(pWnd);
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

void CWinStateManager::SetUpListIfNeedTo()
{
    ASSERT(m_pDoc != NULL);
    if (m_pList == NULL)
        m_pList = new CPtrList();
}

void CWinStateManager::DestroyList()
{
    if (m_pList != NULL)
    {
        POSITION pos = m_pList->GetHeadPosition();
        while (pos != NULL)
        {
            CWinStateElement* pWse = (CWinStateElement*)m_pList->GetNext(pos);
            ASSERT(pWse != NULL);
            delete pWse;
        }
        delete m_pList;
        m_pList = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////

CWinStateManager::CWinStateElement::CWinStateElement()
{
    m_wWinCode = 0;
    m_wUserCode1 = 0;
    m_wUserCode2 = 0;
    m_dwWinStateBfrSize = 0;
    m_pWinStateBfr = NULL;
}

CWinStateManager::CWinStateElement::~CWinStateElement()
{
    if (m_pWinStateBfr != NULL)
        free(m_pWinStateBfr);
}

void CWinStateManager::CWinStateElement::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wWinCode;
        ar << m_wUserCode1;
        ar << m_wUserCode2;
        ar << m_wndState;
        ar << m_dwWinStateBfrSize;
        if (m_dwWinStateBfrSize > 0)
            ar.Write(m_pWinStateBfr, m_dwWinStateBfrSize);
    }
    else
    {
        if (m_pWinStateBfr != NULL)
            free(m_pWinStateBfr);
        m_pWinStateBfr = NULL;

        ar >> m_wWinCode;
        ar >> m_wUserCode1;
        ar >> m_wUserCode2;
        ar >> m_wndState;
        ar >> m_dwWinStateBfrSize;
        if (m_dwWinStateBfrSize > 0)
        {
            m_pWinStateBfr = (BYTE*)malloc(m_dwWinStateBfrSize);
            ASSERT(m_pWinStateBfr != NULL);
            if (m_pWinStateBfr == NULL)
                AfxThrowMemoryException();
            ar.Read(m_pWinStateBfr, m_dwWinStateBfrSize);
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

