// PalReadMsg.h - Dockable message output window
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

#ifndef _PALREADMSG_H
#define _PALREADMSG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CReadMsgWnd class

class CGamDoc;

class CReadMsgWnd : public CDockablePane
{
    DECLARE_DYNAMIC(CReadMsgWnd)

// Construction / destruction
public:
    CReadMsgWnd();
    virtual ~CReadMsgWnd();

// Methods
public:
    void SetText(CGamDoc* pDoc);

// Implementation - variables
protected:
    CGamDoc*        m_pDoc;                 // Doc of current messages
    size_t          m_nMsgCount;            // Number of messages already processed

    CRichEditCtrl   m_editCtrl;

// Implementation - methods
protected:
    void ProcessMessages();
    static bool GetLine(CB::string& strBfr, CB::string& strLine);

    void SetDefaults();

    void InsertText(LPCTSTR pszText, BOOL bAtEnd = TRUE);

    void SetTextStyle(COLORREF cr, DWORD dwEffect = 0);
    void SetTextColor(COLORREF cr);
    void SetTextEffect(DWORD dwEffect);
    void SetInsertAtEnd();

    void GetCurCharFormat(CHARFORMAT& cf);
    void SetCharFormat(CHARFORMAT& cf);

// Implementation - overrides
protected:
    //{{AFX_VIRTUAL(CReadMsgWnd)
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual void OnPressCloseButton();
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CReadMsgWnd)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG
    afx_msg LRESULT OnPaletteHide(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
//    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
//    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

/////////////////////////////////////////////////////////////////////////////

#endif // _PALREADMSG_H

