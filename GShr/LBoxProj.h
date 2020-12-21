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

/* Different views use different enumeration type values as the
    group codes, so divide class into base and template.
    However, the actual group code values are never actually
    stored in a normal variable (they are encoded into the
    strings in the list box), so the template is just a
    type-safe wrapper on the base; it has no actual data. */

class CProjListBoxBase : public CGrafixListBox
{
// Construction
public:
    CProjListBoxBase() { m_nMarkGrp = -1; m_nMarkSourceCode = Invalid_v<size_t>;
        m_nHorzWidth = 0; }

// Attributes
public:

// Operations
protected:
    int AddItem(int nGroupCode, LPCSTR pszText, size_t nSourceCode);
    int AddSeqItem(int nGroupCode, LPCSTR pszText, int nSeqNum,
        size_t nSourceCode);
    void MarkGroupItem(int nGroupCode, size_t nSourceCode)
        { m_nMarkGrp = nGroupCode; m_nMarkSourceCode = nSourceCode; }
    int GetItemGroupCode(int nIndex);
public:
    size_t GetItemSourceCode(int nIndex);
    void GetItemText(int nIndex, CString& str);

    int GetItemWidth(int nItem);

    /* N.B.:  CTileBaseListBox requires providing this, and it
        doesn't hurt much to provide it in general. */
    virtual int OnGetItemDebugIDCode(size_t nItem) override
    {
        ASSERT(!"not impl");
        AfxThrowNotSupportedException();
    }

// Implementation
protected:
    int     m_nMarkGrp;
    size_t  m_nMarkSourceCode;
    int     m_nHorzWidth;

    // Overrides
    virtual unsigned OnItemHeight(size_t nIndex) override;
    virtual void OnItemDraw(CDC* pDC, size_t nIndex, UINT nAction, UINT nState,
        CRect rctItem) override;

    virtual void OnDragEnd(CPoint point) override { ASSERT(!"not used"); AfxThrowNotSupportedException(); }

    //{{AFX_MSG(CProjListBox)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

template<typename T>
class CProjListBox : public CProjListBoxBase
{
public:
    int AddItem(T nGroupCode, LPCSTR pszText, size_t nSourceCode = Invalid_v<size_t>)
    {
        return CProjListBoxBase::AddItem(value_preserving_cast<int>(nGroupCode), pszText, nSourceCode);
    }
    int AddSeqItem(T nGroupCode, LPCSTR pszText, int nSeqNum,
        size_t nSourceCode = Invalid_v<size_t>)
    {
        return CProjListBoxBase::AddSeqItem(value_preserving_cast<int>(nGroupCode), pszText, nSeqNum, nSourceCode);
    }
    void MarkGroupItem(T nGroupCode = Invalid_v<T>, size_t nSourceCode = Invalid_v<size_t>)
    {
        CProjListBoxBase::MarkGroupItem(value_preserving_cast<int>(nGroupCode), nSourceCode);
    }
    T GetItemGroupCode(int nIndex)
    {
        return static_cast<T>(CProjListBoxBase::GetItemGroupCode(nIndex));
    }
};

#endif


