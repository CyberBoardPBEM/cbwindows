// LibMfc.h
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

#ifndef __LIBMFC_H__
#define __LIBMFC_H__

/////////////////////////////////////////////////////////////////////////////
// Handy class for temporarily disabling the Main window. The window is
// reenabled when the object goes out of scope or DisableMainWindow() is
// called.

class CDisableMainWindow
{
public:
    CDisableMainWindow(BOOL bDisable = TRUE);
    ~CDisableMainWindow();
    void DisableMainWindow(BOOL bDisable = TRUE);   // For deferred ops.
};

/////////////////////////////////////////////////////////////////////////////

HWND FindWindowForProcessID(DWORD dwProcessID);
BOOL FindWindowForProcessIDAndBringToFront(DWORD dwProcessID);

BOOL AppendStringToEditBox(CEdit& edit, CString strAppend,
    BOOL bEnsureNewline = FALSE);

BOOL GetMaximumTextExtent(HDC hDC, LPCTSTR pszStr, int nStringLen, int nMaxWidth,
    int* pnFit);    // (not really MFC related)

int CWndOnToolHitTestFix(HWND hWnd, CPoint point, TOOLINFO* pTI);

void GetTemporaryFileName(LPCTSTR lpPrefixString, CString& strTempName);
void TruncatedAnsiStringWithEllipses(CDC *pRefDC, int nTargWidth, CString& str);

BOOL TranslateKeyToScrollBarMessage(CWnd* pWnd, UINT nChar);
UINT LocateSubMenuIndexOfMenuHavingStartingID(CMenu* pMenu, UINT nID);
void CreateSequentialSubMenuIDs(CMenu& menu, UINT nBaseID, CStringArray& tblNames,
        CUIntArray* pTblSelections = NULL, UINT nBreaksAt = 20);

#endif

