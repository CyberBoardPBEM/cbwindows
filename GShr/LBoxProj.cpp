// LBoxProj.cpp
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

wxBEGIN_EVENT_TABLE(CProjListBoxBaseWx, CGrafixListBoxWx)
    EVT_WINDOW_CREATE(CProjListBoxBaseWx::OnCreate)
wxEND_EVENT_TABLE()

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
        wxRect rct = rctItem;
        pDC.DrawText("\xBB"_cbstring, CalcScrolledX(rct.GetLeft()), rct.GetTop());
        rct.Offset(3*g_res.tm8ssb.tmAveCharWidth, 0);
        rct.SetWidth(rct.GetWidth() - 3*g_res.tm8ssb.tmAveCharWidth);
        pDC.DrawText(str.substr(prefixLen), CalcScrolledX(rct.GetLeft()), rct.GetTop());
    }
    else
    {
        pDC.DrawText(str.substr(prefixLen),
                    CalcScrolledX(rctItem.GetLeft() +
                        bHead ? 0 : 3 * g_res.tm8ssb.tmAveCharWidth),
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

