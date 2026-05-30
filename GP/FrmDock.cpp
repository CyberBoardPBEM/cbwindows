// FrmDock.cpp - container window for the marker/tray palettes.
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

#include "stdafx.h"
#include "Gp.h"
#include "FrmMain.h"
#include "FrmDock.h"
#include "PalMark.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CDockPalette, wxWindow)
    EVT_SIZE(OnSize)
wxEND_EVENT_TABLE()

#if 0
IMPLEMENT_DYNAMIC(CDockPalette, CDockablePane);
#endif

/////////////////////////////////////////////////////////////////////////////

CDockPalette::CDockPalette()
{
}

/////////////////////////////////////////////////////////////////////////////

wxWindow* CDockPalette::GetChild()
{
    wxWindowList& children = GetChildren();
    wxASSERT(children.size() <= size_t(1));
    if (children.empty())
    {
        return nullptr;
    }
    wxWindow& child = CheckedDeref(children.front());
    wxASSERT(dynamic_cast<wxWindow*>(&child));
    return &static_cast<wxWindow&>(child);
}

void CDockPalette::SetChild(wxWindow* pChildWnd)
{
    wxWindow* oldChild = GetChild();
    if (oldChild == pChildWnd)
        return;

    if (oldChild != NULL)
    {
        oldChild->Hide();
        oldChild->Reparent(NULL);
        Refresh(TRUE);
    }
    if (pChildWnd != NULL)
    {
        pChildWnd->Reparent(this);
        wxASSERT(GetChild() == pChildWnd);
        SendSizeEvent();
        pChildWnd->Show();
    }
    else
        GetMainFrame()->ShowPane(*this, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CDockPalette message handlers

void CDockPalette::OnSize(wxSizeEvent& event)
{
    wxRect rct = GetClientRect();
    wxWindow* child = GetChild();
    if (child != NULL)
        child->SetSize(rct);
    event.Skip();
}

// see https ://docs.wxwidgets.org/latest/classwx_window_destroy_event.html
CDockPalette::~CDockPalette()
{
    wxWindow* child = GetChild();
    wxASSERT(!child);
    if (child)
    {
        child->Reparent(nullptr);
    }
}

