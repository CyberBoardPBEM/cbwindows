// PalReadMsg.h - Dockable message output window
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

#ifndef _PALREADMSG_H
#define _PALREADMSG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CReadMsgWnd class

class CGamDoc;
#if 0
class CReadMsgWndContainer;
#endif

class CReadMsgWnd : public wxPanel
{
// Construction / destruction
public:
#if 0
    /* N.B. : despite ctor requiring parent,
        this always uses two-phase construction */
    CReadMsgWnd(CReadMsgWndContainer& container);
#else
    CReadMsgWnd();
#endif
    ~CReadMsgWnd() override;
    BOOL Create(wxWindow& parent);

// Methods
public:
#if 0
    const CReadMsgWndContainer& GetParent() const
    {
        return *m_pContainer;
    }
    CReadMsgWndContainer& GetParent()
    {
        return const_cast<CReadMsgWndContainer&>(std::as_const(*this).GetParent());
    }
#endif
    void SetText(CGamDoc* pDoc);

// Implementation - variables
protected:
#if 0
    RefPtr<CReadMsgWndContainer> m_pContainer;
#endif
    CB::propagate_const<CGamDoc*> m_pDoc;                 // Doc of current messages
    size_t          m_nMsgCount;            // Number of messages already processed

    // owned by wx
    RefPtr<wxTextCtrl> m_editCtrl;

// Implementation - methods
protected:
    void ProcessMessages();
    static bool GetLine(CB::string& strBfr, CB::string& strLine);

    void SetDefaults();

    void InsertText(const CB::string& pszText);

    void SetTextStyle(wxColour cr, wxFontWeight dwEffect);

// Implementation - overrides
protected:
    void OnContextMenu(wxContextMenuEvent& event);

#if 0
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#endif
    void OnSize(wxSizeEvent& event);
    wxDECLARE_EVENT_TABLE();
    void OnEditCopy(wxCommandEvent& event);
    void OnUpdateEditCopy(wxUpdateUIEvent& pCmdUI);
//    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
//    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

private:
    typedef wxPanel BASE;
};

#if 0
class CReadMsgWndContainer : public CDockablePane,
                                public CB::NativeContainerWindowMixin
{
public:
    CReadMsgWndContainer();

    operator const CReadMsgWnd&() const { return *child; }
    operator CReadMsgWnd&()
    {
        return const_cast<CReadMsgWnd&>(static_cast<const CReadMsgWnd&>(std::as_const(*this)));
    }

private:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnPaletteHide(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

    typedef CDockablePane BASE;

    // owned by wx
    CB::propagate_const<CReadMsgWnd*> child = new CReadMsgWnd(*this);
};
#endif

/////////////////////////////////////////////////////////////////////////////

#endif // _PALREADMSG_H

