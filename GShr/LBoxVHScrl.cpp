// LBoxVHScrl.cpp
//
// Copyright (c) 2024 By William Su, All Rights Reserved.
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

#include "LBoxVHScrl.h"

namespace CB
{
    wxIMPLEMENT_ABSTRACT_CLASS(VListBoxHScroll, wxVListBox);

    wxBEGIN_EVENT_TABLE(VListBoxHScroll, wxVListBox)
        EVT_SCROLLWIN(VListBoxHScroll::OnScrollWin)
        EVT_SIZE(OnSize)
        EVT_LEFT_DOWN(OnLButtonDown)
        EVT_LEFT_UP(OnLButtonUp)
    wxEND_EVENT_TABLE()

    void VListBoxHScroll::SetItemCount(size_t count)
    {
        wxVListBox::SetItemCount(count);
        wxCoord width = 0;
        for (size_t i = 0 ; i < count ; ++i)
        {
            width = std::max(width, GetItemSize(i).GetWidth());
        }
        SetVirtualSize(width, GetVirtualSize().GetHeight());
    }

    std::vector<size_t> VListBoxHScroll::GetSelections() const
    {
        std::vector<size_t> retval;
        retval.reserve(GetSelectedCount());
        unsigned long cookie;
        for (int sel = GetFirstSelected(cookie) ;
            sel != wxNOT_FOUND ;
            sel = GetNextSelected(cookie))
        {
            retval.push_back(value_preserving_cast<size_t>(sel));
        }
        return retval;
    }

    // logical --> device
    wxCoord VListBoxHScroll::CalcScrolledX(wxCoord x) const
    {
        return x - GetScrollPos(wxHORIZONTAL);
    }

    // device --> logical
    wxCoord VListBoxHScroll::CalcUnscrolledX(wxCoord x) const
    {
        return x + GetScrollPos(wxHORIZONTAL);
    }

    void VListBoxHScroll::DoSetVirtualSize(int x, int y)
    {
        wxVListBox::DoSetVirtualSize(x, y);
        wxSize clientSize = GetClientSize();
        SetScrollbar(wxHORIZONTAL, 0, clientSize.GetWidth(), x);
    }

    void VListBoxHScroll::OnScrollWin(wxScrollWinEvent& event)
    {
        wxEventType evt = event.GetEventType();
#if !defined(NDEBUG)
        const char* name;
        if (evt == wxEVT_SCROLLWIN_TOP)
        {
            name = "wxEVT_SCROLLWIN_TOP";
        }
        else if (evt == wxEVT_SCROLLWIN_BOTTOM)
        {
            name = "wxEVT_SCROLLWIN_BOTTOM";
        }
        else if (evt == wxEVT_SCROLLWIN_LINEUP)
        {
            name = "wxEVT_SCROLLWIN_LINEUP";
        }
        else if (evt == wxEVT_SCROLLWIN_LINEDOWN)
        {
            name = "wxEVT_SCROLLWIN_LINEDOWN";
        }
        else if (evt == wxEVT_SCROLLWIN_PAGEUP)
        {
            name = "wxEVT_SCROLLWIN_PAGEUP";
        }
        else if (evt == wxEVT_SCROLLWIN_PAGEDOWN)
        {
            name = "wxEVT_SCROLLWIN_PAGEDOWN";
        }
        else if (evt == wxEVT_SCROLLWIN_THUMBTRACK)
        {
            name = "wxEVT_SCROLLWIN_THUMBTRACK";
        }
        else if (evt == wxEVT_SCROLLWIN_THUMBRELEASE)
        {
            name = "wxEVT_SCROLLWIN_THUMBRELEASE";
        }
        else
        {
            wxASSERT(!"unknown event type");
            name = "???";
        }
        CPP20_TRACE(L"{}({}, {}, {})\n", __func__, name, event.GetOrientation() == wxHORIZONTAL ? "horz" : "vert", event.GetPosition());
#endif

        if (event.GetOrientation() == wxVERTICAL)
        {
            event.Skip();
            return;
        }

        int width = GetClientSize().GetWidth();
        int maxWidth = GetScrollRange(wxHORIZONTAL);
        int pos = GetScrollPos(wxHORIZONTAL);
        if (evt == wxEVT_SCROLLWIN_TOP)
        {
            pos = 0;
        }
        else if (evt == wxEVT_SCROLLWIN_BOTTOM)
        {
            pos = maxWidth;
        }
        else if (evt == wxEVT_SCROLLWIN_LINEUP)
        {
            pos -= width / 10;
        }
        else if (evt == wxEVT_SCROLLWIN_LINEDOWN)
        {
            pos += width / 10;
        }
        else if (evt == wxEVT_SCROLLWIN_PAGEUP)
        {
            pos -= width * 9 / 10;
        }
        else if (evt == wxEVT_SCROLLWIN_PAGEDOWN)
        {
            pos += width * 9 / 10;
        }
        else if (evt == wxEVT_SCROLLWIN_THUMBTRACK)
        {
            pos = event.GetPosition();
        }
        else if (evt == wxEVT_SCROLLWIN_THUMBRELEASE)
        {
            pos = event.GetPosition();
        }
        else
        {
            wxASSERT(!"unknown event type");
        }
        pos = std::max(0, pos);
        pos = std::min(pos, maxWidth);
        SetScrollPos(wxHORIZONTAL, pos);
        Refresh();
    }

    void VListBoxHScroll::OnSize(wxSizeEvent& event)
    {
        event.Skip();

        SetScrollbar(wxHORIZONTAL,
                    GetScrollPos(wxHORIZONTAL),
                    GetClientSize().GetWidth(),
                    GetVirtualSize().GetWidth());
        Refresh();
    }

    // MFC CListBox captures mouse, so emulate that behavior
    void VListBoxHScroll::OnLButtonDown(wxMouseEvent& event)
    {
        wxASSERT(!GetCapture());
        CaptureMouse();
        event.Skip();
    }

    void VListBoxHScroll::OnLButtonUp(wxMouseEvent& event)
    {
        if (HasCapture())
        {
            ReleaseMouse();
        }
        event.Skip();
    }
}
