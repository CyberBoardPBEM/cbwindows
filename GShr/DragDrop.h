// DragDrop.h
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

#ifndef _DRAGDROP_H
#define _DRAGDROP_H

#include "Tile.h"
#include "Pieces.h"
#include "Marks.h"
class CDrawObj;
class CSelList;

////////////////////////////////////////////////////////////////
// The WM_DRAGDROP message is sent the underlying window
// to see if a drop can occur. The underlying window should
// return a HCURSOR to signify the state of the drop. If zero
// is returned, the "No Drop" cursor is displayed.
//
// The message format is:
//      WPARAM = process id
//      LPARAM = Pointer to DragInfo structure.
// Return:
//      HCURSOR to display (if phaseDragOver). Or zero for no drop cursor.
//      int success/failure (if not phaseDragOver)
//           0 - not handled
//          -1 - failure
//           1 - success

extern const UINT WM_DRAGDROP;
class DragDropEvent;
struct DragInfoWx;
wxDECLARE_EVENT(WM_DRAGDROP_WX, DragDropEvent);
class DragDropEvent : public wxEvent
{
public:
    DragDropEvent(unsigned long pid_, DragInfoWx& di) :
        wxEvent(wxID_ANY, WM_DRAGDROP_WX),
        pid(pid_),
        dragInfo(di)
    {
    }

    unsigned long GetProcessId() const { return pid; }
    const DragInfoWx& GetDragInfo() const { return dragInfo; }

    bool GetAllowScroll() const { return allowScroll; }
    void SetAllowScroll(bool b) { wxASSERT(!"unused code?"); allowScroll = b; }

    wxCursor GetCursor() const { return cursor; }
    void SetCursor(wxCursor c) { cursor = c; }

    wxEvent* Clone() const override { return new DragDropEvent(*this); }

private:
    const unsigned long pid;
    const DragInfoWx& dragInfo;
    bool allowScroll = true;
    wxCursor cursor;
};
typedef void (wxEvtHandler::*DragDropEventFunction)(DragDropEvent&);
#define DragDropEventHandler(func) wxEVENT_HANDLER_CAST(DragDropEventFunction, func)
#define EVT_DRAGDROP(func) \
    wx__DECLARE_EVT0(WM_DRAGDROP_WX, DragDropEventHandler(func))

// Drag phase codes...

enum class PhaseDrag
{
    Enter = 0,
    Exit = 1,
    Over = 2,
    Drop = 3,
};

// ----------------------------------------------- //

enum DragType
{
    DRAG_TILE,
    DRAG_TILELIST,
    DRAG_PIECE,
    DRAG_PIECELIST,
    DRAG_MARKER,
    DRAG_SELECTLIST,
    DRAG_SELECTVIEW,

    DRAG_INVALID = -1,
};

struct DragInfo
{
    DragInfo() { m_dragType = DRAG_INVALID; }
    ~DragInfo() { SetDragType(DRAG_INVALID); }

    DragType    GetDragType() const { return m_dragType; }
    // keep SubInfos constructed to match m_dragType
    void        SetDragType(DragType dt);
private:
    DragType    m_dragType;
public:
    PhaseDrag   m_phase;
//  BOOL        m_bDropAccepted;// Set by droppee so source knows what happened
    CPoint      m_point;        // Loc of mouse in window (dragOver&dragDrop)
    CPoint      m_pointClient;  // list box relative
    HCURSOR     m_hcsrSuggest;  // Suggested cursor to return if can drop

    template<DragType DT>
    struct SubInfo
    {
    };
    template<>
    struct SubInfo<DRAG_TILE>
    {
        TileID m_tileID;
        CSize m_size;
        const CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_TILELIST>
    {
        const std::vector<TileID>* m_tileIDList;
        CSize m_size;
        const CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_PIECE>
    {
        PieceID m_pieceID;
        CSize m_size;
        const CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_PIECELIST>
    {
        const std::vector<PieceID>* m_pieceIDList;
        CSize m_size;
        const CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_MARKER>
    {
        MarkID m_markID;
        CSize m_size;
        CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_SELECTLIST>
    {
        CSelList* m_selectList;
        CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_SELECTVIEW>
    {
        const std::vector<CB::not_null<const CDrawObj*>>* m_ptrArray;
        CGamDoc* m_gamDoc;
    };
    // TODO:  when upgrade to c++ 17, use std::variant
private:
    union SubInfos
    {
        SubInfo<DRAG_TILE> m_tile;
        SubInfo<DRAG_TILELIST> m_tileList;
        SubInfo<DRAG_PIECE> m_piece;
        SubInfo<DRAG_PIECELIST> m_pieceList;
        SubInfo<DRAG_MARKER> m_marker;
        SubInfo<DRAG_SELECTLIST> m_selectList;
        SubInfo<DRAG_SELECTVIEW> m_selectView;

        SubInfos() {}
        ~SubInfos() {}
    } subInfos;
public:
    template<DragType DT>
    const SubInfo<DT>& GetSubInfo() const
    {
        ASSERT(m_dragType == DT);
        /* TODO:  This check could be removed to improve release
            build performance if we trust ourselves to always
            catch any m_dragType mistakes in testing.  Also,
            since this function is inlined, it will often be
            possible for the optimizer to detect that m_dragType
            is correct through data analysis and optimize the
            check out.  */
        if (m_dragType != DT)
        {
            AfxThrowInvalidArgException();
        }
        return reinterpret_cast<const SubInfo<DT>&>(subInfos);
    }

    template<DragType DT>
    SubInfo<DT>& GetSubInfo() { return const_cast<SubInfo<DT>&>(std::as_const(*this).GetSubInfo<DT>()); }
};

struct DragInfoWx
{
    DragInfoWx() { m_dragType = DRAG_INVALID; }
    ~DragInfoWx() { SetDragType(DRAG_INVALID); }

    DragType    GetDragType() const { return m_dragType; }
    // keep SubInfos constructed to match m_dragType
    void        SetDragType(DragType dt);
private:
    DragType    m_dragType;
public:
    PhaseDrag   m_phase;
//  BOOL        m_bDropAccepted;// Set by droppee so source knows what happened
    wxPoint     m_point;        // Loc of mouse in window (dragOver&dragDrop)
    wxPoint     m_pointClient;  // list box relative
    wxCursor    m_hcsrSuggest;  // Suggested cursor to return if can drop

    template<DragType DT>
    struct SubInfo
    {
    };
    template<>
    struct SubInfo<DRAG_TILE>
    {
        TileID m_tileID;
        wxSize m_size;
        const CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_TILELIST>
    {
        const std::vector<TileID>* m_tileIDList;
        wxSize m_size;
        const CGamDoc* m_gamDoc;
    };
#if 0
    template<>
    struct SubInfo<DRAG_PIECE>
    {
        PieceID m_pieceID;
        CSize m_size;
        const CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_PIECELIST>
    {
        const std::vector<PieceID>* m_pieceIDList;
        CSize m_size;
        const CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_MARKER>
    {
        MarkID m_markID;
        wxSize m_size;
        CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_SELECTLIST>
    {
        CSelList* m_selectList;
        CGamDoc* m_gamDoc;
    };
    template<>
    struct SubInfo<DRAG_SELECTVIEW>
    {
        const std::vector<CB::not_null<const CDrawObj*>>* m_ptrArray;
        CGamDoc* m_gamDoc;
    };
#endif
    // TODO:  when upgrade to c++ 17, use std::variant
private:
    union SubInfos
    {
        SubInfo<DRAG_TILE> m_tile;
        SubInfo<DRAG_TILELIST> m_tileList;
#if 0
        SubInfo<DRAG_PIECE> m_piece;
        SubInfo<DRAG_PIECELIST> m_pieceList;
        SubInfo<DRAG_MARKER> m_marker;
        SubInfo<DRAG_SELECTLIST> m_selectList;
        SubInfo<DRAG_SELECTVIEW> m_selectView;
#endif

        SubInfos() {}
        ~SubInfos() {}
    } subInfos;
public:
    template<DragType DT>
    const SubInfo<DT>& GetSubInfo() const
    {
        ASSERT(m_dragType == DT);
        /* TODO:  This check could be removed to improve release
            build performance if we trust ourselves to always
            catch any m_dragType mistakes in testing.  Also,
            since this function is inlined, it will often be
            possible for the optimizer to detect that m_dragType
            is correct through data analysis and optimize the
            check out.  */
        if (m_dragType != DT)
        {
            AfxThrowInvalidArgException();
        }
        return reinterpret_cast<const SubInfo<DT>&>(subInfos);
    }

    template<DragType DT>
    SubInfo<DT>& GetSubInfo() { return const_cast<SubInfo<DT>&>(std::as_const(*this).GetSubInfo<DT>()); }
};

#endif

