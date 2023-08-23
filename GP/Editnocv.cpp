// Editnocv.cpp : implementation file
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

    class RAII_OpenClipboard
    {
    public:
        RAII_OpenClipboard(CWnd* wnd) : b(wnd->OpenClipboard()) {}
        ~RAII_OpenClipboard()
        {
            if (b)
            {
                CloseClipboard();
            }
        }
        explicit operator bool() const { return b; }
    private:
        const bool b;
    } openClipboard(this);
    if (openClipboard)
    {
        std::optional<CB::string> pText;
        {
            class RAII_GlobalLock
            {
            public:
                RAII_GlobalLock(HGLOBAL h) :
                    hData(h),
                    data(GlobalLock(hData))
                {
                }
                operator const void* () const { return data; }
                ~RAII_GlobalLock() { GlobalUnlock(hData); }
            private:
                const HGLOBAL hData;
                const void* const data;
            };

            RAII_GlobalLock globalLockUnicode(::GetClipboardData(CF_UNICODETEXT));
            if (globalLockUnicode)
            {
                pText.emplace(static_cast<const wchar_t*>(static_cast<const void*>(globalLockUnicode)));
            }
            else
            {
                RAII_GlobalLock globalLockText(::GetClipboardData(CF_UNICODETEXT));
                if (globalLockText)
                {
                    pText.emplace(static_cast<const char*>(static_cast<const void*>(globalLockText)));
                }
            }
        }
        if (pText != NULL)
        {
            // Replace chevrons and paragraph chars with spaces
            CB::string str;
            str.reserve(pText->a_size());
            for (size_t i = size_t(0) ; i < pText->a_size() ; ++i)
            {
                char c = (*pText)[i];
                switch (c)
                {
                    case static_cast<char>(0xBB):
                    case static_cast<char>(0xB6):
                        str += ' ';
                        break;
                    default:
                        str += c;
                }
            }
            ReplaceSel(str);
        }
    }
    return (LRESULT)0;
}

