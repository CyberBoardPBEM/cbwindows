// LibMfc.cpp - Miscellaneous MFC Support Functions
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
#include    <WTYPES.H>

#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    LPDWORD pdwPidOrHandle = (LPDWORD)lParam;
    DWORD dwPid;
    GetWindowThreadProcessId(hwnd, &dwPid);

    if (dwPid == *pdwPidOrHandle)
    {
        *pdwPidOrHandle = (DWORD)hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND FindWindowForProcessID(DWORD dwProcessID)
{
    DWORD dwProcIDorHandle = dwProcessID;
    if (EnumWindows(EnumWindowsProc, (LPARAM)&dwProcIDorHandle) != 0)
        return NULL;                // Didn't find one
    return (HWND)dwProcIDorHandle;
}

// Returns TRUE if succeeded
BOOL FindWindowForProcessIDAndBringToFront(DWORD dwProcessID)
{
    HWND hWnd = FindWindowForProcessID(dwProcessID);
    if (hWnd == NULL)
        return FALSE;
    BringWindowToTop(hWnd);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL AppendStringToEditBox(CEdit& edit, CString strAppend,
    BOOL bEnsureNewline /* = FALSE */)
{
    if (bEnsureNewline)
    {
        CString str;
        edit.GetWindowText(str);
        if (str != "" && str.GetAt(str.GetLength() - 1) != '\n')
            AppendStringToEditBox(edit, "\r\n", FALSE);
    }
    int nLen = edit.GetWindowTextLength();
    edit.SetSel(nLen, nLen);
    edit.ReplaceSel(strAppend);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL GetMaximumTextExtent(HDC hDC, LPCTSTR pszStr, int nStringLen, int nMaxWidth,
    int* pnFit)
{
    int  nStrLen = nStringLen == -1 ? lstrlen(pszStr) : nStringLen;
    SIZE size;

    // Determine the largest string that can fit in the cell...
    if (GetTextExtentExPoint(hDC, pszStr, nStrLen, nMaxWidth,
            pnFit, NULL, &size))
        return TRUE;

    // If it failed (probably due to running on Win32s) do it the hard way!
    //
    // First try entire string extent to save time.
    if (!GetTextExtentPoint(hDC, pszStr, nStrLen, &size))
        return FALSE;

    if (size.cx <= nMaxWidth)
    {
        *pnFit = nStrLen;
        return TRUE;
    }
    // Use brute force approach. Perhaps this can be made more clever when
    // the product ship data isn't looming.
    int nChar;
    for (nChar = 0; nChar < nStrLen; nChar++)
    {
        if (!GetTextExtentPoint(hDC, pszStr, nChar + 1, &size))
            return FALSE;
        if (size.cx > nMaxWidth)
            break;
    }
    ASSERT(nChar < nStrLen);        // Should always bail out before end string!
    *pnFit = nChar;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CDisableMainWindow::CDisableMainWindow(BOOL bDisable /* = TRUE */)
{
    DisableMainWindow(bDisable);
}

CDisableMainWindow::~CDisableMainWindow()
{
    DisableMainWindow(FALSE);
}

void CDisableMainWindow::DisableMainWindow(BOOL bDisable /* = TRUE */)
{
    ASSERT(AfxGetApp() != NULL);
    ASSERT(AfxGetApp()->m_pMainWnd != NULL);

    AfxGetApp()->m_pMainWnd->EnableWindow(!bDisable);
}

///////////////////////////////////////////////////////////////////////
// This routine will simply create a unique temporary filename.

void GetTemporaryFileName(LPCTSTR lpPrefixString, CString& strTempName)
{
    TCHAR szTempName[_MAX_PATH];
    TCHAR szPath[_MAX_PATH];

    // Save to temporary path
    VERIFY(GetTempPath(sizeof(szPath)/sizeof(TCHAR), szPath) != 0);
    VERIFY(GetTempFileName(szPath, lpPrefixString, 0, szTempName) != 0);
    strTempName = szTempName;
}

///////////////////////////////////////////////////////////////////////
// TruncatedAnsiStringWithEllipses() - This function shortens a string
// to fit within a target width. The shortened string has "..." at the
// end of it.

void TruncatedAnsiStringWithEllipses(CDC* pRefDC, int nTargWidth, CString& str)
{
    CString strWrk = "..." + str;       // Ellipse is prefix for size calc
    int nFitWidth;
    VERIFY(GetMaximumTextExtent(pRefDC->m_hAttribDC, strWrk, strWrk.GetLength(),
        nTargWidth, &nFitWidth));
    ASSERT(nFitWidth > 0);
    str = str.Left(nFitWidth - 3) + "...";
}

///////////////////////////////////////////////////////////////////////
// This routine will scan the supplied menu for an entry hooked to
// a submenu. It then checks to see if the ID of the first item in
// submenu matches the specified ID. If it does, the submenu's index is
// returned. Otherwise -1 is returned.

UINT LocateSubMenuIndexOfMenuHavingStartingID(CMenu* pMenu, UINT nID)
{
    for (UINT nIndex = 0;
        pMenu->GetMenuState(nIndex, MF_BYPOSITION) != -1;
        nIndex++)
    {
        CMenu* pSubMenu = pMenu->GetSubMenu(nIndex);
        // Support two levels of checking.
        if (pSubMenu != NULL)
        {
            if (pSubMenu->GetMenuItemID(0) == nID)
                return nIndex;
            // Try the next level...
            pSubMenu = pSubMenu->GetSubMenu(0);
            if (pSubMenu != NULL && pSubMenu->GetMenuItemID(0) == nID)
                return nIndex;
        }
//      if (pSubMenu != NULL && pSubMenu->GetMenuItemID(0) == nID)
//          return nIndex;
    }
    return (UINT)-1;
}

///////////////////////////////////////////////////////////////////////
// This routine appends a list of strings to the supplied menu. The first
// string is assigned ID 'nBaseID'. The second ID is 'nBaseID' + 1 and so
// on. You can optionally provide a UINT array which specifies which
// indexes of the string array you wish to have added to the menu (i.e.,
// it allows you to specify a subset of the string array. Finally, since
// the menu could be quite large you can specify how many items are stacked
// vertically before a menu break is forced. The default break value
// is 20 menu items.

void CreateSequentialSubMenuIDs(CMenu& menu, UINT nBaseID, CStringArray& tblNames,
    CUIntArray* pTblSelections /* = NULL */, UINT nBreaksAt /* = 20 */)
{
    if (tblNames.GetSize() > 0)
    {
        int nMenuEntries = pTblSelections != NULL ? pTblSelections->GetSize() :
            tblNames.GetSize();
        for (int i = 0; i < nMenuEntries; i++)
        {
            int nNameIdx = pTblSelections != NULL ? pTblSelections->GetAt(i) : i;
            ASSERT(nNameIdx < tblNames.GetSize());
            // Break the menu every 'nBreaksAt' lines since Windows
            // won't automatically break the menu if it is
            // too tall.
            UINT nFlags = MF_ENABLED | MF_STRING |
                (i % nBreaksAt == 0 && i != 0 ? MF_MENUBARBREAK : 0);
            VERIFY(menu.AppendMenu(nFlags, nBaseID + i, tblNames.GetAt(nNameIdx)));
        }
    }
}

///////////////////////////////////////////////////////////////////////
// This routine maps key codes to scrollbar messages. The scroll
// message is sent to the supplied window. Returns FALSE if
// no key mapping was found.

BOOL TranslateKeyToScrollBarMessage(CWnd* pWnd, UINT nChar)
{
    UINT nCmd = WM_VSCROLL;
    UINT nSBCode = (UINT)-1;
    BOOL bControl = GetKeyState(VK_CONTROL) < 0;
    switch(nChar)
    {
        case VK_UP:    nSBCode = SB_LINEUP; break;
        case VK_DOWN:  nSBCode = SB_LINEDOWN; break;
        case VK_LEFT:  nSBCode = SB_LINELEFT;  nCmd = WM_HSCROLL; break;
        case VK_RIGHT: nSBCode = SB_LINERIGHT; nCmd = WM_HSCROLL; break;
        case VK_PRIOR:
            nSBCode = SB_PAGEUP; if (bControl) nCmd = WM_HSCROLL; break;
        case VK_NEXT:
            nSBCode = SB_PAGEDOWN; if (bControl) nCmd = WM_HSCROLL; break;
        case VK_HOME:
            nSBCode = SB_TOP; if (bControl) nCmd = WM_HSCROLL; break;
        case VK_END:
            nSBCode = SB_BOTTOM; if (bControl) nCmd = WM_HSCROLL; break;
        default: break;
    }
    if (nSBCode != (UINT)-1)
    {
        pWnd->SendMessage(nCmd, MAKELONG(nSBCode, 0));
        return TRUE;
    }
    return FALSE;
}

