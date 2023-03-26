// LBoxProj.cpp
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
#include    "ResTbl.h"
#include    "StrLib.h"
#include    "LBoxProj.h"
#include    "GdiTools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Each item in the listbox has a five character prefix:
// 0: Group Letter ('A'...)
// 1: Style ('*'=bold, '+'=indent non bold)
// 2-4: number used to force a group to be sorted in a particular
//  sequence. default is 3 spaces.

const int prefixLen = 5;                // five characters

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CProjListBoxBase, CGrafixListBox)
    //{{AFX_MSG_MAP(CProjListBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

int CProjListBoxBase::AddItem(int nGroupCode, LPCSTR pszText, size_t nSourceCode)
{
    CString str;
    str += value_preserving_cast<char>(nGroupCode + 'A');
    str += nSourceCode == Invalid_v<size_t> ? '*' : '+';
    str += "   ";           // Will always precede sequenced entries
    str += pszText;
    int nIdx = AddString(str);
    if (nIdx == LB_ERR)
        return nIdx;
    int nItem = SetItemData(nIdx, nSourceCode != Invalid_v<size_t> ? value_preserving_cast<uintptr_t>(nSourceCode) : Invalid_v<uintptr_t>);
    m_nHorzWidth = CB::max(GetItemWidth(nIdx), m_nHorzWidth);
    SetHorizontalExtent(m_nHorzWidth);
    return nItem;
}

int CProjListBoxBase::AddSeqItem(int nGroupCode, LPCSTR pszText, int nSeqNum,
    size_t nSourceCode)
{
    CString str;
    str += value_preserving_cast<char>(nGroupCode + 'A');
    str += nSourceCode == Invalid_v<size_t> ? '*' : '+';

    char szNum[_MAX_ITOSTR_BASE10_COUNT];
    _itoa(nSeqNum, szNum, 10);
    StrLeadZeros(szNum, size_t(3));
    str += szNum;

    str += pszText;
    int nIdx = AddString(str);
    if (nIdx == LB_ERR)
        return nIdx;
    int nItem = SetItemData(nIdx, value_preserving_cast<DWORD_PTR>(nSourceCode));
    m_nHorzWidth = CB::max(GetItemWidth(nIdx), m_nHorzWidth);
    SetHorizontalExtent(m_nHorzWidth);
    return nItem;
}

int CProjListBoxBase::GetItemGroupCode(int nIndex) const
{
    CB::string str = GetText(nIndex);
    return (int)(str[0] - 'A');
}

size_t CProjListBoxBase::GetItemSourceCode(int nIndex) const
{
    return value_preserving_cast<size_t>(GetItemData(nIndex));
}

void CProjListBoxBase::GetItemText(int nIndex, CString& str)
{
    CB::string strTmp = GetText(nIndex);
    str = (const char*)strTmp + prefixLen;
}

/////////////////////////////////////////////////////////////////////////////

int CProjListBoxBase::GetItemWidth(int nIndex) const
{
    int nWidth = 0;

    // First character in string is sorting code. The second is the
    // style ('*' = Heading line, '+' = Item line)
    CB::string str = GetText(nIndex);
    BOOL bHead = str[1] == '*';
    CWindowDC scrnDC(NULL);

    CFont* pPrevFont = scrnDC.SelectObject(CFont::FromHandle(
        bHead ? g_res.h8ssb : g_res.h8ss));

    if (str[0] - 'A' == m_nMarkGrp &&
        GetItemSourceCode(nIndex) == m_nMarkSourceCode)
    {
        // Mark the line with a chevron
        nWidth = scrnDC.GetTextExtent("\xBB", 1).cx;
    }
    nWidth += scrnDC.GetTextExtent((const char*)str + prefixLen,
        lstrlen((const char*)str + prefixLen)).cx + 16; // (fudge factor)
    scrnDC.SelectObject(pPrevFont);
    return nWidth;
}

/////////////////////////////////////////////////////////////////////////////

CSize CProjListBoxBase::OnItemSize(size_t nIndex) const
{
    return CSize(GetItemWidth(value_preserving_cast<int>(nIndex)), g_res.tm8ssb.tmHeight);
}

void CProjListBoxBase::OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
    CRect rctItem) const
{
    if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
    {
        COLORREF crPrevBack = pDC.SetBkColor(GetSysColor(
            nState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
        COLORREF crPrevText = pDC.SetTextColor(GetSysColor(
            nState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        // First character in string is sorting code. The second is the
        // style ('*' = Heading line, '+' = Item line)
        CB::string str = GetText(value_preserving_cast<int>(nIndex));
        BOOL bHead = str[1] == '*';

        CFont* pPrevFont = pDC.SelectObject(CFont::FromHandle(
            bHead ? g_res.h8ssb : g_res.h8ss));

        if (str[0] - 'A' == m_nMarkGrp &&
            GetItemSourceCode(value_preserving_cast<int>(nIndex)) == m_nMarkSourceCode)
        {
            CRect rct = rctItem;
            // Mark the line with a chevron
            pDC.ExtTextOut(rct.left, rct.top, ETO_OPAQUE, rct,
                "\xBB", 1, NULL);
            rct.left += 3 * g_res.tm8ssb.tmAveCharWidth;
            pDC.ExtTextOut(rctItem.left + 3 * g_res.tm8ssb.tmAveCharWidth,
                rctItem.top, ETO_OPAQUE, rct,
                (const char*)str + prefixLen,
                lstrlen((const char*)str + prefixLen), NULL);
        }
        else
        {
            pDC.ExtTextOut(rctItem.left +
                bHead ? 0 : 3 * g_res.tm8ssb.tmAveCharWidth,
                rctItem.top, ETO_OPAQUE, rctItem,
                (const char*)str + prefixLen,
                lstrlen((const char*)str + prefixLen), NULL);
        }

        pDC.SelectObject(pPrevFont);
        pDC.SetBkColor(crPrevBack);
        pDC.SetTextColor(crPrevText);
    }
    if (nAction & ODA_FOCUS)
        pDC.DrawFocusRect(&rctItem);
}


