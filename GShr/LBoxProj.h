// LBoxProj.h
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

#ifndef _LBOXPROJ_H
#define _LBOXPROJ_H

#ifndef     _LBOXGRFX_H
#include    "LBoxGrfx.h"
#endif

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CProjListBox : public CGrafixListBox
{
// Construction
public:
    CProjListBox() { m_nMarkGrp = -1; m_nMarkSourceCode = -1;
        m_nHorzWidth = 0; }

// Attributes
public:

// Operations
public:
    int AddItem(int nGroupCode, LPCSTR pszText, int nSourceCode = -1);
    int AddSeqItem(int nGroupCode, LPCSTR pszText, int nSeqNum,
        int nSourceCode = -1);
    void MarkGroupItem(int nGroupCode = -1, int nSourceCode = -1)
        { m_nMarkGrp = nGroupCode; m_nMarkSourceCode = nSourceCode; }
    int GetItemGroupCode(int nIndex);
    int GetItemSourceCode(int nIndex);
    void GetItemText(int nIndex, CString& str);

    int CProjListBox::GetItemWidth(int nItem);

// Implementation
protected:
    int     m_nMarkGrp;
    int     m_nMarkSourceCode;
    int     m_nHorzWidth;

    // Overrides
    virtual int OnItemHeight(int nIndex);
    virtual void OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
        CRect rctItem);

    //{{AFX_MSG(CProjListBox)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif


