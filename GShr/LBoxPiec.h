// LBoxPiec.h
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

#ifndef _LBOXPIEC_H
#define _LBOXPIEC_H

#ifndef     _LBOXTILEBASE_H
#include    "LBoxTileBase.h"
#endif

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CPieceManager;
class CGamDoc;

/////////////////////////////////////////////////////////////////////////////

class CPieceListBox : public CTileBaseListBox
{
// Construction
public:
    CPieceListBox();

// Attributes
public:
    void SetDocument(CGamDoc* pDoc);

    virtual CTileManager* GetTileManager();

// Operations
public:

// Implementation
protected:
    CGamDoc*        m_pDoc;
    CPieceManager*  m_pPMgr;

    // Overrides
    virtual int OnItemHeight(int nIndex);
    virtual void OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
        CRect rctItem);
    virtual BOOL OnDragSetup(DragInfo* pDI);

    // Tool tip processing
    virtual BOOL OnIsToolTipsEnabled();
    virtual int  OnGetHitItemCodeAtPoint(CPoint point, CRect& rct);
    virtual void OnGetTipTextForItemCode(int nItemCode, CString& strTip, CString& strTitle);
    virtual BOOL OnDoesItemHaveTipText(int nItem);

    //{{AFX_MSG(CPieceListBox)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif

