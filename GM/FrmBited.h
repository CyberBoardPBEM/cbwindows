// FrmBited.h
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

class CTileSelView;

/////////////////////////////////////////////////////////////////////////////
// CBitEditFrame frame

class CBitEditFrame : public CMDIChildWndEx,
                        public CB::wxNativeContainerWindowMixin
{
    DECLARE_DYNCREATE(CBitEditFrame)
protected:
    CBitEditFrame();            // protected constructor used by dynamic creation

// Attributes
protected:
    CSplitterWndEx m_wndSplitter;

// Operations
public:

// Implementation
protected:
    virtual ~CBitEditFrame();

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext);

    // Generated message map functions
    //{{AFX_MSG(CBitEditFrame)
    afx_msg void OnClose();
    afx_msg void OnImageDiscard();
    afx_msg void OnImageUpdate();
    afx_msg void OnToolsResizeTile();
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg BOOL OnRotateTile(UINT id);
    afx_msg void OnUpdateRotateTile(CCmdUI* pCmdUI);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CTileSelView& GetTileSelView();
};

/////////////////////////////////////////////////////////////////////////////
