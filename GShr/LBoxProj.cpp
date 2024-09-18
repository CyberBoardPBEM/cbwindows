// LBoxProj.cpp
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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

const size_t prefixLen = size_t(5);                // five characters

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CProjListBoxBase, CGrafixListBox)
    //{{AFX_MSG_MAP(CProjListBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

wxBEGIN_EVENT_TABLE(CProjListBoxBaseWx, CGrafixListBoxWx)
    EVT_WINDOW_CREATE(CProjListBoxBaseWx::OnCreate)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////

int CProjListBoxBase::AddItem(int nGroupCode, const CB::string& pszText, size_t nSourceCode)
{
    // str += "   ";           // Will always precede sequenced entries
    CB::string str = std::format(L"{}{}   {}",
                                value_preserving_cast<char>(nGroupCode + 'A'),
                                nSourceCode == Invalid_v<size_t> ? '*' : '+',
                                pszText);
    int nIdx = AddString(str);
    if (nIdx == LB_ERR)
        return nIdx;
    int nItem = SetItemData(nIdx, nSourceCode != Invalid_v<size_t> ? value_preserving_cast<uintptr_t>(nSourceCode) : Invalid_v<uintptr_t>);
    m_nHorzWidth = CB::max(GetItemWidth(nIdx), m_nHorzWidth);
    SetHorizontalExtent(m_nHorzWidth);
    return nItem;
}

int CProjListBoxBase::AddSeqItem(int nGroupCode, const CB::string& pszText, int nSeqNum,
    size_t nSourceCode)
{
    CB::string str = std::format(L"{}{}{:03}{}",
                                value_preserving_cast<char>(nGroupCode + 'A'),
                                nSourceCode == Invalid_v<size_t> ? '*' : '+',
                                nSeqNum,
                                pszText);

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
    return (int)(str[size_t(0)] - 'A');
}

size_t CProjListBoxBase::GetItemSourceCode(int nIndex) const
{
    return value_preserving_cast<size_t>(GetItemData(nIndex));
}

CB::string CProjListBoxBase::GetItemText(int nIndex) const
{
    CB::string strTmp = GetText(nIndex);
    return strTmp.substr(prefixLen);
}

/////////////////////////////////////////////////////////////////////////////

int CProjListBoxBase::GetItemWidth(int nIndex) const
{
    int nWidth = 0;

    // First character in string is sorting code. The second is the
    // style ('*' = Heading line, '+' = Item line)
    CB::string str = GetText(nIndex);
    BOOL bHead = str[size_t(1)] == '*';
    CWindowDC scrnDC(NULL);

    CFont* pPrevFont = scrnDC.SelectObject(CFont::FromHandle(
        bHead ? g_res.h8ssb : g_res.h8ss));

    if (str[size_t(0)] - 'A' == m_nMarkGrp &&
        GetItemSourceCode(nIndex) == m_nMarkSourceCode)
    {
        // Mark the line with a chevron
        nWidth = scrnDC.GetTextExtent("\xBB"_cbstring, 1).cx;
    }
    nWidth += scrnDC.GetTextExtent(str.substr(prefixLen)).cx + 16; // (fudge factor)
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
        BOOL bHead = str[size_t(1)] == '*';

        CFont* pPrevFont = pDC.SelectObject(CFont::FromHandle(
            bHead ? g_res.h8ssb : g_res.h8ss));

        if (str[size_t(0)] - 'A' == m_nMarkGrp &&
            GetItemSourceCode(value_preserving_cast<int>(nIndex)) == m_nMarkSourceCode)
        {
            CRect rct = rctItem;
            // Mark the line with a chevron
            pDC.ExtTextOut(rct.left, rct.top, ETO_OPAQUE, rct,
                "\xBB"_cbstring, 1, NULL);
            rct.left += 3 * g_res.tm8ssb.tmAveCharWidth;
            pDC.ExtTextOut(rctItem.left + 3 * g_res.tm8ssb.tmAveCharWidth,
                rctItem.top, ETO_OPAQUE, rct,
                str.substr(prefixLen),
                NULL);
        }
        else
        {
            pDC.ExtTextOut(rctItem.left +
                bHead ? 0 : 3 * g_res.tm8ssb.tmAveCharWidth,
                rctItem.top, ETO_OPAQUE, rctItem,
                str.substr(prefixLen),
                NULL);
        }

        pDC.SelectObject(pPrevFont);
        pDC.SetBkColor(crPrevBack);
        pDC.SetTextColor(crPrevText);
    }
    if (nAction & ODA_FOCUS)
        pDC.DrawFocusRect(&rctItem);
}

/////////////////////////////////////////////////////////////////////////////

size_t CProjListBoxBaseWx::AddItem(int nGroupCode, const CB::string& pszText, size_t nSourceCode)
{
    // str += "   ";           // Will always precede sequenced entries
    CB::string str = std::format(L"{}{}   {}",
        value_preserving_cast<char>(nGroupCode + 'A'),
        nSourceCode == Invalid_v<size_t> ? '*' : '+',
        pszText);
    size_t nIdx = AddString(str);
    SetItemData(nIdx, nSourceCode != Invalid_v<size_t> ? value_preserving_cast<uintptr_t>(nSourceCode) : Invalid_v<uintptr_t>);
    return nIdx;
}

size_t CProjListBoxBaseWx::AddSeqItem(int nGroupCode, const CB::string& pszText, int nSeqNum,
    size_t nSourceCode)
{
    CB::string str = std::format(L"{}{}{:03}{}",
        value_preserving_cast<char>(nGroupCode + 'A'),
        nSourceCode == Invalid_v<size_t> ? '*' : '+',
        nSeqNum,
        pszText);

    size_t nIdx = AddString(str);
    SetItemData(nIdx, value_preserving_cast<uintptr_t>(nSourceCode));
    return nIdx;
}

int CProjListBoxBaseWx::GetItemGroupCode(size_t nIndex) const
{
    const CB::string& str = GetString(nIndex);
    return str[size_t(0)] - 'A';
}

// delete all items from the control
void CProjListBoxBaseWx::Clear()
{
    items.clear();
    CGrafixListBoxWx::Clear();
}

size_t CProjListBoxBaseWx::GetItemSourceCode(size_t nIndex) const
{
    return value_preserving_cast<size_t>(GetItemData(nIndex));
}

CB::string CProjListBoxBaseWx::GetItemText(size_t nIndex) const
{
    const CB::string& strTmp = GetString(nIndex);
    return strTmp.substr(prefixLen);
}

/////////////////////////////////////////////////////////////////////////////

unsigned CProjListBoxBaseWx::GetItemWidth(size_t nIndex) const
{
    int nWidth = 0;

    // First character in string is sorting code. The second is the
    // style ('*' = Heading line, '+' = Item line)
    const CB::string& str = GetString(nIndex);
    BOOL bHead = str[size_t(1)] == '*';
    /* KLUDGE:  wxInfoDC should not modify wxWindow,
        so const_cast<> should be safe */
    wxInfoDC scrnDC(const_cast<CProjListBoxBaseWx*>(this));

    scrnDC.SetFont(bHead ? g_res.h8ssbWx : g_res.h8ssWx);

    if (str[size_t(0)] - 'A' == m_nMarkGrp &&
        GetItemSourceCode(nIndex) == m_nMarkSourceCode)
    {
        // Mark the line with a chevron
        nWidth = scrnDC.GetTextExtent("\xBB"_cbstring).x;
    }
    nWidth += scrnDC.GetTextExtent(str.substr(prefixLen)).x + 16; // (fudge factor)
    return value_preserving_cast<unsigned>(nWidth);
}

/////////////////////////////////////////////////////////////////////////////

wxSize CProjListBoxBaseWx::GetItemSize(size_t nIndex) const
{
    return wxSize(value_preserving_cast<decltype(std::declval<wxSize>().x)>(GetItemWidth(nIndex)), g_res.tm8ssb.tmHeight);
}

void CProjListBoxBaseWx::OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const
{
    bool selected = IsSelected(nIndex);
    // wx automatically draws selected background
    /* wx defaults to drawing text w/ transparent background,
        so we don't need to set text background color */
    wxDCTextColourChanger setColor(pDC, wxSystemSettings::GetColour(selected ?
        wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT));

    // First character in string is sorting code. The second is the
    // style ('*' = Heading line, '+' = Item line)
    const CB::string& str = GetString(nIndex);
    BOOL bHead = str[size_t(1)] == '*';

    wxDCFontChanger setFont(pDC, bHead ? g_res.h8ssbWx : g_res.h8ssWx);

    if (str[size_t(0)] - 'A' == m_nMarkGrp &&
        GetItemSourceCode(nIndex) == m_nMarkSourceCode)
    {
#if 0
        CRect rct = rctItem;
        // Mark the line with a chevron
        pDC.ExtTextOut(rct.left, rct.top, ETO_OPAQUE, rct,
            "\xBB"_cbstring, 1, NULL);
        rct.left += 3 * g_res.tm8ssb.tmAveCharWidth;
        pDC.ExtTextOut(rctItem.left + 3 * g_res.tm8ssb.tmAveCharWidth,
            rctItem.top, ETO_OPAQUE, rct,
            str.substr(prefixLen),
            NULL);
#else
        wxRect rct = rctItem;
        pDC.DrawText("\xBB"_cbstring, rct.GetLeftTop());
        rct.Offset(3*g_res.tm8ssb.tmAveCharWidth, 0);
        rct.SetWidth(rct.GetWidth() - 3*g_res.tm8ssb.tmAveCharWidth);
        pDC.DrawText(str.substr(prefixLen), rct.GetLeftTop());
#endif
    }
    else
    {
        pDC.DrawText(str.substr(prefixLen),
                    rctItem.GetLeft() +
                        bHead ? 0 : 3 * g_res.tm8ssb.tmAveCharWidth,
                    rctItem.GetTop());
    }

    // wx automatically draws focus rect
}

/* XRC requires default constructor, and Create() is not
    virtual, so initialization must be done here */
void CProjListBoxBaseWx::OnCreate(wxWindowCreateEvent& event)
{
    // need to do this after base class OnCreate()
    CallAfter([this]{
        /* we draw text w/ wxSYS_COLOUR, not theme color,
            so we need to make background use wxSYS_COLOUR
            as well */
        SetSelectionBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));

        // now we can paint
        Thaw();
        Refresh();
    });

    // delay painting until background set
    Freeze();

    event.Skip();
}

size_t CProjListBoxBaseWx::AddString(CB::string s)
{
    Item item(std::move(s), Invalid_v<uintptr_t>);
    auto ub = std::upper_bound(items.begin(), items.end(), item,
                                [](const Item& lhs, const Item& rhs)
                                {
                                    return lhs.string < rhs.string;
                                });
    ub = items.insert(ub, std::move(item));
    SetItemCount(items.size());
    return value_preserving_cast<size_t>(ub - items.begin());
}

void CProjListBoxBaseWx::SetItemData(size_t index, uintptr_t data)
{
    items.at(index).data = data;
}

const CB::string& CProjListBoxBaseWx::GetString(size_t index) const
{
    return items.at(index).string;
}

uintptr_t CProjListBoxBaseWx::GetItemData(size_t index) const
{
    return items.at(index).data;
}

