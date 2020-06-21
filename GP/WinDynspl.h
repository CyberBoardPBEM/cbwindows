// WinDynspl.h
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

#ifndef __WINDYNSPL_H__
#define __WINDYNSPL_H__
#pragma once

// CCbSplitterWnd is a CSplitterWndEx derived class which adds the ability
// to hide and show splitter panes, based upon its index.
class CCbSplitterWnd : public CSplitterWndEx
{
    DECLARE_DYNAMIC(CCbSplitterWnd)
public:
    CCbSplitterWnd();
    virtual ~CCbSplitterWnd();

public:
    int m_nHiddenCol;   // Index of the hidden column.
    int m_nHiddenRow;   // Index of the hidden row.

public:
    BOOL IsRowHidden() { return m_nHiddenRow != -1; }
    BOOL IsColHidden() { return m_nHiddenCol != -1; }

    // Called to switch, or swap a splitter view with another.
    BOOL SwitchView(int nRow, int nCol, CView *pNewView);

    // Called to replace an existing splitter view with another.
    BOOL ReplaceView(int nRow, int nCol, CRuntimeClass *pViewClass);

    // Called to show the column that was previously hidden.
    void ShowColumn();

    // Called to hide a column based upon its index.
    void HideColumn(int nColHide);

    // Called to show the row that was previously hidden.
    void ShowRow();

    // Hides a row based upon its index.
    void HideRow(int nRowHide);

protected:
    DECLARE_MESSAGE_MAP()
};

#endif // __WINDYNSPL_H__


