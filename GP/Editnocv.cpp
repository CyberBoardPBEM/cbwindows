// Editnocv.cpp : implementation file
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

#include "stdafx.h"
#include "Gp.h"
#include "Editnocv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditNoChevron

CEditNoChevron::CEditNoChevron()
{
}

CEditNoChevron::~CEditNoChevron()
{
}


BEGIN_MESSAGE_MAP(CEditNoChevron, CEdit)
    //{{AFX_MSG_MAP(CEditNoChevron)
    ON_WM_CHAR()
    ON_MESSAGE(WM_PASTE, OnPasteMessage)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditNoChevron message handlers

void CEditNoChevron::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // Filter chevrons ('>>') and paragraph marks ('reverse P')
    if (nChar != 0xBB && nChar != 0xB6)
        CEdit::OnChar(nChar, nRepCnt, nFlags);
}

LRESULT CEditNoChevron::OnPasteMessage(WPARAM, LPARAM)
{
    if (GetStyle() & ES_READONLY)
        return (LRESULT)0;

    if (OpenClipboard())
    {
        LPSTR pText = (LPSTR)GlobalLock(::GetClipboardData(CF_TEXT));
        if (pText != NULL)
        {
            CString str = pText;
            pText = str.LockBuffer();
            while (*pText)
            {
                // Replace chevrons and paragraph chars with spaces
                if (*pText == (char)0xBB || *pText == (char)0xB6)
                    *pText = ' ';
                pText++;
            }
            str.UnlockBuffer();
            ReplaceSel(str);
        }
        GlobalUnlock(::GetClipboardData(CF_TEXT));
        CloseClipboard();
    }
    return (LRESULT)0;
}

