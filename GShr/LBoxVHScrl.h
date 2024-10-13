// LBoxVHScrl.h
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

#ifndef _LBOXVHSCRL_H
#define _LBOXVHSCRL_H

namespace CB
{
    class VListBoxHScroll : public wxVListBox
    {
    public:
        VListBoxHScroll() = default;

        void SetItemCount(size_t count) override;
        std::vector<size_t> GetSelections() const;

        // the derived class must implement this function to actually draw the item
        // with the given index on the provided DC
        void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const override = 0;

        // the derived class must implement this method to return the height of the
        // specified item
        wxCoord OnMeasureItem(size_t n) const override
        {
            return GetItemSize(n).GetHeight();
        }

        virtual wxSize GetItemSize(size_t n) const = 0;

        // logical --> device
        wxCoord CalcScrolledX(wxCoord x) const;
        wxPoint CalcScrolledPosition(const wxPoint& pt) const
        {
            return wxPoint(CalcScrolledX(pt.x), pt.y);
        }
        // device --> logical
        wxCoord CalcUnscrolledX(wxCoord x) const;
        wxPoint CalcUnscrolledPosition(const wxPoint& pt) const
        {
            return wxPoint(CalcUnscrolledX(pt.x), pt.y);
        }

    protected:
        void DoSetVirtualSize(int x, int y) override;

    private:
        wxDECLARE_ABSTRACT_CLASS(VListBoxHScroll);
        wxDECLARE_EVENT_TABLE();

        void OnScrollWin(wxScrollWinEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnLButtonDown(wxMouseEvent& event);
        void OnLButtonUp(wxMouseEvent& event);
    };
}

#endif
