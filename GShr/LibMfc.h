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

BOOL AppendStringToEditBox(CEdit& edit, const CB::string& strAppend,
    BOOL bEnsureNewline = FALSE);

BOOL TranslateKeyToScrollBarMessage(CWnd* pWnd, UINT nChar);
UINT LocateSubMenuIndexOfMenuHavingStartingID(CMenu* pMenu, UINT nID);
void CreateSequentialSubMenuIDs(CMenu& menu, UINT nBaseID, const std::vector<CB::string>& tblNames,
        const std::vector<size_t>* pTblSelections = NULL, UINT nBreaksAt = 20);

/////////////////////////////////////////////////////////////////////////////
// Extend CMDIFrameWndEx with message handlers to split tabs
// into groups.

class CMDIFrameWndExCb : public CMDIFrameWndEx
{
    DECLARE_DYNAMIC(CMDIFrameWndExCb)

protected:
    CMDIFrameWndExCb();

    afx_msg void OnUpdateWindowTile(CCmdUI* pCmdUI);
    afx_msg BOOL OnWindowTile(UINT nID);

    DECLARE_MESSAGE_MAP()

    class CMDIClientAreaWndCb;
    const CMDIClientAreaWndCb& GetMDIClient() const;
    CMDIClientAreaWndCb& GetMDIClient();
};

/////////////////////////////////////////////////////////////////////////////
// provide overloads of MFC functions that support portable types

void DDX_Check(CDataExchange* pDX, int nIDC, bool& value);
void DDX_Text(CDataExchange* pDX, int nIDC, CB::string& value);
void DDV_MaxChars(CDataExchange* pDX, CB::string const& value, int nChars);
CB::string AfxFormatString1(UINT nIDS, const CB::string& lpsz1);

#endif

