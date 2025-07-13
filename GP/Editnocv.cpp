// Editnocv.cpp : implementation file
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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

wxIMPLEMENT_DYNAMIC_CLASS(CEditNoChevron, wxTextCtrl);


wxBEGIN_EVENT_TABLE(CEditNoChevron, wxTextCtrl)
    EVT_CHAR(OnChar)
    EVT_TEXT_PASTE(wxID_ANY, OnPasteMessage)
wxEND_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////
// CEditNoChevron message handlers

void CEditNoChevron::OnChar(wxKeyEvent& event)
{
    // Filter chevrons ('>>') and paragraph marks ('reverse P')
    if (event.GetUnicodeKey() != 0xBB && event.GetUnicodeKey() != 0xB6)
    {
        event.Skip();
    }
}

void CEditNoChevron::OnPasteMessage(wxClipboardTextEvent& event)
{
    wxClipboardLocker clipLock;
    wxTextDataObject data;
    if (wxTheClipboard->GetData(data))
    {
        wxString s = data.GetText();
        for (auto it = s.begin() ; it != s.end() ; ++it)
        {
            switch ((*it).GetValue())
            {
                case 0xBB:
                case 0xB6:
                    *it = ' ';
                    break;
            }
        }
        long from, to;
        GetSelection(&from, &to);
        Replace(from, to, s);
    }
}

